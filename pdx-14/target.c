/* file: target.c */

#include <stdlib.h>
#include "target.h"

static void setPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	if(x<0 || x>=surface->w || y<0 || y>=surface->h) return;
	if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
	Uint32 *pixels = (Uint32 *)surface->pixels;
	pixels[(y * (surface->pitch / 4)) + x] = color;
	if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}


static Uint32 getPixel(SDL_Surface *surface, int x, int y) {
	if(x<0 || x>=surface->w || y<0 || y>=surface->h) return 0;
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	switch (bpp) {
		case 1: return *p;
		case 2: return *(Uint16 *)p;
		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
		case 4: return *(Uint32 *)p;
		default: return 0;
	}
}


Target *Target_New(Palette *palette, Canvas *canvas, int x, int y) {
    Target *target = malloc(sizeof(*target));

    if(!target) return NULL;

    target->palette = palette;
    target->canvas = canvas;
    target->x = x;
    target->y = y;
    target->visible = true;

    return target;
}

void Target_Free(Target *target) {
    free(target);
}

void Target_Update(Target *target, Mouse *mouse) {
    if(mouse->state & SDL_BUTTON_RMASK) {
    	target->x = mouse->x;
     	target->y = mouse->y;
   }
}

void Target_Draw(Target *target, SDL_Renderer *renderer) {
    int i, j;
    Uint32 pixel;
    Uint8 r, g, b, a;

    int w = 4 - -4 + 1, h = 4 - -4 + 1;
    SDL_Rect area = { target->x - 4, target->y - 4, w, h };

    SDL_Window *window = SDL_RenderGetWindow(renderer);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    Uint32 format = SDL_GetWindowPixelFormat(window);
    int pitch = w * SDL_BYTESPERPIXEL(format);
    Uint32 *pixels = malloc(pitch * h);

    if(!pixels) return;

    if (SDL_RenderReadPixels(renderer, &area, format, pixels, pitch) == 0) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        for(i = -4; i <= 4; i++) {
            int sx = i + 4;
            int sy = 4;
            pixel = pixels[sy * w + sx];
            SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(renderer, 255 - r, 255 - g, 255 - b, a);
            SDL_RenderDrawPoint(renderer, target->x + i, target->y);
        }

        for(j = -4; j <= 4; j++) {
            int sx = 4;
            int sy = j + 4;
            pixel = pixels[sy * w + sx];
            SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(renderer, 255 - r, 255 - g, 255 - b, a);
            SDL_RenderDrawPoint(renderer, target->x, target->y + j);
        }
    }

    free(pixels);
}

