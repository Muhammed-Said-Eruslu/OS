#ifndef STRING_BUILTIN_H
#define STRING_BUILTIN_H

#include <stdint.h>

// String functions
void *memset(void *s, int c, uint32_t len);
void *memcpy(void *dest, const void *src, uint32_t len);
void *memmove(void *dest, const void *src, uint32_t len);
int memcmp(const void *s1, const void *s2, uint32_t len);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, uint32_t len);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, uint32_t len);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, uint32_t len);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
int strlen(const char *s);
char *strstr(const char *haystack, const char *needle);
int sprintf(char *buf, const char *fmt, ...);
void my_sprintf(char *buffer, const char *format, ...);
void my_strcpy(char *dest, const char *src);
void my_strncpy(char *dest, const char *src, int n);
void my_strncat(char *dest, const char *src, int n);
int my_strcmp(const char *s1, const char *s2);
int my_strlen(const char *s);
void int_to_str(int num, char *str);
void my_strcat(char *dest, const char *src);
void my_strncat(char *dest, const char *src, int n);

#endif