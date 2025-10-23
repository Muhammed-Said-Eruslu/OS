#include "kernel.h"
#include "fs_internal.h"
#include "memory/string_builtin.h"


#define strcat  my_strcat

#define MAX_PATH_DEPTH 8

extern Node* fs_get_root(void);
extern Node* fs_get_current_dir(void);
extern void fs_set_current_dir(Node* dir);
extern Node* create_node(const char* name, int type);
extern Node* find_child(Node* dir, const char* name);
extern void fs_save(void);
extern void strcopy(char* dest, const char* src);

void fs_mkdir(const char* name)
{
    Node* current_dir = fs_get_current_dir();
    if (find_child(current_dir, name)) {
        print("Directory already exists.\n", 0x0C);
        return;
    }

    Node* n = create_node(name, NODE_DIR);
    n->parent = current_dir;   // 🔧 parent ayarla
    current_dir->children[current_dir->child_count++] = n;

    print("Created directory: ", 0x0A);
    print(name, 0x0A);
    print("\n", 0x0A);
    fs_save();
}


void fs_cd(const char *name)
{
    Node *dir = fs_get_current_dir();

    if (!name || !*name) return;

    // 🔹 "cd /" => köke dön
    if (my_strcmp(name, "/") == 0) {
        fs_set_current_dir(fs_get_root());
        fs_set_current_path("/");
        return;
    }

    // 🔹 "cd .." => üst dizine çık
    if (my_strcmp(name, "..") == 0) {
        if (dir->parent)
            dir = dir->parent;
        else
            dir = fs_get_root();

        fs_set_current_dir(dir);

        // Path'i kırp
        char temp[128];
        my_strcpy(temp, fs_get_current_path());
        int len = my_strlen(temp);
        if (len > 1) {
            while (len > 1 && temp[len - 1] != '/') len--;
            if (len == 1) temp[1] = '\0';
            else temp[len - 1] = '\0';
        }
        fs_set_current_path(temp);
        return;
    }

    // 🔹 Alt dizine gir
    for (int i = 0; i < dir->child_count; i++) {
        Node *child = dir->children[i];
        if (child->type == NODE_DIR && my_strcmp(child->name, name) == 0) {
            fs_set_current_dir(child);

            char temp[128];
            my_strcpy(temp, fs_get_current_path());

            // Kök değilse '/' ekle
            if (my_strlen(temp) > 1)
                my_strcat(temp, "/");

            my_strcat(temp, name);
            fs_set_current_path(temp);
            return;
        }
    }

    print("Directory not found.\n", 0x0C);
}



void fs_pwd(void)
{
    print(fs_get_current_path(), 0x0F);
    print("\n", 0x0F);
}



void fs_list_files() {
    Node* current_dir = fs_get_current_dir();
    if (current_dir->child_count == 0) { print("(empty)\n", 0x08); return; }
    for (int i = 0; i < current_dir->child_count; i++) {
        Node* c = current_dir->children[i];
        if (c->type == 1) print("[DIR] ", 0x0B);
        else               print("[FILE] ", 0x07);
        print(c->name, 0x0F); print("\n", 0x0F);
    }
}

void fs_write_file(const char* name, const char* text) {
    Node* current_dir = fs_get_current_dir();
    Node* existing = find_child(current_dir, name);
    if (existing && existing->type == 2) {
        strcopy(existing->content, text);
        print("File updated.\n", 0x0A);
        fs_save();
        return;
    }
    Node* n = create_node(name, 2);
    n->parent = current_dir;
    strcopy(n->content, text);
    current_dir->children[current_dir->child_count++] = n;
    print("File created: ", 0x0A); print(name, 0x0A); print("\n", 0x0A);
    fs_save();
}

char *fs_read_file(const char *filename) {
    Node* n = find_child(fs_get_current_dir(), filename);
    if (n && n->type == 2) return n->content;
    return NULL;
}

void fs_rm(const char* name) {
    Node* current_dir = fs_get_current_dir();
    for (int i = 0; i < current_dir->child_count; i++) {
        Node* c = current_dir->children[i];
        if (strcmp(c->name, name) == 0) {
            for (int j = i; j < current_dir->child_count - 1; j++)
                current_dir->children[j] = current_dir->children[j + 1];
            current_dir->child_count--;
            print("Removed: ", 0x0C); print(name, 0x0C); print("\n", 0x0C);
            fs_save();
            return;
        }
    }
    print("No such file or directory.\n", 0x0C);
}

void fs_mv(const char* oldname, const char* newname) {
    Node* current_dir = fs_get_current_dir();
    for (int i = 0; i < current_dir->child_count; i++) {
        Node* c = current_dir->children[i];
        if (strcmp(c->name, oldname) == 0) {
            strcopy(c->name, newname);
            print("Renamed to: ", 0x0A); print(newname, 0x0A); print("\n", 0x0A);
            fs_save();
            return;
        }
    }
    print("No such file or directory.\n", 0x0C);
}

// ============================================================
// ✨ touch — boş dosya oluşturur (varsa tarihi günceller)
// ============================================================
void fs_touch(const char* name)
{
    if (!name || !*name) {
        print("Usage: touch <filename>\n", 0x08);
        return;
    }

    Node* current_dir = fs_get_current_dir();
    Node* existing = find_child(current_dir, name);

    // 🔹 Dosya zaten varsa, sadece "güncellendi" mesajı ver
    if (existing && existing->type == NODE_FILE) {
        print("> touch ", 0x07);     // gri komut
        print(name, 0x07);
        print("\n[OK] Updated: ", 0x0E);  // sarı renk (güncelleme)
        print(name, 0x0E);
        print("\n", 0x0A);
        fs_save();
        return;
    }

    // 🔹 Yeni dosya oluştur
    Node* n = create_node(name, NODE_FILE);
    if (!n) {
        print("Error: file pool full.\n", 0x0C);
        return;
    }

    n->parent = current_dir;
    n->content[0] = '\0';
    current_dir->children[current_dir->child_count++] = n;

    // 🔹 Şık Linux tarzı çıktı
    print("> touch ", 0x07);     // gri komut
    print(name, 0x07);
    print("\n[OK] Created: ", 0x0A);  // yeşil başarı
    print(name, 0x0A);
    print("\n", 0x0A);

    fs_save();
}
