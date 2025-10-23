#ifndef FS_INTERNAL_H
#define FS_INTERNAL_H

#include "types.h"

// =========================
// Dosya sistemi yapı tipleri
// =========================
#define MAX_FILES       64
#define MAX_NAME        32
#define MAX_CONTENT     256
#define MAX_PATH_DEPTH  8

// 0 artık padding olarak kullanılmıyor
typedef enum {
    NODE_DIR = 1,
    NODE_FILE = 2
} NodeType;

typedef struct Node {
    char name[MAX_NAME];
    NodeType type;
    char content[MAX_CONTENT];
    struct Node* parent;
    struct Node* children[MAX_FILES];
    int child_count;
} Node;

// Global erişimciler (fs_core.c tarafından tanımlanıyor)
Node* fs_get_root(void);
Node* fs_get_current_dir(void);
void fs_set_current_dir(Node* dir);

#endif
