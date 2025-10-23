#include "kernel.h"
#include <stdint.h>
#include "fs_internal.h"

#define TAG_OPEN   '{'
#define TAG_CLOSE  '}'

extern Node* fs_get_root(void);
extern void fs_init(void);
extern Node* create_node(const char* name, int type);
extern void disk_write_sector(int sector, const uint8_t* data);
extern void disk_read_sector(int sector, uint8_t* data);

// ============================================================
// Dosya sistemini diske yazma (fs_save)
// ============================================================
static void fs_save_node(Node *dir, uint8_t *buffer, int *pos, int *sector)
{
    for (int i = 0; i < dir->child_count; i++) {
        Node *c = dir->children[i];

        // Türü yaz (1=DIR, 2=FILE)
        buffer[(*pos)++] = (uint8_t)c->type;

        // İsim
        int k = 0;
        while (c->name[k] && *pos < SECTOR_SIZE - 2)
            buffer[(*pos)++] = c->name[k++];
        buffer[(*pos)++] = '\0';

        // Dosya içeriği veya alt dizin
        if (c->type == NODE_FILE) {
            int t = 0;
            while (c->content[t] && *pos < SECTOR_SIZE - 2)
                buffer[(*pos)++] = c->content[t++];
            buffer[(*pos)++] = '\0';
        } else if (c->type == NODE_DIR) {
            buffer[(*pos)++] = TAG_OPEN;
            fs_save_node(c, buffer, pos, sector);
            buffer[(*pos)++] = TAG_CLOSE;
        }

        // Buffer dolmaya yakınsa diske yaz
        if (*pos > SECTOR_SIZE - 64) {
            disk_write_sector((*sector)++, buffer);
            for (int j = 0; j < SECTOR_SIZE; j++)
                buffer[j] = 0;
            *pos = 0;
        }
    }
}

void fs_save(void)
{
    uint8_t buffer[SECTOR_SIZE] = {0};
    int sector = 0;
    int pos = 0;

    // Diski temizle
    uint8_t zero[SECTOR_SIZE] = {0};
    for (int s = 0; s < MAX_SECTORS; s++)
        disk_write_sector(s, zero);

    // Root düğümünden itibaren yaz
    fs_save_node(fs_get_root(), buffer, &pos, &sector);

    if (pos > 0)
        disk_write_sector(sector, buffer);

    print("[Disk] File system saved\n", 0x0B);
}

// ============================================================
// Diskten dosya sistemini geri yükleme (fs_load)
// ============================================================

// Yardımcı fonksiyon: sektör geçişini kontrol eder
static inline uint8_t fs_peek(uint8_t *buf, int *pos, int *sector)
{
    if (*pos >= SECTOR_SIZE) {
        (*sector)++;
        if (*sector >= MAX_SECTORS)
            return 0;
        disk_read_sector(*sector, buf);
        *pos = 0;
    }
    return buf[*pos];
}

// String okuma — sektör sınırlarını kontrol eder
static void fs_read_cstring(char *out, int maxlen, uint8_t *buf, int *pos, int *sector)
{
    int i = 0;
    for (;;) {
        if (*pos >= SECTOR_SIZE) {
            (*sector)++;
            if (*sector >= MAX_SECTORS) {
                out[i] = '\0';
                return;
            }
            disk_read_sector(*sector, buf);
            *pos = 0;
        }

        uint8_t b = buf[(*pos)++];

        if (b == '\0') {
            out[i] = '\0';
            return;
        }

        if (i < maxlen - 1)
            out[i++] = (char)b;
    }
}

// Alt dizinleri rekürsif olarak okur
static void fs_load_node(Node *dir, uint8_t *buffer, int *pos, int *sector)
{
    for (;;) {
        uint8_t tok = fs_peek(buffer, pos, sector);

        // Boş sektör ya da padding
        if (tok == 0) {
            (*sector)++;
            if (*sector >= MAX_SECTORS)
                return;
            disk_read_sector(*sector, buffer);
            *pos = 0;
            tok = buffer[*pos];
            if (tok == 0)
                return;
        }

        // Dizin bitişi
        if (tok == TAG_CLOSE) {
            (*pos)++;
            return;
        }

        // Tür (1=DIR, 2=FILE)
        (*pos)++;
        NodeType type = (NodeType)tok;

        // İsim oku
        char name[MAX_NAME];
        fs_read_cstring(name, MAX_NAME, buffer, pos, sector);

        // Düğüm oluştur
        Node *n = create_node(name, type);
        n->parent = dir;
        dir->children[dir->child_count++] = n;

        if (type == NODE_FILE) {
            fs_read_cstring(n->content, MAX_CONTENT, buffer, pos, sector);
        } else if (type == NODE_DIR) {
            uint8_t openTok = fs_peek(buffer, pos, sector);
            if (openTok == TAG_OPEN) {
                (*pos)++;
                fs_load_node(n, buffer, pos, sector);
            }
        }
    }
}

void fs_load(void)
{
    uint8_t buffer[SECTOR_SIZE];
    int sector = 0;
    disk_read_sector(sector, buffer);

    // Disk boşsa yeni FS başlat
    int allZero = 1;
    for (int i = 0; i < SECTOR_SIZE; i++) {
        if (buffer[i]) {
            allZero = 0;
            break;
        }
    }

    if (allZero) {
        print("[Disk] No file system found\n", 0x08);
        fs_init();
        return;
    }

    fs_init();
    int pos = 0;
    fs_load_node(fs_get_root(), buffer, &pos, &sector);
    print("[Disk] File system loaded successfully\n", 0x0A);
}
