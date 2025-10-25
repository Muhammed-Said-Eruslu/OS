#include "autocomplete.h"
#include "memory/string_builtin.h"
#include "fs.h"
#include "vga.h"

#define MAX_SUGGESTIONS 64

// ======================================
// 1️⃣ Komut listesi
// ======================================
static const char *builtin_commands[] = {
    "help", "clear", "ls", "mkdir", "cd", "pwd",
    "echo", "cat", "rm", "mv", "touch",
    "setkb", "time", "sysinfo", "reboot",
    "coreedit", "edit", "exit", 0
};

// ======================================
// 2️⃣ Basit baş harfe göre eşleşme
// ======================================
static const char* find_command_completion(const char *prefix)
{
    int prefix_len = my_strlen(prefix);
    for (int i = 0; builtin_commands[i]; i++) {
        if (my_strncmp(builtin_commands[i], prefix, prefix_len) == 0)
            return builtin_commands[i];
    }
    return 0;
}

// ======================================
// 3️⃣ Dosya tabanlı tamamlama
// ======================================
// fs_list_entries() fonksiyonun yoksa fs_list_files()’ı modifiye edip çağırabilirim
// burada basitçe fs_get_dir_entry() benzeri bir mekanizma farz ediyoruz

static const char* find_file_completion(const char *prefix)
{
    static char result[64];
    const char *best = 0;
    int prefix_len = my_strlen(prefix);

    fs_entry_t entries[MAX_SUGGESTIONS];
    int count = fs_list_entries(entries, MAX_SUGGESTIONS);
    for (int i = 0; i < count; i++) {
        if (my_strncmp(entries[i].name, prefix, prefix_len) == 0) {
            best = entries[i].name;
            break;
        }
    }
    if (best) {
        my_strcpy(result, best);
        return result;
    }
    return 0;
}

// ======================================
// 4️⃣ Genel autocomplete_try
// ======================================
void autocomplete_try(char *input_buffer, int *input_length, volatile int *cursor_pos)
{
    // Eğer komutun ilk kelimesindeyiz:
    int i = 0;
    while (input_buffer[i] && input_buffer[i] != ' ') i++;
    int has_space = (input_buffer[i] == ' ');

    if (!has_space)
    {
        // Komut kısmı için
        const char *match = find_command_completion(input_buffer);
        if (match) {
            my_strcpy(input_buffer, match);
            *input_length = my_strlen(input_buffer);
            *cursor_pos = *input_length;
        }
        return;
    }
    else
    {
        // Komut zaten var, dosya argümanı tamamla
        const char *prefix = input_buffer + i + 1;
        const char *match = find_file_completion(prefix);
        if (match) {
            // prefix kısmını silip yerine tam dosya adını yaz
            int cmd_len = i + 1;
            for (int k = 0; match[k]; k++)
                input_buffer[cmd_len + k] = match[k];
            input_buffer[cmd_len + my_strlen(match)] = '\0';
            *input_length = my_strlen(input_buffer);
            *cursor_pos = *input_length;
        }
    }
}
