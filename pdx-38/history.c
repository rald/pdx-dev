/* file: history.c */
#include "history.h"

History *History_New(Canvas *canvas) {
    int i;
    History *h = malloc(sizeof(*h));
    h->size = canvas->w * canvas->h * canvas->nframe;
    h->current = -1;
    h->count = 0;
    for(i = 0; i < MAX_HISTORY; i++) h->states[i] = malloc(h->size);
    
    /* Save initial state */
    History_Push(h, canvas);
    return h;
}

void History_Push(History *h, Canvas *canvas) {
    /* If we are in the middle of the undo stack and draw, discard "future" states */
    h->current++;
    if (h->current >= MAX_HISTORY) {
        /* Shift everything left to make room at the end */
        byte *temp = h->states[0];
        int i;
        for(i = 0; i < MAX_HISTORY - 1; i++) h->states[i] = h->states[i+1];
        h->states[MAX_HISTORY - 1] = temp;
        h->current = MAX_HISTORY - 1;
    }
    
    memcpy(h->states[h->current], canvas->pixels, h->size);
    h->count = h->current + 1;
}

void History_Undo(History *h, Canvas *canvas) {
    if (h->current > 0) {
        h->current--;
        memcpy(canvas->pixels, h->states[h->current], h->size);
    }
}

void History_Redo(History *h, Canvas *canvas) {
    if (h->current < h->count - 1) {
        h->current++;
        memcpy(canvas->pixels, h->states[h->current], h->size);
    }
}

void History_Free(History *h) {
    int i;
    for(i = 0; i < MAX_HISTORY; i++) free(h->states[i]);
    free(h);
}


