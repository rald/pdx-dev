/* file: scrollbar.c */

#include <stdlib.h>
#include "scrollbar.h"

#define SCROLLBAR_BTN 16

static int ScrollBar_GetViewPortSize(ScrollBar *sb) {
    return sb->orientation == SCROLLBAR_ORIENTATION_VERTICAL
        ? sb->h - SCROLLBAR_BTN * 2
        : sb->w - SCROLLBAR_BTN * 2;
}

static int ScrollBar_GetTrackArea(ScrollBar *sb) {
    return sb->orientation == SCROLLBAR_ORIENTATION_VERTICAL
        ? sb->h - SCROLLBAR_BTN * 2
        : sb->w - SCROLLBAR_BTN * 2;
}

static int ScrollBar_GetContentSize(ScrollBar *sb) {
    if(sb->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
        return sb->canvas->gridShow
            ? sb->canvas->h * (sb->canvas->pixelSize + 1) + 1
            : sb->canvas->h * sb->canvas->pixelSize;
    }

    return sb->canvas->gridShow
        ? sb->canvas->w * (sb->canvas->pixelSize + 1) + 1
        : sb->canvas->w * sb->canvas->pixelSize;
}

static int ScrollBar_GetTrackStart(ScrollBar *sb) {
    return sb->orientation == SCROLLBAR_ORIENTATION_VERTICAL
        ? sb->y + SCROLLBAR_BTN
        : sb->x + SCROLLBAR_BTN;
}

static void ScrollBar_UpdateThumbRect(ScrollBar *sb) {
    if(sb->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
        sb->rectThumb = (SDL_Rect){
            sb->x,
            sb->y + SCROLLBAR_BTN + sb->thumbPosition,
            sb->w,
            sb->thumbSize
        };
    } else {
        sb->rectThumb = (SDL_Rect){
            sb->x + SCROLLBAR_BTN + sb->thumbPosition,
            sb->y,
            sb->thumbSize,
            sb->h
        };
    }
}

static void ScrollBar_ApplyThumb(ScrollBar *sb) {
    int maxScroll = sb->contentSize - sb->viewPortSize;

    if(maxScroll <= 0) {
        sb->scrollPosition = 0;
        sb->thumbPosition = 0;
        sb->contentScrollPosition = 0;
        sb->thumbSize = sb->trackArea;
        if(sb->orientation == SCROLLBAR_ORIENTATION_VERTICAL) sb->canvas->y = 0;
        else sb->canvas->x = 0;
        return;
    }

    sb->scrollPosition = clamp(sb->scrollPosition, 0, maxScroll);

    sb->thumbSize = (int)((double)sb->viewPortSize / sb->contentSize * sb->trackArea);
    sb->thumbSize = clamp(sb->thumbSize, 1, sb->trackArea);

    if(sb->trackArea - sb->thumbSize > 0) {
        sb->thumbPosition =
            (int)((double)sb->scrollPosition / maxScroll * (sb->trackArea - sb->thumbSize));
        sb->contentScrollPosition =
            (int)((double)sb->thumbPosition / (sb->trackArea - sb->thumbSize) * maxScroll);
    } else {
        sb->thumbPosition = 0;
        sb->contentScrollPosition = 0;
    }

    if(sb->orientation == SCROLLBAR_ORIENTATION_VERTICAL) sb->canvas->y = -sb->contentScrollPosition;
    else sb->canvas->x = -sb->contentScrollPosition;
}

static void ScrollBar_BeginDrag(ScrollBar *sb, Mouse *mouse, int trackStart) {
    sb->dragging = 1;
    if(sb->orientation == SCROLLBAR_ORIENTATION_VERTICAL) {
        sb->dragOffset = mouse->y - (trackStart + sb->thumbPosition);
    } else {
        sb->dragOffset = mouse->x - (trackStart + sb->thumbPosition);
    }
}

static void ScrollBar_DoDrag(ScrollBar *sb, Mouse *mouse, int trackStart, int trackLen) {
    int mousePos = (sb->orientation == SCROLLBAR_ORIENTATION_VERTICAL) ? mouse->y : mouse->x;
    int maxScroll = sb->contentSize - sb->viewPortSize;
    int travel = trackLen - sb->thumbSize;

    if(maxScroll <= 0 || travel <= 0) return;

    sb->thumbPosition = clamp(mousePos - trackStart - sb->dragOffset, 0, travel);
    sb->scrollPosition = (int)((double)sb->thumbPosition / travel * maxScroll);
    sb->contentScrollPosition = sb->scrollPosition;

    if(sb->orientation == SCROLLBAR_ORIENTATION_VERTICAL) sb->canvas->y = -sb->contentScrollPosition;
    else sb->canvas->x = -sb->contentScrollPosition;
}

static void ScrollBar_Refresh(ScrollBar *sb) {
    sb->viewPortSize = ScrollBar_GetViewPortSize(sb);
    sb->trackArea = ScrollBar_GetTrackArea(sb);
    sb->contentSize = ScrollBar_GetContentSize(sb);
    ScrollBar_ApplyThumb(sb);
    ScrollBar_UpdateThumbRect(sb);
}

