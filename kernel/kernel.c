__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,
    0x00,
    -(0x1BADB002)
};

#include "kernel.h"
#include "memory/string_builtin.h"
#include "autocomplete.h"


// glibc'siz ortam için özel string fonksiyonları
#define strlen   my_strlen
#define strcpy   my_strcpy
#define strncpy  my_strncpy
#define strncmp  my_strncmp   // autocomplete'te kullanılıyor

void move_cursor(int row, int col);


#define MAX_HISTORY 10

int g_should_exit_terminal = 0; // terminal döngüsü kontrolü
extern void sysinfo_print();

static char input_buffer[INPUT_BUFFER_SIZE];
static int  input_length = 0;
volatile int cursor_pos = 0; // metin içindeki imleç konumu

static char history[MAX_HISTORY][INPUT_BUFFER_SIZE];
static int  history_count = 0;
static int  history_index = -1;

static inline int prompt_len(void) {
    return my_strlen(fs_get_current_path()) + 2;
}


void add_to_history(const char *cmd)
{
    if (strlen(cmd) == 0) return;

    if (history_count < MAX_HISTORY) {
        strcpy(history[history_count++], cmd);
    } else {
        for (int i = 1; i < MAX_HISTORY; i++)
            strcpy(history[i - 1], history[i]);
        strcpy(history[MAX_HISTORY - 1], cmd);
    }
    history_index = history_count;
}

static void redraw_input_line(void)
{
    char *video = (char*)VGA_ADDRESS;
    int start = cursor_row * MAX_COLS * 2;

    for (int i = 0; i < MAX_COLS; i++) {
        video[start + i * 2] = ' ';
        video[start + i * 2 + 1] = 0x0F;
    }

    const char *path = fs_get_current_path();
    int color = (my_strlen(path) == 1) ? 0x0A : 0x0B; // kök için yeşil, alt dizin için mavi
    int i = 0;
    while (path[i]) {
        video[start + i * 2] = path[i];
        video[start + i * 2 + 1] = color;
        i++;
    }

    video[start + i * 2] = '>';
    video[start + i * 2 + 1] = 0x0F; i++;
    video[start + i * 2] = ' ';
    video[start + i * 2 + 1] = 0x0F; i++;

    for (int j = 0; j < input_length; j++) {
        video[start + (i + j) * 2] = input_buffer[j];
        video[start + (i + j) * 2 + 1] = 0x0F;
    }

    move_cursor(cursor_row, prompt_len() + cursor_pos);
}

void terminal_run(void)
{
    clear_screen();
    print("Mini Terminal v6 Ready.\nType 'help' for commands.\n\n", 0x0A);

    if (my_strlen(fs_get_current_path()) == 0)
        fs_set_current_path("/");

    input_length = 0;
    history_count = 0;
    history_index = -1;
    g_should_exit_terminal = 0;
    cursor_pos = 0;

    redraw_input_line();

    while (!g_should_exit_terminal)
    {
        char c = read_key();
        if (!c) continue;

        if (c == '\n') {
            input_buffer[input_length] = '\0';
            print("\n", 0x0F);

            if (input_length > 0) {
                execute_command(input_buffer);
                add_to_history(input_buffer);
            }

            input_length = 0;
            input_buffer[0] = '\0';
            cursor_pos = 0;

            if (!g_should_exit_terminal) {
                redraw_input_line();
            }
            continue;
        }

        else if (c == '\b') {
            if (cursor_pos > 0) {
                for (int i = cursor_pos - 1; i < input_length - 1; i++)
                    input_buffer[i] = input_buffer[i + 1];
                input_length--;
                cursor_pos--;
                redraw_input_line();
            }
        }

        else if (c == 1) {
            if (history_index > 0) {
                history_index--;
                strcpy(input_buffer, history[history_index]);
                input_length = strlen(input_buffer);
                cursor_pos = input_length;
                redraw_input_line();
            }
        }

        else if (c == 2) {
            if (history_index < history_count - 1) {
                history_index++;
                strcpy(input_buffer, history[history_index]);
                input_length = strlen(input_buffer);
                cursor_pos = input_length;
            } else {
                history_index = history_count;
                input_buffer[0] = '\0';
                input_length = 0;
                cursor_pos = 0;
            }
            redraw_input_line();
        }

        else if (c == 3) {
            if (cursor_pos > 0) cursor_pos--;
            move_cursor(cursor_row, prompt_len() + cursor_pos);
        }

        else if (c == 4) {
            if (cursor_pos < input_length) cursor_pos++;
            move_cursor(cursor_row, prompt_len() + cursor_pos);
        }

       else if (c == '\t') {
            autocomplete_try(input_buffer, &input_length, &cursor_pos);
            redraw_input_line();
        }

        else if (input_length < INPUT_BUFFER_SIZE - 1) {
            for (int i = input_length; i > cursor_pos; i--)
                input_buffer[i] = input_buffer[i - 1];
            input_buffer[cursor_pos] = c;
            input_length++;
            cursor_pos++;
            redraw_input_line();
        }
    }
}

void kernel_main(void)
{
    clear_screen();

    print("Mini Kernel Booting...\n", 0x0A);
    sysinfo_print();
    idt_init();
    isr_install();
    irq_install();
    timer_install();

   
    __asm__ __volatile__("sti");

    print("[Timer] Installing...\n", 0x0B);
    print("[Timer] Initialized (100 Hz)\n", 0x0B);
    print("Interrupts initialized!\n", 0x0A);

    
    disk_init();
    fs_init();
    fs_load();

    print("File system loaded.\n", 0x0A);
    print("Starting terminal...\n", 0x0A);

    terminal_run();

    for (;;)
        __asm__ __volatile__("hlt");
}
