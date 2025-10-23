#include "kernel.h"
#include "drivers.h"

void cpu_init(void)
{
    print("[CPU] Driver initialized\n", 0x0A);
}

// Opsiyonel: CPU adını al (CPUID komutu)
void cpu_get_name(char *out)
{
    unsigned int data[12];
    __asm__ __volatile__(
        "cpuid"
        : "=b"(data[0]), "=d"(data[1]), "=c"(data[2])
        : "a"(0x80000002)
    );
    __asm__ __volatile__(
        "cpuid"
        : "=b"(data[3]), "=d"(data[4]), "=c"(data[5])
        : "a"(0x80000003)
    );
    __asm__ __volatile__(
        "cpuid"
        : "=b"(data[6]), "=d"(data[7]), "=c"(data[8])
        : "a"(0x80000004)
    );

    char *dst = out;
    for (int i = 0; i < 9; i++) {
        unsigned int val = data[i];
        *dst++ = (val >> 0) & 0xFF;
        *dst++ = (val >> 8) & 0xFF;
        *dst++ = (val >> 16) & 0xFF;
        *dst++ = (val >> 24) & 0xFF;
    }
    *dst = '\0';
}
