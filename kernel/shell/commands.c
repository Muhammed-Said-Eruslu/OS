#include "kernel.h"
#include <stdint.h>


#define strcpy   my_strcpy
#define strncpy  my_strncpy
#define strcat   my_strcat
#define strlen   my_strlen

extern int g_should_exit_terminal;


// ------------ Yardımcılar ------------

// ascii lower
static inline char to_lower(char c) {
    return (c >= 'A' && c <= 'Z') ? (c + 32) : c;
}

// case-insensitive strcmp
static int strcmpi(const char *a, const char *b) {
    while (*a && *b) {
        char ca = to_lower(*a++), cb = to_lower(*b++);
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
    }
    return (unsigned char)to_lower(*a) - (unsigned char)to_lower(*b);
}


static void itoa2(uint8_t v, char out[3]) {
    out[0] = (char)('0' + (v / 10));
    out[1] = (char)('0' + (v % 10));
    out[2] = '\0';
}

// baştaki/sondaki boşlukları kırp
static const char* lskip(const char* s) {
    while (*s==' ' || *s=='\t') s++;
    return s;
}
static int rtrim_copy(char* out, const char* start, int len) {
    int i = len;
    while (i>0 && (start[i-1]==' ' || start[i-1]=='\t')) i--;
    for (int k=0;k<i;k++) out[k]=start[k];
    out[i]='\0';
    return i;
}

// "echo ..." kısmında '>' arayıp (boşluk olsun/olmasın) parçala
// "hello>f", "hello > f", "hello  >f" hepsi destekli
static void echo_parse_and_run(const char* text) {
    const char* p = text;
    const char* redir = 0;
    for (; *p; p++) {
        if (*p == '>') { redir = p; break; }
    }

    if (!redir) {
        // yönlendirme yok → ekrana bas
        print(text, 0x0F);
        print("\n", 0x0F);
        return;
    }

    // sol taraf (mesaj)
    char msg[512];
    int left_len = (int)(redir - text);
    rtrim_copy(msg, text, left_len);

    // sağ taraf (dosya adı): '>' sonrası boşlukları atla
    const char* fn = redir + 1;
    fn = lskip(fn);
    if (*fn == '\0') {
        print("Usage: echo <text> > <filename>\n", 0x0C);
        return;
    }
    char filename[64];
    strncpy(filename, fn, 63);
    filename[63] = '\0';

    // yaz ve auto-save tetiklenir (fs_write_file içinde de var)
    fs_write_file(filename, msg);
}

