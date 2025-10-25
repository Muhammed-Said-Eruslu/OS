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

else 
for (int i = 1; i < MAX_HISTORY; i++)
    strcpy(history[i - 1], history[i]);
        strcpy(history[MAX_HISTORY - 1], cmd);

Case B: Geçmiş dolu

for döngüsü MAX_HISTORY-1 aralığındaki tüm öğeleri bir sola kaydırır.
history[1] --> history[0],
history[2] --> history[1],
....
history[MAX_HISTORY-1] --> history[MAX_HISTORY-2].
. Böylece en eski history[0] silinmiş olur.
.Ardından en sona (history[MAX_HISTORY-1]) yeni komut yazılır.

history_index: yukarı aşağı ok tusları ıle gezınırken su an hangı satırda oldugumuzu gösterir.


### 4 static void redraw_input_line(void)

static void redraw_input_line(void)

char *video = (char*)VGA_ADDRESS;
int start = cursor_row * MAX_COLS * 2;

.VGA text-mode belleği adresi: 0xB8000 (macro olarak VGA_ADDRESS tanımladık).

. Her karakter 2 byte
    . 1. byte: Karakterin ASCII kodu
    . 2. byte: renk (ön plan/arka plan renk bitleri)

cursor_row * MAX_COLS * 2:
. İmlecin bulunduğu satırın video belleğindeki başlangıç adresini verir.
. Örneğin: cursor_row = 10, MAX_COLS = 80
--> start = 10 * 160 = 1600, yani 11. satırın başlangıcı

⚙️ Adım 2 — Satırı tamamen temizle
for (int i = 0; i < MAX_COLS; i++) {
    video[start + i * 2] = ' ';
    video[start + i * 2 + 1] = 0x0F;
}
80 sütunun her birini boşluk karakteri ' ' ile doldur.
. Renk 0x0F = beyaz yazı/ siyah arka plan (standart VGA attribute).
. Bu, önceki yazıların kalmaması için satırı sıfırlar.

0x0F hex karşlığı:
. "0x0 = arka plan siyah,"
. "0xF = ön plan beyaz."

⚙️ Adım 3 — Prompt’taki path bilgisini yaz

const char *path = fs_get_current_path();
int color = (my_strlen(path) == 1) ? 0x0A : 0x0B; // kök için yeşil, alt dizin için mavi
int i = 0;
while (path[i]) {
    video[start + i * 2] = path[i];
    video[start + i * 2 + 1] = color;
    i++;
}

fs_get_current_path() --> şu anda hangi dizinde olduğumuzu döndürür (örnek: "/" veya "/home")

. Renk seçimi:
. Eğer path uzunluğu 1 (yani sadece "/"): yeşil (0x0A)
. Aksi halde (alt dizin): mavi (0x0B)

. Döngüde her karakter vga belleğine yazılır:
. video[start + i*2] = karakter,
. video[start + i*2 + 1] = renk kodu.

⚙️ Adım 4 — Prompt sembolünü yaz

video[start + i * 2] = '>';
video[start + i * 2 + 1] = 0x0F; i++;
video[start + i * 2] = ' ';
video[start + i * 2 + 1] = 0x0F; i++;

⚙️ Adım 5 — Kullanıcı girdisini (input_buffer) yaz

for (int j = 0; j < input_length; j++) {
    video[start + (i + j) * 2] = input_buffer[j];
    video[start + (i + j) * 2 + 1] = 0x0F;
}

. Kullanıcının yazdığı her karaketer input_buffer[] içinde.
. Döngü:
    . j = 0 input_length-1 arası tüm karakterleri VGA belleğine çizer.
    . Renk yine beyaz
. Örneğin kullanıcı "echo hi" yazmışsa:
/home> echo hi
. i burada prompt uzunluğunun (path + >) sonundan başlıyor, yani karakterler tam prompt'tan sonra çizliyor.

