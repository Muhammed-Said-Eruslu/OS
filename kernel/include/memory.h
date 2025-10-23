#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

// String fonksiyonları
int my_strlen(const char *s);
char *my_strcpy(char *dest, const char *src);
char *my_strncpy(char *dest, const char *src, unsigned int n);
int my_strncmp(const char *s1, const char *s2, unsigned int n);
char *my_strcat(char *dest, const char *src);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, unsigned int n);
char *strstr(const char *haystack, const char *needle);
void strcopy(char *dest, const char *src);
char *strchr(const char *s, int c);

// Yardımcılar
void int_to_str(int num, char *str);

#endif