ScrollBar *ScrollBar_New(
        Palette *palette, Canvas *canvas,
        ScrollBarOrientation orientation,
        int x, int y, int w, int h) {

    ScrollBar *sb = malloc(sizeof(*sb));
    if(!sb) return NULL;

    sb->orientation = orientation;
    sb->x = x;
    sb->y = y;
    sb->w = w;
    sb->h = h;
    sb->palette = palette;
    sb->canvas = canvas;
    sb->scrollPosition = 0;
    sb->contentScrollPosition = 0;
    sb->thumbPosition = 0;
    sb->thumbSize = 0;
    sb->viewPortSize = 0;
    sb->contentSize = 0;
    sb->trackArea = 0;
    sb->dragging = 0;
    sb->dragOffset = 0;

    switch(orientation) {
    case SCROLLBAR_ORIENTATION_VERTICAL:
        sb->buttonUp = Button_New(x, y, w, SCROLLBAR_BTN, palette);
        sb->buttonDown = Button_New(x, y + h - SCROLLBAR_BTN, w, SCROLLBAR_BTN, palette);
        sb->buttonLeft = NULL;
        sb->buttonRight = NULL;
        break;

    case SCROLLBAR_ORIENTATION_HORIZONTAL:
        sb->buttonLeft = Button_New(x, y, SCROLLBAR_BTN, h, palette);
        sb->buttonRight = Button_New(x + w - SCROLLBAR_BTN, y, SCROLLBAR_BTN, h, palette);
        sb->buttonUp = NULL;
        sb->buttonDown = NULL;
        break;

    default:
        sb->buttonUp = NULL;
        sb->buttonDown = NULL;
        sb->buttonLeft = NULL;
        sb->buttonRight = NULL;
        break;
    }

    ScrollBar_Refresh(sb);
    return sb;
}

void ScrollBar_Update(ScrollBar *sb, Mouse *mouse) {
    int trackStart, trackLen;
    SDL_Rect thumbHit;

    ScrollBar_Refresh(sb);

    switch(sb->orientation) {
    case SCROLLBAR_ORIENTATION_VERTICAL:
        if(Button_Update(sb->buttonUp, mouse)) sb->scrollPosition--;
        if(Button_Update(sb->buttonDown, mouse)) sb->scrollPosition++;

        trackStart = ScrollBar_GetTrackStart(sb);
        trackLen = sb->trackArea;
        ScrollBar_ApplyThumb(sb);

        thumbHit = (SDL_Rect){
            sb->x,
            trackStart + sb->thumbPosition,
            sb->w,
            sb->thumbSize
        };

        if(mouse->state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if(!sb->dragging) {
                if(inrect(mouse->x, mouse->y, thumbHit.x, thumbHit.y, thumbHit.w, thumbHit.h))
                    ScrollBar_BeginDrag(sb, mouse, trackStart);
            } else {
                ScrollBar_DoDrag(sb, mouse, trackStart, trackLen);
            }
        } else {
            sb->dragging = 0;
        }

        ScrollBar_ApplyThumb(sb);
        ScrollBar_UpdateThumbRect(sb);
        break;

    case SCROLLBAR_ORIENTATION_HORIZONTAL:
        if(Button_Update(sb->buttonLeft, mouse)) sb->scrollPosition--;
        if(Button_Update(sb->buttonRight, mouse)) sb->scrollPosition++;

        trackStart = ScrollBar_GetTrackStart(sb);
        trackLen = sb->trackArea;
        ScrollBar_ApplyThumb(sb);

        thumbHit = (SDL_Rect){
            trackStart + sb->thumbPosition,
            sb->y,
            sb->thumbSize,
            sb->h
        };

        if(mouse->state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if(!sb->dragging) {
                if(inrect(mouse->x, mouse->y, thumbHit.x, thumbHit.y, thumbHit.w, thumbHit.h))
                    ScrollBar_BeginDrag(sb, mouse, trackStart);
            } else {
                ScrollBar_DoDrag(sb, mouse, trackStart, trackLen);
            }
        } else {
            sb->dragging = 0;
        }

        ScrollBar_ApplyThumb(sb);
        ScrollBar_UpdateThumbRect(sb);
        break;

    default:
        break;
    }
}

void ScrollBar_Draw(ScrollBar *sb, SDL_Renderer *renderer) {
    SDL_Rect clip = { sb->x, sb->y, sb->w, sb->h };
    SDL_RenderSetClipRect(renderer, &clip);

    SDL_SetRenderDrawColor(renderer,
        sb->palette->colors[12].r,
        sb->palette->colors[12].g,
        sb->palette->colors[12].b,
        255
    );
    SDL_RenderDrawRect(renderer, &clip);

    switch(sb->orientation) {
    case SCROLLBAR_ORIENTATION_VERTICAL:
        Button_Draw(sb->buttonUp, renderer);
        Button_Draw(sb->buttonDown, renderer);
        break;
    case SCROLLBAR_ORIENTATION_HORIZONTAL:
        Button_Draw(sb->buttonLeft, renderer);
        Button_Draw(sb->buttonRight, renderer);
        break;
    default:
        break;
    }

    SDL_SetRenderDrawColor(renderer,
        sb->palette->colors[13].r,
        sb->palette->colors[13].g,
        sb->palette->colors[13].b,
        255
    );
    SDL_RenderFillRect(renderer, &sb->rectThumb);

    SDL_RenderSetClipRect(renderer, NULL);
}