⚙️ Adım 6 — İmleci doğru yere taşı

move_cursor(cursor_row, plen + cursor_pos);

. plen: prompt_len(), yani (path uzunluğu + 2)
. cursor_pos = imlecin input_buffer içindeki konumu
. Bu ikisi toplanınca imlecin ekrandaki gerçek sütun pozisyonu bulunur.

🧩 Fonksiyon: void terminal_run(void)
🎯 Amaç

Bu fonksiyon, terminalin çalışmasını sağlayan ana döngüyü yönetir.

Ekranı hazırlar ve prompt’u çizer.

Kullanıcıdan tuş girdilerini (read_key()) alır.

Yazma, silme, yön tuşları, geçmiş, otomatik tamamlama gibi davranışları yönetir.

Enter’a basıldığında komutu çalıştırır (execute_command()), geçmişe ekler.

“exit” komutu verilene kadar sonsuz döngüde kalır.

🧠 Genel Akış
Ekranı hazırla  →  Kullanıcı girdisini oku  →  Duruma göre işle  →  Ekranı yenile

🔹 1️⃣ Terminal başlangıç ayarları
clear_screen();
print("Mini Terminal v6 Ready.\nType 'help' for commands.\n\n", 0x0A);

. Ekranı temizler (clear_screen).
. Yeşil renkli (0x0A) bir açılış mesajı yazar.

📁 Varsayılan dizin kontrolü
if (my_strlen(fs_get_current_path()) == 0)
    fs_set_current_path("/");

. Eğer dosya sistemi tarafından boşsa (""), kök dizin / yapılır.
 Böylece her zaman terminal /> şeklinde prompt ile başlar.

 🧹 Değişkenleri sıfırla
 input_length = 0;
history_count = 0;
history_index = -1;
g_should_exit_terminal = 0;
cursor_pos = 0;

| Değişken                 | Anlamı                             |
| ------------------------ | ---------------------------------- |
| `input_length`           | Girdi uzunluğu (şu an 0 karakter). |
| `history_count`          | Geçmişte komut yok.                |
| `history_index`          | -1: henüz geçmişte gezinmiyor.     |
| `g_should_exit_terminal` | Döngü kontrolü. 0 = devam et.      |
| `cursor_pos`             | İmleç başta (yazı yok).            |

🖥️ İlk satırı çiz
redraw_input_line();
. /> prompt'u ekrana yazar.
. Kullanıcı artık yazı yazmaya başlıyabilir.

🔄 2️⃣ Ana terminal döngüsü
while (!g_should_exit_terminal)

. Sonsuz döngü.
. exit komutu g_should_exit_terminal = 1 yaparsa dongü kırılır.

🧩 3️⃣ Tuş okuma
char c = read_key();
if (!c) continue;
. read_key() fonksiyonu klavyeden bir karakter veya özel tuş kodu döndürür.

. Eğer hiçbir tuş basılmadıysa (0 döndüyse) --> döngünün başına dön.

🔹 4️⃣ ENTER (\n)

if (c == '\n') {
    input_buffer[input_length] = '\0';
    print("\n", 0x0F);
}
. Kullanıcı komut girdi enter'a bastı.
. Mevcut komut satırı null-terminated yapılır (\0 sonuna eklenir).
. Yeni satıra geçmek için \n yazılır (beyaz renk).

    if (input_length > 0) {
        execute_command(input_buffer);
        add_to_history(input_buffer);
    }
. Eğer boş değilse yani kullanıcı birşey girdiyse:
. execute_command() cağrılır --> örneğin ls, time, help vs.
. add_to_history() --> komut geçmişine ekler.


input_length = 0;
input_buffer[0] = '\0';
cursor_pos = 0;

. Komut çalıştıktan sonra buffer sıfırlanır (yeni satır için temiz başlangıç).

if (!g_should_exit_terminal)
    redraw_input_line();

