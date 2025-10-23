#include "editor.h"
#include "vga.h"
#include "keyboard.h"
#include "fs.h"
#include "string_builtin.h"
#include <stdint.h>

/* ===========================
   Kilo (bare-metal port)
   - VGA 80x25 text mode
   - keyboard scancodes
   - fs_read_file / fs_write_file
   - no libc, no malloc
   =========================== */

#define ED_MAX_LINES   1024
#define ED_MAX_COLS    256
#define ED_STATUS_ROW  23   /* 0-indexed: 0..24; 23=status, 24=message */
#define ED_MSG_ROW     24
#define ED_VIEW_ROWS   23   /* 0..22 içerik alanı */
#define ED_VIEW_COLS   80

/* ---- dahili yardımcılar (libc yok) ---- */
static inline int min_i(int a, int b){ return a<b?a:b; }
static inline int max_i(int a, int b){ return a>b?a:b; }
static inline int clamp_i(int v,int lo,int hi){ return v<lo?lo:(v>hi?hi:v); }
static inline int is_printable(char c){ return c >= 32 && c <= 126; }

/* overlap güvenli */
static void memmove_local(char *dst, const char *src, int n){
    if (dst == src || n <= 0) return;
    if (dst < src){
        for (int i=0;i<n;i++) dst[i] = src[i];
    } else {
        for (int i=n-1;i>=0;i--) dst[i] = src[i];
    }
}
static void memset_local(char *dst, char v, int n){
    for (int i=0;i<n;i++) dst[i]=v;
}

/* ---- editör durumu ---- */
static char  lines[ED_MAX_LINES][ED_MAX_COLS+1];
static int   num_lines = 1;

static int   fx = 0, fy = 0;       /* dosya içi imleç (kolon/satır) */
static int   coloff = 0, rowoff = 0; /* kaydırma */
static int   dirty = 0;
static int   quit_arm = 0;         /* kaydedilmemiş çıkış için iki kez Ctrl+Q */
static char  filename[64] = "(unnamed)";
static char  msg[80] = "";

/* tek bir büyük tampon: kaydetme/okuma sırasında metni birleştirmek için */
static char  bigbuf[EDITOR_MAX_BUFSIZE];

/* ----------------- durum / mesaj ----------------- */
static void set_status(const char *txt){
    // status bar (ters renk efekti için farklı renk kullanalım)
    char st[80]; st[0]='\0';
    my_strcpy(st, "File: "); my_strcat(st, filename);
    my_strcat(st, "  |  ");
    my_strcat(st, dirty? "[Modified]" : "[Saved]");
    my_strcat(st, "  |  Ln "); 
    // küçük itoa
    {
        int line = fy+1; char nbuf[12]; int i=0, t=line;
        if (t==0){ nbuf[i++]='0'; }
        else { char tmp[12]; int k=0; while(t>0){ tmp[k++] = '0'+(t%10); t/=10; }
               while(k--) nbuf[i++] = tmp[k]; }
        nbuf[i]=0; my_strcat(st, nbuf);
        my_strcat(st, ", Col ");
        int col = fx+1; i=0; t=col;
        if (t==0){ nbuf[i++]='0'; }
        else { char tmp[12]; int k=0; while(t>0){ tmp[k++]='0'+(t%10); t/=10; }
               while(k--) nbuf[i++]=tmp[k]; }
        nbuf[i]=0; my_strcat(st, nbuf);
    }

    /* çiz */
    draw_at(ED_STATUS_ROW, 0, "                                                                                ", 0x08);
    draw_at(ED_STATUS_ROW, 0, st, 0x0F);
}

static void set_message(const char *txt){
    my_strncpy(msg, txt, (int)sizeof(msg)-1);
    msg[sizeof(msg)-1] = '\0';
    draw_at(ED_MSG_ROW, 0, "                                                                                ", 0x07);
    draw_at(ED_MSG_ROW, 0, msg, 0x07);
}

/* ----------------- ekran çizimi ------------------ */
static void draw_line_area(void){
    /* içerik alanını temizle */
    for (int r=0;r<ED_VIEW_ROWS;r++)
        draw_at(r, 0, "                                                                                ", 0x07);

    for (int vis=0; vis<ED_VIEW_ROWS; vis++){
        int lr = rowoff + vis;                /* gerçek satır */
        if (lr >= num_lines) {
            draw_at(vis, 0, "~", 0x08);
            continue;
        }
        int L = my_strlen(lines[lr]);
        if (coloff >= L) continue;

        int len = min_i(ED_VIEW_COLS, L - coloff);
        if (len <= 0) continue;

        /* görünür kısım */
        char tmp[ED_VIEW_COLS+1];
        for (int i=0;i<len;i++) tmp[i] = lines[lr][coloff + i];
        tmp[len] = '\0';
        draw_at(vis, 0, tmp, 0x07);
    }
}

