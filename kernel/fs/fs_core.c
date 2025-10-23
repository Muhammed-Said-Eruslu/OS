#include "kernel.h"
#include "fs_internal.h"
#include "memory/string_builtin.h"

static Node root;
static Node* current_dir = &root;

void strcopy(char* dest, const char* src)
{
    while ((*dest++ = *src++));
}

Node* create_node(const char* name, NodeType type)
{
    static Node pool[MAX_FILES * 10];
    static int used = 0;
    if (used >= MAX_FILES * 10) return 0;

    Node* n = &pool[used++];
    strcopy(n->name, name);
    n->type = type;
    n->child_count = 0;
    n->parent = NULL;
    n->content[0] = '\0';

    for (int i = 0; i < MAX_FILES; i++)
        n->children[i] = 0;

    return n;
}

Node* find_child(Node* dir, const char* name)
{
    for (int i = 0; i < dir->child_count; i++)
        if (!my_strcmp(dir->children[i]->name, name))
            return dir->children[i];
    return 0;
}

Node* fs_get_root(void) { return &root; }
Node* fs_get_current_dir(void) { return current_dir; }
void fs_set_current_dir(Node* dir) { current_dir = dir; }