. Eğer exit komutu girilmemişse  (g_should_exit_terminal == 0), yeni prompt çizer.
. sonra continue; --> döngünün başına gider.

🔹 5️⃣ BACKSPACE (\b)

else if (c == '\b') {
    if (cursor_pos > 0) {
        for (int i = cursor_pos - 1; i < input_length - 1; i++)
            input_buffer[i] = input_buffer[i + 1];
        input_length--;
        cursor_pos--;
        redraw_input_line();
    }
}

. Sadece imleç başta değilse çalışır.
. Mevcut karakteri siler:
. cursor_pos - 1'den başlıyarak tüm karakterleri sola kaydırır.
. girdi uzunluğu (input_length) 1 azalır.
. İmleç bir karakter sola gider.
. Ardından satırı tamamen yeniden çizer (redraw_input_line()).

🔹 6️⃣ Yukarı ok (kod 1)

else if (c == 1) {
    if (history_index > 0) {
        history_index--;
        strcpy(input_buffer, history[history_index]);
        input_length = strlen(input_buffer);
        cursor_pos = input_length;
        redraw_input_line();
    }
}

. history_index bir adım yukarı gider (önceki komut).
. O satır input_buffer'a kopyalanır.
. Uzunluk ve imleç güncellenir (sonuna gelir).
. Ekran yenilenir --> geçmiş komut görsel olarak gelir.

🔹 7️⃣ Aşağı ok (kod 2)

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

. Eğer hala geçmişte ilerlenebilecek komut varsa
 . Bir sonraki komutu getir.
. Değilse:
    . history_index = history_count --> "boş satır" (yeni girdi).
    . Buffer sıfırlanır.
. Yeniden çizim yapılır

🔹 8️⃣ Sol ok (kod 3)

else if (c == 3) {
    if (cursor_pos > 0) cursor_pos--;
    move_cursor(cursor_row, prompt_len() + cursor_pos);
}
. İmleç başta değilse sola gider.
. move_cursor() gerçek VGA imlecini kaydırır.

🔹 9️⃣ Sağ ok (kod 4)
else if (c == 4) {
    if (cursor_pos < input_length) cursor_pos++;
    move_cursor(cursor_row, prompt_len() + cursor_pos);
}

. İmleç son karakterde değilse bir sağa kayar.
. VGA imleci aynı sekılde güncellenir.

🔹 🔟 TAB (\t) – Basit otomatik tamamlama
else if (c == '\t') {
    if (strncmp(input_buffer, "ec", 2) == 0) {
        strcpy(input_buffer, "echo ");
        input_length = strlen(input_buffer);
        cursor_pos = input_length;
        redraw_input_line();
    }
}

. Kullanıcı "ec" yazmıssa" --> echo olarak otomatık tamamlar.
. Sadece ornek mantık suankı 

🔹 11️⃣ Normal karakterler (yazı yazma)

else if (input_length < INPUT_BUFFER_SIZE - 1) {
    for (int i = input_length; i > cursor_pos; i--)
        input_buffer[i] = input_buffer[i - 1];
    input_buffer[cursor_pos] = c;
    input_length++;
    cursor_pos++;
    redraw_input_line();
}

. Bu kod "insert mode" davranısı.
. Kullanıcı yazı yazdıgında:
. Mevcut karakter bir sağa kayar (for döngüsü).
. Yeni karakter imlecin bulundugu yere eklenir.
. Uzunluk ve imlec komutu artar.
. Satır yeniden cızılır.

🔚 Döngüden çıkış

Döngü while (!g_should_exit_terminal) koşuluna bağlı.
execute_command() içinde “exit” komutu işlendiğinde:

🧩 Fonksiyon: void kernel_main(void)
🎯 Amaç

kernel_main işletim sisteminin boot sonrasında kontrolü aldığı ilk fonksiyondur.
Yani GRUB tarafından “multiboot” etiketi sayesinde çekirdek belleğe yüklendikten sonra CPU bu fonksiyonu çağırır.