static void refresh_screen(void){
    draw_line_area();
    set_status("");
    set_message(msg);
    /* görünür imleç */
    int cy = fy - rowoff;
    int cx = fx - coloff;
    cy = clamp_i(cy, 0, ED_VIEW_ROWS-1);
    cx = clamp_i(cx, 0, ED_VIEW_COLS-1);
    move_cursor(cy, cx);
}

/* ----------------- dosya işlemleri ---------------- */
static void load_from_text(const char *data){
    num_lines = 0;
    int i = 0;
    while (data && data[i] && num_lines < ED_MAX_LINES){
        /* satırı doldur */
        int ccol = 0;
        while (data[i] && data[i] != '\n' && ccol < ED_MAX_COLS){
            lines[num_lines][ccol++] = data[i++];
        }
        lines[num_lines][ccol] = '\0';
        num_lines++;

        if (data[i] == '\n') i++;  /* newline atla */
    }
    if (num_lines == 0){
        lines[0][0] = '\0';
        num_lines = 1;
    }
}

static void open_file(const char *name){
    my_strncpy(filename, name ? name : "(unnamed)", (int)sizeof(filename)-1);
    filename[sizeof(filename)-1]='\0';
    char *data = fs_read_file(filename);
    if (data) load_from_text(data);
    else { num_lines = 1; lines[0][0] = '\0'; }
    fx = fy = 0; coloff = rowoff = 0;
    dirty = 0; quit_arm = 0;
    set_message("HELP: Ctrl+S save | Ctrl+Q quit | Ctrl+L layout");

}

static void save_file(void){
    /* satırları bigbuf içine birleştir */
    int pos = 0;
    for (int r=0; r<num_lines; r++){
        int L = my_strlen(lines[r]);
        if (pos + L + 1 >= (int)sizeof(bigbuf)) { set_message("! Buffer too big to save"); return; }
        for (int i=0;i<L;i++) bigbuf[pos++] = lines[r][i];
        if (r != num_lines-1) bigbuf[pos++] = '\n';
    }
    bigbuf[pos] = '\0';
    fs_write_file(filename, bigbuf);
    dirty = 0;
    quit_arm = 0;
    set_message("Saved.");
}

/* ----------------- düzenleme primitifleri ---------------- */
static void ensure_cursor_visible(void){
    if (fy < rowoff) rowoff = fy;
    if (fy >= rowoff + ED_VIEW_ROWS) rowoff = fy - (ED_VIEW_ROWS-1);

    if (fx < coloff) coloff = fx;
    if (fx >= coloff + ED_VIEW_COLS) coloff = fx - (ED_VIEW_COLS-1);
}

static void insert_char_into_line(int r, int c, char ch){
    if (r < 0 || r >= num_lines) return;
    int L = my_strlen(lines[r]);
    if (L >= ED_MAX_COLS) return; /* taşma engeli */

    c = clamp_i(c, 0, L);
    /* sağa kaydır */
    memmove_local(&lines[r][c+1], &lines[r][c], L - c + 1);
    lines[r][c] = ch;
    dirty = 1;
}

static void delete_char_before_cursor(void){
    if (fx > 0){
        int L = my_strlen(lines[fy]);
        memmove_local(&lines[fy][fx-1], &lines[fy][fx], L - fx + 1);
        fx--;
        dirty = 1;
    } else if (fy > 0){
        /* bu satırı önceki satıra yapıştır */
        int prevL = my_strlen(lines[fy-1]);
        int curL  = my_strlen(lines[fy]);
        if (prevL + curL < ED_MAX_COLS) {
            my_strcat(lines[fy-1], lines[fy]);
            for (int r = fy; r < num_lines - 1; r++)
                my_strcpy(lines[r], lines[r + 1]);
            num_lines--;
            fy--;
            fx = prevL;
            dirty = 1;
        }
    }
}


static void insert_newline(void){
    int L = my_strlen(lines[fy]);
    int rightLen = L - fx;

    if (num_lines >= ED_MAX_LINES) return;

    /* satırları aşağı kaydır */
    for (int r=num_lines; r>fy+1; r--){
        my_strcpy(lines[r], lines[r-1]);
    }
    lines[fy+1][0] = '\0';
    num_lines++;

    /* sağ kısmı yeni satıra taşı */
    for (int i=0;i<rightLen;i++) lines[fy+1][i] = lines[fy][fx + i];
    lines[fy+1][rightLen] = '\0';
    /* mevcut satırı kes */
    lines[fy][fx] = '\0';

    fy++; fx = 0;
    dirty = 1;
}

