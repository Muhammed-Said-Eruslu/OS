Amaç/Sorumluluk
İki ana sorumluluğu var
1. Kernel boot sırası: Multiboot'la açılış/interrupt sürücüleri ve dosya sistemi başlatma

2. Metin terminali: Komut satırı arayüzü (prompt, yazma/silme, ok tuşları, geçmiş, aouto complate)

Bağımlılıklar

kernel.h --> VGA yazdırma (print, clear_screen), cursor_row, MAX_COLS, VGA_ADDRESS, INPUT_BUFFER_SIZE vb çekirdek semboller.

memory/string_builtin_.h --> my_strlen, my_strcpy, my_strncpy, my_strncmp (glibc yok).

Dış API'ler (extern)
. Ekran/imleç: move_cursor(int row, int col)
FS: fs_get_current_path(), ft_set_current_path(const char*), fs_init(), fs_load()
Komut yürütme: execute_command(const char *line)
Donanım/interrupt: idt_init(), isr_install(), irq_install(), timer_install()
Disk: disk_init()
Bilgi: sysinfo_print()
Klavye: read_key() (özel tuşlar için sürücünün döndürdüğü kontrol kodlarına dayanır)

🧱 Sabitler ve Değişkenler
| Ad                       | Tür                                    | Açıklama                                              |
| ------------------------ | -------------------------------------- | ----------------------------------------------------- |
| `MAX_HISTORY`            | `#define`                              | Terminal geçmişinde saklanacak maksimum komut sayısı. |
| `g_should_exit_terminal` | `int`                                  | Terminal döngüsünü kontrol eder (`exit` komutu için). |
| `input_buffer`           | `char[]`                               | Kullanıcının yazdığı geçerli komut satırı.            |
| `input_length`           | `int`                                  | `input_buffer` içindeki aktif karakter sayısı.        |
| `cursor_pos`             | `volatile int`                         | İmlecin mevcut konumu (giriş metni içinde).           |
| `history`                | `char[MAX_HISTORY][INPUT_BUFFER_SIZE]` | Komut geçmişi dizisi.                                 |
| `history_count`          | `int`                                  | Kaç komutun saklandığını tutar.                       |
| `history_index`          | `int`                                  | Yukarı / aşağı tuşlarıyla seçilen geçmiş indeksi.     |


🧩 Fonksiyon Açıklamalar

### 1️⃣ Multiboot Header


__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002, 0x00, -(0x1BADB002)
};

.GRUB önyükleyicisinin çekirdeği tanıması için zorunlu.
0x1BADB002: Magic number
0x00: Flags (hiç özel parametre yok)
-(0x1BADB002): Checksum (toplam = 0 olmalı)


### 2️⃣ prompt_len()

static inline int prompt_len(void) {
    return my_strlen(fs_get_current_path()) + 2;
}

.<path> ve > karakterlerni hesaba katarak prompt uzunluğu döndürür.
.Böylece imleç konumları sabit değil dinamik hesaplanır.

### 3️⃣ add_to_history()

void add_to_history(const char *cmd)

.Kullanıcının girdiği satırı history dizisine ekler.
. Dizi dolduysa FIFO(first in first out) mantığı ile en esikiyi siler
. history_index son elemana ayarlanır.

void add_to_history(const char *cmd)
. Dışardan gelen cmd (null-terminated C string) geçmişe eklenececk.

if (strlen(cmd) == 0) return;
Koruma: Boş satır ("") eklenmesin.

if (history_count < MAX_HISTORY)
    strcpy(history[history_count++], cmd);

Case A: Geçmişte hala yer var.Mesala hıstory count suan 10 max kapasıtem 15 o zaman yer var demektir.

.Yeni komutu history[history_count] konumuna kopyalar (sondaki '\0' dahil).
.Sonra history_count++ ile kayıt sayısı 1 artırır.