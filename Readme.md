# MyOS

x86 (32-bit) mimarisi için sıfırdan yazılmış, GRUB ile önyüklenen küçük bir işletim sistemi çekirdeği.
Kesme yönetimi, temel donanım sürücüleri, diske kaydedilen bir dosya sistemi, komut geçmişi ve
otomatik tamamlama destekli bir komut satırı ile çekirdek içinde çalışan bir metin editörü içerir.

Proje, bir bilgisayarın açılışından kullanıcı komutlarının işlenmesine kadar geçen süreci uygulamalı
olarak öğrenmek amacıyla geliştirildi. Standart kütüphane (libc) kullanılmaz; string ve bellek
fonksiyonları dahil her şey proje içinde yazılmıştır.

*A small hobby x86 kernel written from scratch in C and NASM: interrupts, drivers, a persistent
file system, a shell with history and tab completion, and an in-kernel text editor.*

## Özellikler

- **Önyükleme:** Multiboot başlığı, GRUB ile açılan ISO, özel linker script
- **Kesmeler:** IDT kurulumu, CPU istisnaları (ISR), donanım kesmeleri (IRQ, PIC yeniden eşleme), Assembly giriş kodları
- **Sürücüler:** PIT zamanlayıcı, PS/2 klavye (TR / US düzen), VGA metin modu, RTC (gerçek zamanlı saat), ATA disk (sektör okuma/yazma), CPUID ile işlemci bilgisi
- **Dosya sistemi:** dizin ağacı (`cd ..` ile üst dizine çıkma), dosya oluşturma/okuma/yazma, taşıma ve silme; içerik disk imajına (`fs.img`) kaydedilir
- **Komut satırı:** ok tuşlarıyla imleç hareketi, komut geçmişi, Tab ile otomatik tamamlama, dosyaya yönlendirme (`echo metin > dosya`)
- **Metin editörü:** [kilo](https://github.com/antirez/kilo) editöründen esinlenen, çekirdek içinde çalışan editör (Ctrl+S kaydet, Ctrl+Q çık, Ctrl+L klavye düzeni)

## Komutlar

| Komut | Açıklama |
|---|---|
| `help` | Komut listesini gösterir |
| `clear` | Ekranı temizler |
| `ls`, `pwd`, `cd <dizin>` | Dizin içeriği, bulunulan dizin, dizin değiştirme |
| `mkdir <ad>`, `touch <dosya>` | Dizin / boş dosya oluşturma |
| `cat <dosya>` | Dosya içeriğini yazdırır |
| `echo <metin> [> dosya]` | Metni yazdırır veya dosyaya yazar |
| `rm <ad>`, `mv <eski> <yeni>` | Silme, taşıma / yeniden adlandırma |
| `edit <dosya>` | Dosyayı metin editöründe açar |
| `setkb tr` / `setkb us` | Klavye düzenini değiştirir |
| `time`, `sysinfo` | Tarih/saat ve sistem bilgisi |
| `reboot`, `exit` | Yeniden başlatma, terminalden çıkış |

## Derleme ve çalıştırma

Gereksinimler (Linux / WSL): `gcc` (32-bit destekli, `gcc-multilib`), `nasm`, `binutils` (`ld`),
`qemu-system-i386`.

```bash
make          # myos.bin çekirdeğini derler
make run      # QEMU'da fs.img diski ile çalıştırır
make clean    # derleme çıktılarını siler
```

Çekirdek `-m32 -ffreestanding -nostdlib -fno-builtin` bayraklarıyla derlenir ve `linker.ld` ile
1 MB adresine bağlanır. Önyüklenebilir imaj `myos.iso` olarak depoda bulunur; `iso/` klasöründen
`grub-mkrescue -o myos.iso iso` ile yeniden üretilebilir.

## Proje yapısı

```text
kernel/
  kernel.c            Multiboot başlığı, çekirdek girişi ve terminal döngüsü
  interrupts/         IDT, ISR, IRQ, PIT zamanlayıcı ve Assembly giriş kodları
  drivers/            VGA, RTC, ATA disk, CPU bilgisi, sürücü başlatma
  keyboard/           PS/2 klavye sürücüsü (TR/US düzen)
  fs/                 Dosya sistemi çekirdeği, yol çözümleme, komutlar, diske kayıt
  shell/              Komut yorumlayıcı, otomatik tamamlama, sistem bilgisi
  editor/             Çekirdek içi metin editörü
  memory/, include/   libc'siz string/bellek fonksiyonları ve başlık dosyaları
document/             Çekirdek kodu hakkında notlar
iso/                  GRUB yapılandırması ve önyüklenebilir imaj içeriği
linker.ld, Makefile   Bağlama ve derleme
main_gui.c            (Deneysel) LVGL + SDL ile QEMU'yu başlatan masaüstü düğmesi
dogsos_gui/           (Deneysel) Flutter ile masaüstü arayüz konsepti
```

## Sınırlamalar

Bu bir öğrenme projesidir: tek görevlidir, sayfalama (paging) ve kullanıcı modu yoktur, dosya sistemi
basit ve projeye özeldir. Gerçek donanımda değil QEMU üzerinde test edilmiştir.

## Geliştirici

Muhammed Said Eruslu · [GitHub](https://github.com/Muhammed-Said-Eruslu)