/* ----------------- klavye / döngü ---------------- */
static void move_left(void){
    if (fx > 0) {
        fx--;
    } else if (fy > 0) {
        /* önceki satıra atla ve satır sonuna git */
        fy--;
        fx = my_strlen(lines[fy]);
        ensure_cursor_visible();
    }
}

static void move_right(void){
    int L = my_strlen(lines[fy]);
    if (fx < L) {
        fx++;
    } else if (fy + 1 < num_lines) {
        /* sonraki satıra geç ve başa git */
        fy++;
        fx = 0;
        ensure_cursor_visible();
    }
}

static void move_up(void){
    if (fy > 0) fy--;
    int L = my_strlen(lines[fy]);
    if (fx > L) fx = L;
}
static void move_down(void){
    if (fy+1 < num_lines) fy++;
    int L = my_strlen(lines[fy]);
    if (fx > L) fx = L;
}

void editor_run_file(const char *name)
{
    clear_screen();
    open_file(name);
    refresh_screen();

    while (1)
    {
        ensure_cursor_visible();
        refresh_screen();

        /* ==============================
           🔹 Klavye girdisi
           ============================== */
        uint16_t sc = read_scancode();
        char ch = scancode_to_ascii(sc);

        /* ==============================
           🔹 Ctrl kombinasyonları
           ============================== */
           
        if (ch == 19) {  /* Ctrl+S → 19 (ASCII) */
            save_file();
            continue;
        }
        if (ch == 12) {  /* Ctrl+L → layout toggle */
    kb_toggle_layout();
    int layout = kb_get_layout();
    set_message(layout == LAYOUT_TR ? "Layout switched to: TR" : "Layout switched to: US");
    continue;
}

        if (ch == 17) {  /* Ctrl+Q → 17 (ASCII) */
            if (dirty && !quit_arm) {
                quit_arm = 1;
                set_message("Unsaved changes. Press Ctrl+Q again to quit.");
                continue;
            }
            /* çıkış */
            set_message("Exit.");
            clear_screen();
            move_cursor(0, 0);
            print("> ", 0x07);   // shell prompt geri gelsin
            return;
        }

        /* ==============================
           🔹 Özel scancode işlemleri
           (hem normal hem extended)
           ============================== */
        if (sc == 0x001C) {                    /* Enter */
            insert_newline();
        }
        else if (sc == 0x000E) {               /* Backspace */
            delete_char_before_cursor();
        }
        else if (sc == 0x004B || sc == 0xE04B) {  /* Left */
            move_left();
        }
        else if (sc == 0x004D || sc == 0xE04D) {  /* Right */
            move_right();
        }
        else if (sc == 0x0048 || sc == 0xE048) {  /* Up */
            move_up();
        }
        else if (sc == 0x0050 || sc == 0xE050) {  /* Down */
            move_down();
        }
        else if (sc == 0x0047 || sc == 0xE047) {  /* Home */
            fx = 0;
        }
        else if (sc == 0x004F || sc == 0xE04F) {  /* End */
            fx = my_strlen(lines[fy]);
        }
        else if (sc == 0x0049 || sc == 0xE049) {  /* PageUp */
            fy = (fy > ED_VIEW_ROWS) ? fy - ED_VIEW_ROWS : 0;
            int L = my_strlen(lines[fy]);
            if (fx > L) fx = L;
        }
        else if (sc == 0x0051 || sc == 0xE051) {  /* PageDown */
            fy = min_i(num_lines - 1, fy + ED_VIEW_ROWS);
            int L = my_strlen(lines[fy]);
            if (fx > L) fx = L;
        }
        else if (sc == 0x000F) {                /* Tab → 4 boşluk */
            for (int i = 0; i < 4; i++)
                insert_char_into_line(fy, fx + i, ' ');
            fx += 4;
        }
        else {
            /* Yazılabilir ASCII karakter */
            if (is_printable(ch)) {
                insert_char_into_line(fy, fx, ch);
                fx++;
            }
        }

        /* ==============================
           🔹 Durum çubuğu / güncelleme
           ============================== */
        if (!dirty)
            quit_arm = 0;  /* bir şey yaptıysa artık çıkış iki basış istemesin */

        set_status("");
    }
}
