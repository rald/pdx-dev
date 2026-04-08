/* file: history.h */
#ifndef HISTORY_H
#define HISTORY_H

#include "canvas.h"

#define MAX_HISTORY 20

typedef struct {
    byte *states[MAX_HISTORY];
    int current; /* Index of the current state */
    int count;   /* Total states currently stored */
    int size;    /* Size of the pixel buffer in bytes */
} History;

History *History_New(Canvas *canvas);
void History_Free(History *h);
void History_Push(History *h, Canvas *canvas);
void History_Undo(History *h, Canvas *canvas);
void History_Redo(History *h, Canvas *canvas);

#endif


