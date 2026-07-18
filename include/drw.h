// See LICENSE file for copyright and license details.

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

typedef struct {
    Cursor cursor;
} Cur;

typedef struct Fnt {
    Display *dpy;
    unsigned h;
    XftFont *xfont;
    FcPattern *pattern;
    struct Fnt *next;
} Fnt;

typedef XftColor Clr;
enum {
    ColFg,
    ColBg
};

typedef struct {
    unsigned w;
    unsigned h;
    Display *dpy;
    int screen;
    Window root;
    Drawable drawable;
    GC gc;
    Clr *scheme;
    Fnt *fonts;
} Drw;

// Drawable abstraction
Drw *drw_create(Display *dpy, const int screen, const Window root, const unsigned w, const unsigned h);
void drw_free(Drw *drw);
void drw_resize(Drw *drw, const unsigned w, const unsigned h);

// Font abstraction
Fnt *drw_fontset_create(Drw* drw, const char **fonts, const size_t fontcount);
void drw_fontset_free(Fnt* set);
void drw_setfontset(Drw *drw, Fnt *set);
unsigned drw_fontset_getwidth(Drw *drw, const char *text);
unsigned drw_fontset_getwidth_clamp(Drw *drw, const char *text, const unsigned n);
void drw_font_getexts(const Fnt *font, const char *text, const unsigned len, unsigned *w, unsigned *h);

// Colorscheme abstraction
void drw_clr_create(const Drw *drw, Clr *dest, const char *clrname);
Clr *drw_scm_create(const Drw *drw, const char **clrnames, const size_t clrcount);
void drw_setscheme(Drw *drw, Clr *scm);

// Cursor abstraction
Cur *drw_cur_create(const Drw *drw, const int shape);
void drw_cur_free(const Drw *drw, Cur *cursor);

// Drawing functions
void drw_rect(
    const Drw *drw,
    const int x,
    const int y,
    const unsigned w,
    const unsigned h,
    const bool filled,
    const bool invert
);
int drw_text(
    Drw *drw,
    const int x,
    const int y,
    const unsigned w,
    const unsigned h,
    const unsigned lpad,
    const char *text,
    const bool invert
);

// Map functions
void drw_map(
    const Drw *drw,
    const Window win,
    const int x,
    const int y,
    const unsigned w,
    const unsigned h
);
