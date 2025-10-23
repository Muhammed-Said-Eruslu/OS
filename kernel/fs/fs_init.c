#include "kernel.h"
#include "fs_internal.h"

#define MAX_FILES 64

extern void strcopy(char* dest, const char* src);
extern Node* fs_get_root(void);
extern void fs_set_current_dir(Node* dir);
extern void fs_set_current_path(const char* path);

void fs_init() {
    Node* root = fs_get_root();
    strcopy(root->name, "/");
    root->type = NODE_DIR;
    root->parent = NULL;          // 🔧 düzeltildi (önceden root->parent = root idi)
    root->child_count = 0;

    for (int i = 0; i < MAX_FILES; i++)
        root->children[i] = 0;

    fs_set_current_dir(root);
    fs_set_current_path("/");     // 🔧 path senkronize et
}
