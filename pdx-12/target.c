/* file: target.c */

#include <stdlib.h>
#include "target.h"

static int Target_CellStep(Canvas *canvas) {
    return canvas->gridShow ? canvas->pixelSize + 1 : canvas->pixelSize;
}

static int Target_ToPixelIndex(Canvas *canvas, int screenCoord) {
    int step = Target_CellStep(canvas);
    return (screenCoord - canvas->x) / step;
}

static int Target_ToScreenCoord(Canvas *canvas, int pixelIndex) {
    int step = Target_CellStep(canvas);
    int gap = canvas->gridShow ? 1 : 0;
    return canvas->x + pixelIndex * step + gap;
}

Target *Target_New(Palette *palette, Canvas *canvas, int px, int py) {
    Target *target = malloc(sizeof(*target));
    if(!target) return NULL;

    target->palette = palette;
    target->canvas = canvas;
    target->px = px;
    target->py = py;
    target->visible = true;

    return target;
}

void Target_Free(Target *target) {
    free(target);
}

void Target_SetPixel(Target *target, int px, int py) {
    if(!target || !target->canvas) return;

    if(px >= 0 && px < target->canvas->w && py >= 0 && py < target->canvas->h) {
        target->px = px;
        target->py = py;
        target->visible = true;
    } else {
        target->visible = false;
    }
}

void Target_Update(Target *target, Mouse *mouse) {
    if(!target || !target->canvas || !mouse) return;

    if(mouse->state & SDL_BUTTON_RMASK) {
        int px = Target_ToPixelIndex(target->canvas, mouse->x);
        int py = Target_ToPixelIndex(target->canvas, mouse->y);
        Target_SetPixel(target, px, py);
    }
}

void Target_Draw(Target *target, SDL_Renderer *renderer) {
    if(!target || !target->canvas || !target->visible) return;

    int step = Target_CellStep(target->canvas);
    int x = Target_ToScreenCoord(target->canvas, target->px) + target->canvas->pixelSize / 2;
    int y = Target_ToScreenCoord(target->canvas, target->py) + target->canvas->pixelSize / 2;

    SDL_SetRenderDrawColor(renderer,
        target->palette->colors[0].r,
        target->palette->colors[0].g,
        target->palette->colors[0].b,
        255);

    SDL_RenderDrawLine(renderer, x - 4, y, x + 4, y);
    SDL_RenderDrawLine(renderer, x, y - 4, x, y + 4);
}