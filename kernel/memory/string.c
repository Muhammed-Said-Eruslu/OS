#include "kernel.h"

// ============================
// String Karşılaştırma Fonksiyonları
// ============================

// strcmp — iki string'i karşılaştırır
int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// strncmp — iki string'i belirli bir uzunluğa kadar karşılaştırır
int strncmp(const char *s1, const char *s2, unsigned int n)
{
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0)
        return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// ============================
// String Arama Fonksiyonları
// ============================

// strstr — bir alt string'i bulur
char* strstr(const char* haystack, const char* needle)
{
    if (!*needle)
        return (char*)haystack;

    for (; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        if (!*n)
            return (char*)haystack;
    }
    return 0;
}

// strchr — bir karakteri string içinde bulur
char *strchr(const char *s, int c)
{
    while (*s)
    {
        if (*s == (char)c)
            return (char *)s;
        s++;
    }
    return 0;
}

// ============================
// String Kopyalama Fonksiyonları
// ============================

// strcpy — bir string'i diğerine kopyalar
char *strcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

// strncpy — belirli uzunlukta kopyalar
char *strncpy(char *dest, const char *src, unsigned int n)
{
    unsigned int i = 0;
    while (i < n && src[i]) {
        dest[i] = src[i];
        i++;
    }
    while (i < n)
        dest[i++] = '\0';
    return dest;
}


char *my_strcat(char *dest, const char *src)
{
    int i = 0;
    int j = 0;

    while (dest[i])
        i++;
    while (src[j])
    {
        dest[i + j] = src[j];
        j++;
    }
    dest[i + j] = '\0';
    return dest;
}
