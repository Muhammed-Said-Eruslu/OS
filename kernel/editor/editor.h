#ifndef EDITOR_H
#define EDITOR_H

#include <stdint.h>

// ============================
//  Limitler ve Ayarlar
// ============================
#define EDITOR_MAX_BUFSIZE 32768

// ============================
//  Editör Durumu (opsiyonel)
// ============================
typedef struct {
    int cx, cy;              // İmleç konumu
    int screenrows, screencols;
    int numrows;
    char filename[256];
    char *buf;
    int buflen;
    int dirty;
} Editor;

// Global değişken (gerekirse)
extern Editor E;

// Fonksiyon bildirimi
void editor_run_file(const char *name);

#endif
