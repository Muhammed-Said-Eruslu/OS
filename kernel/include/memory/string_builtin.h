#ifndef STRING_BUILTIN_H
#define STRING_BUILTIN_H

#include "types.h"

int my_strlen(const char *str);
char *my_strcpy(char *dest, const char *src);
char *my_strncpy(char *dest, const char *src, unsigned int n);
int my_strcmp(const char *s1, const char *s2);
int my_strncmp(const char *s1, const char *s2, unsigned int n);
char *my_strstr(const char *haystack, const char *needle);
char *my_strchr(const char *s, int c);
char *my_strcat(char *dest, const char *src);
void int_to_str(int num, char *str);
char *my_strncat(char *dest, const char *src, int n);

#endif
