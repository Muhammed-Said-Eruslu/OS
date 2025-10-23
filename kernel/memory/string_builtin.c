#include "kernel.h"
#include "memory/string_builtin.h"
#include <stdarg.h>

// ==========================================
//   MyOS Kernel — Temel String Fonksiyonları
// ==========================================

// ------------------------------------------
// Uzunluk ölçümü
// ------------------------------------------
int my_strlen(const char *s)
{
    if (!s) return 0;
    int len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

// ------------------------------------------
// Kopyalama
// ------------------------------------------
char *my_strcpy(char *dest, const char *src)
{
    if (!dest || !src) return dest;
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

// ------------------------------------------
// Belirli uzunlukta kopyalama
// ------------------------------------------
char *my_strncpy(char *dest, const char *src, unsigned int n)
{
    if (!dest || !src) return dest;
    unsigned int i = 0;
    while (i < n && src[i])
    {
        dest[i] = src[i];
        i++;
    }
    while (i < n)
        dest[i++] = '\0';
    return dest;
}

// ------------------------------------------
// Karşılaştırma (tam)
// ------------------------------------------
int my_strcmp(const char *s1, const char *s2)
{
    if (!s1 || !s2) return -1;
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

// ------------------------------------------
// Karşılaştırma (n karakter)
// ------------------------------------------
int my_strncmp(const char *s1, const char *s2, unsigned int n)
{
    if (!s1 || !s2) return -1;
    while (n-- > 0 && *s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    if ((int)n < 0)
        return 0;
    return (unsigned char)*s1 - (unsigned char)*s2;
}

// ------------------------------------------
// Alt dize arama (substring)
// ------------------------------------------
char *my_strstr(const char *haystack, const char *needle)
{
    if (!haystack || !needle) return 0;
    if (!*needle) return (char *)haystack;

    for (; *haystack; haystack++)
    {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && (*h == *n))
        {
            h++;
            n++;
        }
        if (!*n)
            return (char *)haystack;
    }
    return 0;
}

// ------------------------------------------
// Karakter arama
// ------------------------------------------
char *my_strchr(const char *s, int c)
{
    if (!s) return 0;
    while (*s)
    {
        if (*s == (char)c)
            return (char *)s;
        s++;
    }
    return 0;
}

// ------------------------------------------
// Sayıyı string'e çevirme (int → ASCII)
// ------------------------------------------
void int_to_str(int num, char *str)
{
    int i = 0, neg = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        neg = 1;
        num = -num;
    }

    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (neg)
        str[i++] = '-';

    str[i] = '\0';

    // Reverse string
    for (int j = 0; j < i / 2; j++) {
        char tmp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = tmp;
    }
}

char *my_strncat(char *dest, const char *src, int n)
{
    int i = 0;
    int j = 0;

    // dest sonuna git
    while (dest[i])
        i++;

    // src'den n karakter kopyala
    while (src[j] && j < n - 1)
    {
        dest[i + j] = src[j];
        j++;
    }

    dest[i + j] = '\0';
    return dest;
}

void my_sprintf(char *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    char *str_val;
    int int_val;
    char char_val;
    char num_buffer[32];
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 's':
                    str_val = va_arg(args, char*);
                    my_strcpy(buffer, str_val);
                    buffer += my_strlen(str_val);
                    break;
                case 'd':
                    int_val = va_arg(args, int);
                    int_to_str(int_val, num_buffer);
                    my_strcpy(buffer, num_buffer);
                    buffer += my_strlen(num_buffer);
                    break;
                case 'c':
                    char_val = (char)va_arg(args, int);
                    *buffer++ = char_val;
                    break;
                case '%':
                    *buffer++ = '%';
                    break;
            }
        } else {
            *buffer++ = *format;
        }
        format++;
    }
    *buffer = '\0';
    va_end(args);
}