// ------------ Komut Yorumlayıcı ------------
void execute_command(const char *cmd_in)
{
    const char* cmd = lskip(cmd_in);

    // HELP
    if (strcmpi(cmd, "help") == 0)
    {
        print("Commands:\n", 0x0F);
        print("  help      - Show this help\n", 0x0F);
        print("  clear     - Clear screen\n", 0x0F);
        print("  ls        - List files and folders\n", 0x0F);
        print("  mkdir X   - Create folder X\n", 0x0F);
        print("  cd X      - Change directory to X (.. up)\n", 0x0F);
        print("  pwd       - Show current path\n", 0x0F);
        print("  echo T    - Print T (or: echo T > file)\n", 0x0F);
        print("  cat F     - Show file content F\n", 0x0F);
        print("  rm X      - Delete file or folder X\n", 0x0F);
        print("  mv A B    - Rename A to B\n", 0x0F);
        print("  reboot    - Restart the OS\n", 0x0F);
        print("  setkb us  - Switch keyboard to US layout\n", 0x0F);
        print("  setkb tr  - Switch keyboard to TR layout\n", 0x0F);
        print("  time      - Show current RTC time\n", 0x0F);
        print("  sysinfo   - Display system information\n", 0x0F);
        print("  touch     - Create an empty file (or update if exists)\n", 0x0F);
        print("  coreedit  - Open CoreEdit text editor\n", 0x0F);
        print("  exit      - Return to desktop\n", 0x0F);
        return;
    }

    // CLEAR
    if (strcmpi(cmd, "clear") == 0) { clear_screen(); return; }
    if (strcmpi(cmd, "exit") == 0) { g_should_exit_terminal = 1; return; }
    // LS
    if (strcmpi(cmd, "ls") == 0) { fs_list_files(); return; }

    // MKDIR
    if (to_lower(cmd[0])=='m' && to_lower(cmd[1])=='k' && to_lower(cmd[2])=='d' &&
        to_lower(cmd[3])=='i' && to_lower(cmd[4])=='r' && cmd[5]==' ')
    {
        fs_mkdir(cmd + 6);
        return;
    }

    // CD
    if ((to_lower(cmd[0])=='c' && to_lower(cmd[1])=='d') && cmd[2]==' ')
    {
        fs_cd(cmd + 3);
        return;
    }

    // PWD
    if (strcmpi(cmd, "pwd") == 0) { fs_pwd(); return; }

    // SETKB
    if (to_lower(cmd[0])=='s' && to_lower(cmd[1])=='e' && to_lower(cmd[2])=='t' &&
        to_lower(cmd[3])=='k' && to_lower(cmd[4])=='b' && cmd[5]==' ')
    {
        const char* arg = lskip(cmd + 6);
        if (arg[0] && !arg[1] ? 0 : 0) {} // no-op to avoid warnings
        if (to_lower(arg[0])=='u' && to_lower(arg[1])=='s' && arg[2]==0) {
            kb_set_layout_us();
            print("[Keyboard] Layout: US\n", 0x0B);
        } else if (to_lower(arg[0])=='t' && to_lower(arg[1])=='r' && arg[2]==0) {
            kb_set_layout_tr();
            print("[Keyboard] Layout: TR\n", 0x0B);
        } else {
            print("Usage: setkb us|tr\n", 0x0C);
        }
        return;
    }

    // ECHO (boşluklu/boşluksuz '>' destekli)
    if (to_lower(cmd[0])=='e' && to_lower(cmd[1])=='c' &&
        to_lower(cmd[2])=='h' && to_lower(cmd[3])=='o' &&
        (cmd[4]==0 || cmd[4]==' '))
    {
        const char* text = lskip(cmd + 4);
        echo_parse_and_run(text);
        return;
    }

    // CAT
    if (to_lower(cmd[0])=='c' && to_lower(cmd[1])=='a' &&
        to_lower(cmd[2])=='t' && cmd[3]==' ')
    {
        const char *filename = lskip(cmd + 4);
        char clean[64]; int i=0;
        while (filename[i] && filename[i]!='\r' && filename[i]!='\n' && i<63) {
            clean[i] = filename[i]; i++;
        }
        clean[i]='\0';
        char *content = fs_read_file(clean);
        if (content) { print(content, 0x0F); print("\n", 0x0F); }
        else print("File not found.\n", 0x0C);
        return;
    }

    // RM
    if (to_lower(cmd[0])=='r' && to_lower(cmd[1])=='m' && cmd[2]==' ')
    {
        fs_rm(lskip(cmd + 3));
        return;
    }

    // MV
    if (to_lower(cmd[0])=='m' && to_lower(cmd[1])=='v' && cmd[2]==' ')
    {
        const char* args = lskip(cmd + 3);
        const char* sp = args;
        while (*sp && *sp!=' ') sp++;
        if (!*sp) { print("Usage: mv <oldname> <newname>\n", 0x0C); return; }

        char oldname[64], newname[64];
        int olen = (int)(sp - args);
        strncpy(oldname, args, (olen<63?olen:63)); oldname[olen<63?olen:63]='\0';
        strcpy(newname, lskip(sp + 1));
        fs_mv(oldname, newname);
        return;
    }

    // REBOOT
    if (strcmpi(cmd, "reboot") == 0) {
        print("[System] Rebooting...\n", 0x0E);
        asm volatile ("outb %%al, $0x64" : : "a"(0xFE));
        return;
    }
if (strcmpi(cmd, "time") == 0)
{
    uint8_t h, m, s;
    rtc_read_time(&h, &m, &s);

    char buf[32], hh[3], mm[3], ss[3];
    itoa2(h, hh); itoa2(m, mm); itoa2(s, ss);

    strcpy(buf, "Time: ");
    strcat(buf, hh); strcat(buf, ":");
    strcat(buf, mm); strcat(buf, ":");
    strcat(buf, ss); strcat(buf, "\n");

    print(buf, 0x0E);
    return;
}
if (strcmpi(cmd, "sysinfo") == 0)
{
    sysinfo_print();
    return;
}
else if (my_strncmp(cmd, "pwd", 3) == 0)
    fs_pwd();

else if (my_strncmp(cmd, "cd ", 3) == 0)
    fs_cd(cmd + 3);
    
else if (my_strncmp(cmd, "touch ", 6) == 0)
    fs_touch(cmd + 6);

else if (strncmp(cmd, "coreedit ", 9) == 0)
{
    const char *fn = cmd + 9;
    if (fn[0]) editor_run_file(fn);
    else print("Usage: coreedit <filename>\n", 0x0C);
}
else if (strcmp(cmd, "coreedit") == 0)
{
    print("Usage: coreedit <filename>\n", 0x0C);
}

else if (strncmp(cmd, "edit ", 5) == 0)
{
    const char *fn = cmd + 5;
    if (fn[0]) editor_run_file(fn);
    else print("Usage: edit <filename>\n", 0x0C);
}
  else
  {
      // Unknown
    print("Unknown command.\n", 0x0C);
  }
}
