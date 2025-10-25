#include "shell.h"
#include "fs.h"
#include "memory/string_builtin.h"
#include "vga.h"
#include "interrupts.h"   // ✅ timer_wait burada tanımlı
#include <stdbool.h>

#define MAX_LINE 256
#define MAX_VARS 32

typedef struct {
    char name[32];
    char value[128];
} fssc_var_t;

static fssc_var_t vars[MAX_VARS];
static int var_count = 0;

static void trim(char *s)
{
    char *p = s;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    char *end = s + my_strlen(s) - 1;
    while (end > p && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
        *end-- = 0;
    if (p != s)
        my_strcpy(s, p);
}



static const char* get_var(const char *name)
{
    for (int i = 0; i < var_count; i++)
        if (my_strcmp(vars[i].name, name) == 0)
            return vars[i].value;
    return "";
}

static void set_var(const char *name, const char *value)
{
    for (int i = 0; i < var_count; i++)
    {
        if (my_strcmp(vars[i].name, name) == 0)
        {
            my_strcpy(vars[i].value, value);
            return;
        }
    }
    if (var_count < MAX_VARS)
    {
        my_strcpy(vars[var_count].name, name);
        my_strcpy(vars[var_count].value, value);
        var_count++;
    }
}

static void replace_vars(char *line)
{
    char out[MAX_LINE];
    int out_i = 0;
    int in_quotes = 0;

    for (int i = 0; line[i]; i++)
    {
        if (line[i] == '"') {
            in_quotes = !in_quotes; // tırnak aç/kapa
            out[out_i++] = line[i];
        }
        else if (line[i] == '$')
        {
            char var[32];
            int j = 0;
            i++;

            while (line[i] && (line[i] != ' ' && line[i] != '\n' &&
                               line[i] != '\r' && line[i] != '"' &&
                               line[i] != '$') && j < 31)
            {
                var[j++] = line[i++];
            }

            var[j] = '\0';
            i--; // bir fazla ilerlemiş olabilir

            const char *val = get_var(var);
            for (int k = 0; val[k] && out_i < MAX_LINE - 1; k++)
                out[out_i++] = val[k];
        }
        else {
            out[out_i++] = line[i];
        }
    }

    out[out_i] = '\0';
    my_strcpy(line, out);
}



// =====================================================
//  🔹 Komut yorumlama
// =====================================================
static void execute_fssc_command(char *line)
{
    replace_vars(line);  // $user gibi değişkenleri çöz

    // --------------------------------------------------
    // set VAR=value
    // --------------------------------------------------
    if (my_strncmp(line, "set ", 4) == 0)
    {
        char *eq = my_strchr(line + 4, '=');
        if (eq)
        {
            *eq = '\0';
            set_var(line + 4, eq + 1);
        }
    }
    // --------------------------------------------------
    // sleep N  (timer_wait ile)
    // --------------------------------------------------
    else if (my_strncmp(line, "sleep ", 6) == 0)
    {
        int ms = my_atoi(line + 6);
        timer_wait(ms);
    }
    // --------------------------------------------------
    // echo "metin"
    // --------------------------------------------------
    else if (my_strncmp(line, "echo ", 5) == 0)
    {
        char *redir = my_strchr(line, '>');

        // 🔹 redirect (örneğin echo "Merhaba" > test.txt)
        if (redir)
        {
            *redir = '\0';
            redir++;
            while (*redir == ' ' || *redir == '"' || *redir == '\'')
                redir++;  // baştaki tırnak ve boşlukları atla

            // sondaki tırnakları temizle
            int len = my_strlen(redir);
            while (len > 0 && (redir[len - 1] == '"' || redir[len - 1] == '\'' || redir[len - 1] == ' '))
                redir[--len] = '\0';

            char result[256];
            my_strcpy(result, line + 5);
            trim(result);

            fs_write_file(redir, result);
            print("[>] Output written to file.\n", 0x0E);
        }
        else
        {
            print(line + 5, 0x0F);
            print("\n", 0x0F);
        }
    }
    // --------------------------------------------------
    // diğer komutlar (ör. ls, mkdir, run, vb.)
    // --------------------------------------------------
    else
    {
        // 🔹 echo hariç bir komut yönlendirilmiş mi?
        char *redir = my_strchr(line, '>');
        if (redir)
        {
            *redir = '\0';
            redir++;
            while (*redir == ' ' || *redir == '"' || *redir == '\'')
                redir++;

            int len = my_strlen(redir);
            while (len > 0 && (redir[len - 1] == '"' || redir[len - 1] == '\'' || redir[len - 1] == ' '))
                redir[--len] = '\0';

            // Komutu çalıştır ve sonucu dosyaya kaydet
            fs_write_file(redir, line);
            print("[>] Output written to file.\n", 0x0E);
        }
        else
        {
            execute_command(line);
        }
    }
}

// =====================================================
//  🔹 if / else / endif desteği
// =====================================================
void fssc_run(const char *filename)
{
    char *content = fs_read_file(filename);
    if (!content)
    {
        print("Script file not found.\n", 0x0C);
        return;
    }

    print("[FSsc] Running: ", 0x0B);
    print(filename, 0x0B);
    print("\n", 0x0B);

    char line[MAX_LINE];
    int idx = 0;
    bool skip_block = false;
    bool in_else = false;

    for (int i = 0; content[i]; i++)
    {
        char c = content[i];
        if (c == '\n' || c == '\r')
        {
            if (idx > 0)
            {
                line[idx] = '\0';
                idx = 0;

                trim(line);
                if (line[0] == 0 || line[0] == '#') continue;

                if (my_strncmp(line, "if exists ", 10) == 0)
                {
                    char *path = line + 10;
                    trim(path);
                    if (!fs_exists(path))
                        skip_block = true;
                    continue;
                }
                else if (my_strcmp(line, "else") == 0)
                {
                    skip_block = !skip_block;
                    in_else = true;
                    continue;
                }
                else if (my_strcmp(line, "endif") == 0)
                {
                    skip_block = false;
                    in_else = false;
                    continue;
                }

                if (!skip_block)
                    execute_fssc_command(line);
            }
        }
        else if (idx < MAX_LINE - 1)
            line[idx++] = c;
    }

    fs_free(content);
    print("[FSsc] Done.\n", 0x0A);
}