Bu fonksiyonun görevi:

Ekranı temizleyip açılış mesajlarını göstermek,

Donanım kesme (interrupt) sistemini başlatmak,

Timer, disk ve dosya sistemini hazırlamak,

Terminali çalıştırmak,

Ardından sistemin sonsuz döngüde beklemesini sağlamak.

🔧 Adım 1 — Ekranı temizle
clear_screen();
VGA belleğini sıfırlar, önceki metinleri siler.

Yeni açılış mesajları tertemiz görünür.

sysinfo_print();
Donanım veya sistem hakkındaki bilgileri gösterir (örnek: çekirdek versiyonu, bellek durumu, CPU, timer frekansı vs.).

Genellikle sysinfo_print() VGA’ya şu tarz bilgiler basar:
CPU: 32-bit
Memory: 512 MB
VGA Mode: 80x25

⚙️ Adım 4 — Interrupt altyapısını kur
idt_init();
isr_install();
irq_install();
timer_install();

Bu kısım sistemin “beyni” gibidir.
CPU’nun dış dünyadan (klavye, timer, disk, vb.) gelen kesme sinyallerine (interrupt) nasıl tepki vereceğini belirler.

| Fonksiyon         | Görev                                                                                                                             |
| ----------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| `idt_init()`      | IDT (Interrupt Descriptor Table) oluşturur — CPU hangi kesmede hangi fonksiyona gideceğini burada öğrenir.                        |
| `isr_install()`   | ISR (Interrupt Service Routines) — CPU iç kesmelerine (örneğin Division by Zero, Page Fault) karşı servis fonksiyonlarını yükler. |
| `irq_install()`   | IRQ (donanım kesmeleri) — klavye, disk, timer gibi dış donanımların kesme numaralarını tanımlar.                                  |
| `timer_install()` | PIT (Programmable Interval Timer)’ı kurar, sistemde periyodik kesmeler oluşturur (örneğin her 10 ms’de bir).                      |


u dört fonksiyon birlikte çalışarak sistemin donanım olaylarını yönetmesini sağlar.

⚡ Adım 5 — Kesmelere izin ver
__asm__ __volatile__("sti");

Assembly komutu: STI = Set Interrupt Flag

CPU’daki IF (Interrupt Flag) bitini 1 yapar.

Bu sayede artık donanım kesmeleri (örneğin klavyeden tuş, timer tick) CPU tarafından işlenebilir.

🚫 Eğer sti çağrılmasa, CPU hiçbir kesmeye tepki vermez — sistem “donmuş” gibi olur.


💾 Adım 7 — Disk ve Dosya Sistemi başlatma

disk_init();
fs_init();
fs_load();

| Fonksiyon     | Görev                                                                                      |
| ------------- | ------------------------------------------------------------------------------------------ |
| `disk_init()` | Disk sürücüsünü (ör. ATA/IDE) hazırlar; okuma/yazma portlarını tanımlar.                   |
| `fs_init()`   | Sanal veya gerçek dosya sistemini oluşturur (ör. RAMFS, FAT benzeri yapı).                 |
| `fs_load()`   | Diskteki dosya yapısını belleğe yükler. (örneğin `/bin`, `/etc`, `/editor` gibi dizinler). |

💤 Adım 9 — Sonsuz bekleme (idle loop)
for (;;)
    __asm__ __volatile__("hlt");

Sonsuz döngü (for(;;)).

hlt → CPU’yu düşük güç moduna alır.

CPU burada “boşta” bekler.

Bir kesme (örneğin timer tick veya klavye tuşu) geldiğinde tekrar uyanır.

Bu yöntemle CPU boşa güç harcamaz, ısı ve enerji tasarrufu sağlanır.
Gerçek işletim sistemlerinde de “idle task” bu işi yapar.