// See LICENSE file for copyright and license details
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "drw.h"
#include "util.h"

#define UTF_INVALID 0xFFFD
#define UTF_SIZ     4

static const unsigned char utfbyte[UTF_SIZ + 1] = { 0x80,     0,    0xC0,  0xE0,   0xF0 };
static const unsigned char utfmask[UTF_SIZ + 1] = { 0xC0,     0x80, 0xE0,  0xF0,   0xF8 };
static const long utfmin[UTF_SIZ + 1]           = { 0,        0,    0x80,  0x800,  0x10000 };
static const long utfmax[UTF_SIZ + 1]           = { 0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF };

/*==========================
      Private Functions
===========================*/

/**
 * Decode a single byte of a UTF-8 codepoint.
 *
 * The byte to be decoded is specified by idx.
 * @param[in] c The single character to be decoded
 * @param[in/out] idx The offset of c relative to the codepoint being decoded
 * @returns The decoded value of the byte or 0 if idx > UTF_SIZ
 */
static long utf8decodebyte(const char c, size_t *idx) {
    for (*idx = 0; *idx < (UTF_SIZ + 1); ++(*idx)) {
        if (((unsigned char)c & utfmask[*idx]) == utfbyte[*idx]) {
            return (unsigned char)c & ~utfmask[*idx];
        }
    }

    return 0;
}

/**
 * Validate whether a UTF-8 codepoint is within the proper range.
 *
 * @param[in/out] u The UTF-8 codepoint to be validated
 * @param[in/out] len Length of the UTF-8 codepoint
 * @returns Length of the UTF-8 codepoint
 */
static size_t utf8validate(long *u, const size_t len) {
    if (!BETWEEN(*u, utfmin[len], utfmax[len]) || BETWEEN(*u, 0xD800, 0xDFFF)) {
        *u = UTF_INVALID;
    }

    size_t idx;
    for (idx = 1; *u > utfmax[idx]; ++idx) ;
    return idx;
}

/**
 * Decode a UTF-8 encoded string.
 *
 * @param[in] s A UTF-8 encoded string
 * @param[in/out] u Reference to a variable where the decoded UTF-8 codepoint will be output
 * @param[in] slen The length of the UTF-8 encoded string
 * @returns
 */
static size_t utf8decode(const char *s, long *u, size_t slen) {
    size_t i, j, len, type;
    long udecoded;

    *u = UTF_INVALID;
    if (!slen) {
        return 0;
    }

    udecoded = utf8decodebyte(s[0], &len);
    if (!BETWEEN(len, 1, UTF_SIZ)) {
        return 1;
    }

    for (i = 1, j = 1; i < slen && j < len; ++i, ++j) {
        udecoded = (udecoded << 6) | utf8decodebyte(s[i], &type);
        if (type) {
            return j;
        }
    }

    if (j < len) {
        return 0;
    }

    *u = udecoded;
    utf8validate(u, len);

    return len;
}

/**
 * Create an X font based on the selected font name or font pattern.
 *
 * @param[in/out] drw Reference to a Drw context object
 * @param[in] fontname Optional string that contains the name of the font
 * @param[in] fontpattern Optional pattern that describes the font
 * @returns The closest matching font if it exists, otherwise returns NULL
 */
static Fnt *xfont_create(Drw *drw, const char *fontname, FcPattern *fontpattern) {
    XftFont *xfont = NULL;
    FcPattern *pattern = NULL;

    if (fontname) {
        /*
            Using the pattern found at font->xfont->pattern does not yield the
            same substitution results as using the pattern returned by
            FcNameParse; using the latter results in the desired fallback
            behaviour whereas the former just results in missing-character
            rectangles being drawn, at least with some fonts.
        */
        if (!(xfont = XftFontOpenName(drw->dpy, drw->screen, fontname))) {
            fprintf(stderr, "error, cannot load font from name: '%s'\n", fontname);
            return NULL;
        }

        if (!(pattern = FcNameParse((FcChar8 *) fontname))) {
            fprintf(stderr, "error, cannot parse font name to pattern: '%s'\n", fontname);
            XftFontClose(drw->dpy, xfont);
            return NULL;
        }
    } else if (fontpattern) {
        if (!(xfont = XftFontOpenPattern(drw->dpy, fontpattern))) {
            fprintf(stderr, "error, cannot load font from pattern.\n");
            return NULL;
        }
    } else {
        die("no font specified.");
    }

    Fnt *font = ecalloc(1, sizeof(Fnt));
    font->xfont = xfont;
    font->pattern = pattern;
    font->h = xfont->ascent + xfont->descent;
    font->dpy = drw->dpy;

    return font;
}

/**
 * Destroy an X font.
 *
 * @param[in] font The font to deallocate
 */
static void xfont_free(Fnt *font) {
    if (!font) {
        return;
    }

    if (font->pattern) {
        FcPatternDestroy(font->pattern);
    }
    XftFontClose(font->dpy, font->xfont);
    free(font);
}

/*==========================
       Public Functions
===========================*/

/**
 * Create basic canvas with X display, screen, and pixmap.
 *
 * @param[in] dpy X display connection
 * @param[in] screen Screen ID
 * @param[in] root Parent window
 * @param w The initial width of the canvas
 * @param h The initial height of the canvas
 */
Drw *drw_create(
    Display *dpy,
    const int screen,
    const Window root,
    const unsigned w,
    const unsigned h
) {
    Drw *drw = ecalloc(1, sizeof(Drw));
    drw->dpy = dpy;
    drw->screen = screen;
    drw->root = root;
    drw->w = w;
    drw->h = h;
    drw->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
    drw->gc = XCreateGC(dpy, root, 0, NULL);
    XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);

    return drw;
}

/**
 * Destroy the canvas.
 *
 * @param[in] drw Reference to the Drw object to be destroyed
 */
void drw_free(Drw *drw) {
    XFreePixmap(drw->dpy, drw->drawable);
    XFreeGC(drw->dpy, drw->gc);
    drw_fontset_free(drw->fonts);
    free(drw);
}

/**
 * Resize the canvas.
 *
 * @param[in] drw Reference to the Drw object to resize
 * @param[in] w New width of the canvas
 * @param[in] h New height of the canvas
 */
void drw_resize(Drw *drw, const unsigned w, const unsigned h) {
    if (!drw) {
        return;
    }

    drw->w = w;
    drw->h = h;
    if (drw->drawable) {
        XFreePixmap(drw->dpy, drw->drawable);
    }
    drw->drawable = XCreatePixmap(drw->dpy, drw->root, w, h, DefaultDepth(drw->dpy, drw->screen));
}

/**
 * Create and bundle a set of fonts.
 *
 * @param[in/out] drw Reference to a Drw context object
 * @param[in] fonts Pointer to a list of font names
 * @param[in] fontcount The number of fonts to bundle
 * @returns A pointer to a block of contiguous memory containing the fonts
 */
Fnt *drw_fontset_create(Drw *drw, const char **fonts, const size_t fontcount) {
    Fnt *cur, *ret = NULL;

    if (!drw || !fonts) {
        return NULL;
    }

    for (size_t i = 1; i <= fontcount; i++) {
        if ((cur = xfont_create(drw, fonts[fontcount - i], NULL))) {
            cur->next = ret;
            ret = cur;
        }
    }

    return (drw->fonts = ret);
}

/**
 * Destroy a set of X fonts.
 *
 * @param[in] set The set of fonts to be destroyed
 */
void drw_fontset_free(Fnt *set) {
    if (set) {
        drw_fontset_free(set->next);
        xfont_free(set);
    }
}

/**
 * Sets the active font set.
 *
 * @param[in/out] drw Reference to a Drw context object
 * @param[in] set The font set to apply as active
 */
void drw_setfontset(Drw *drw, Fnt *set) {
    if (drw) drw->fonts = set;
}

/**
 * Returns the width of the text (accounts for font).
 *
 * @param[in] drw Reference to a Drw context object
 * @param[in] text The text who's width shall be returned
 */
unsigned drw_fontset_getwidth(Drw *drw, const char *text) {
    if (!drw || !drw->fonts || !text) {
        return 0;
    }
    return drw_text(drw, 0, 0, 0, 0, 0, text, 0);
}

/**
 * Returns the width of the text (accounts for font).
 * This variant of the function clamps to a max width of n.
 *
 * @param[in] drw Reference to a Drw context object
 * @param[in] text The text who's width shall be returned
 * @param[in] n Max width i.e. clamp ceiling
 * @returns The width of text or n if width > n
 */
unsigned drw_fontset_getwidth_clamp(Drw *drw, const char *text, const unsigned n) {
    unsigned tmp = 0;
    if (drw && drw->fonts && text && n) {
        tmp = drw_text(drw, 0, 0, 0, 0, 0, text, n);
    }

    return MIN(n, tmp);
}

/**
 * Get maximum dimensions of an X font.
 *
 * @param[in] font The font to get extents for
 * @param[in] text UTF-8 encoded text
 * @param[in] Length of the text in characters
 * @param[in/out] w The maximum width of text factoring in font
 * @param[in/out] h The maximum height of text factoring in font
 */
void drw_font_getexts(
    const Fnt *font,
    const char *text,
    const unsigned len,
    unsigned *w,
    unsigned *h
) {
    XGlyphInfo ext;

    if (!font || !text) {
        return;
    }

    XftTextExtentsUtf8(font->dpy, font->xfont, (XftChar8 *)text, len, &ext);
    if (w) {
        *w = ext.xOff;
    }
    if (h) {
        *h = font->h;
    }
}

/**
 * Create a single X colorscheme.
 *
 * @param[in/out] drw Reference to the drw object
 * @param[out] Pointer to location where the colorscheme will be output
 * @param[in] Name associated with this colorscheme
 */
void drw_clr_create(const Drw *drw, Clr *dest, const char *clrname) {
    if (!drw || !dest || !clrname) {
        return;
    }

    if (!XftColorAllocName(
        drw->dpy,
        DefaultVisual(drw->dpy, drw->screen),
        DefaultColormap(drw->dpy, drw->screen),
        clrname,
        dest)
    ) {
        die("error, cannot allocate color '%s'", clrname);
    }
}

/*
 * Wrapper to create color schemes.
 *
 * @warn The caller has to call free(3) on the returned color scheme when done using it.
 * @param[in/out] drw Reference to the Drw object
 * @param[in] clrnames List of names for each colorscheme
 * @param[in] clrcount Number of colorschemes to create
 * @returns Pointer to array of colorschemes
 */
Clr *drw_scm_create(const Drw *drw, const char **clrnames, const size_t clrcount) {
    // Need at least two colors for a scheme
    if (!drw || !clrnames || clrcount < 2) {
        return NULL;
    }

    Clr *ret = ecalloc(clrcount, sizeof(XftColor));
    if (!ret) {
        return NULL;
    }

    for (size_t i = 0; i < clrcount; i++) {
        drw_clr_create(drw, &ret[i], clrnames[i]);
    }

    return ret;
}

/**
 * Sets the active color scheme.
 *
 * @param[in/out] drw Reference to a Drw context object
 * @param[in] set The font set to apply as active
 */
void drw_setscheme(Drw *drw, Clr *scm) {
    if (drw) drw->scheme = scm;
}

/**
 * Creates an X cursor.
 *
 * @param[in/out] drw Reference to a Drw context object
 * @param[in] shape Integer which maps to the desired X cursor shape
 * @returns The newly created cursor object
 */
Cur *drw_cur_create(const Drw *drw, const int shape) {
    Cur *cur;

    if (!drw || !(cur = ecalloc(1, sizeof(Cur)))) {
        return NULL;
    }

    cur->cursor = XCreateFontCursor(drw->dpy, shape);
    return cur;
}

/**
 * Destroys an X cursor.
 *
 * @param[in] drw Reference to the Drw object whos cursor shall be deallocated
 * @param[in] cursor The cursor to be destroyed
 */
void drw_cur_free(const Drw *drw, Cur *cursor) {
    if (!cursor) {
        return;
    }

    XFreeCursor(drw->dpy, cursor->cursor);
    free(cursor);
}

/**
 * Draws a primitive rectangle with an optional border.
 *
 * @param[in] drw Reference to a Drw context object
 * @param[in] x The X offset of the rectangle's top-left corner
 * @param[in] y The y offset of the rectangle's top-left corner
 * @param[in] w The width of the rectangle
 * @param[in] h The height of the rectangle
 * @param[in] filled True if no border is desired
 * @param[in] invert True if the fg and bg colors of the scheme should be inverted
 */
void drw_rect(
    const Drw *drw,
    const int x,
    const int y,
    const unsigned w,
    const unsigned h,
    const bool filled,
    const bool invert
) {
    if (!drw || !drw->scheme) {
        return;
    }

    XSetForeground(drw->dpy, drw->gc, invert ? drw->scheme[ColBg].pixel : drw->scheme[ColFg].pixel);
    if (filled) {
        XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
    } else {
        XDrawRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w - 1, h - 1);
    }
}

/**
 * Renders the text specified by __text__ using an appropriate font.
 *
 * @param[in/out] drw Reference to a Drw context object
 * @param[in] x The x offset of where the text will be rendered
 * @param[in] y The y offset of where the text will be rendered
 * @param[in] w The maximum width of the text (ellipses will be used to truncate trailing text)
 * @param[in] h The maximum number of lines of text to draw
 * @param[in] lpad Number of whitespace characters to left-pad the text with
 * @param[in] text The text to render
 * @param[in] invert True if the fg and bg colors should be inverted
 * @returns The width from the left edge of the screen to the end of the text
 */
int drw_text(
    Drw *drw,
    const int x,
    const int y,
    const unsigned w,
    const unsigned h,
    const unsigned lpad,
    const char *text,
    const bool invert
) {
    unsigned _w = w;
    unsigned _x = x;

    int ty, ellipsis_x = 0;
    unsigned tmpw, ew, ellipsis_w = 0, ellipsis_len;
    XftDraw *d = NULL;
    Fnt *usedfont, *curfont, *nextfont;
    int utf8strlen, utf8charlen, render = _x || y || _w || h;
    long utf8codepoint = 0;
    const char *utf8str;
    FcCharSet *fccharset;
    FcPattern *fcpattern;
    FcPattern *match;
    XftResult result;
    int charexists = 0, overflow = 0;

    // Keep track of a couple codepoints for which we have no match
    enum {
        nomatches_len = 64
    };

    static struct {
        long codepoint[nomatches_len];
        unsigned idx;
    } nomatches;

    static unsigned ellipsis_width = 0;

    if (!drw || (render && (!drw->scheme || !_w)) || !text || !drw->fonts) {
        return 0;
    }

    if (!render) {
        _w = invert ? invert : ~invert;
    } else {
        XSetForeground(drw->dpy, drw->gc, drw->scheme[invert ? ColFg : ColBg].pixel);
        XFillRectangle(drw->dpy, drw->drawable, drw->gc, _x, y, _w, h);
        d = XftDrawCreate(
            drw->dpy, drw->drawable,
            DefaultVisual(drw->dpy, drw->screen),
            DefaultColormap(drw->dpy, drw->screen)
        );
        _x += lpad;
        _w -= lpad;
    }

    usedfont = drw->fonts;
    if (!ellipsis_width && render) {
        ellipsis_width = drw_fontset_getwidth(drw, "...");
    }

    while (true) {
        ew = ellipsis_len = utf8strlen = 0;
        utf8str = text;
        nextfont = NULL;

        while (*text) {
            utf8charlen = utf8decode(text, &utf8codepoint, UTF_SIZ);
            for (curfont = drw->fonts; curfont; curfont = curfont->next) {
                charexists = charexists || XftCharExists(drw->dpy, curfont->xfont, utf8codepoint);
                if (charexists) {
                    drw_font_getexts(curfont, text, utf8charlen, &tmpw, NULL);
                    if (ew + ellipsis_width <= _w) {
                        // Keep track where the ellipsis still fits
                        ellipsis_x = _x + ew;
                        ellipsis_w = _w - ew;
                        ellipsis_len = utf8strlen;
                    }

                    if (ew + tmpw > _w) {
                        overflow = 1;
                        // Called from drw_fontset_getwidth_clamp(): it wants the width AFTER the overflow
                        if (!render) {
                            _x += tmpw;
                        } else {
                            utf8strlen = ellipsis_len;
                        }
                    } else if (curfont == usedfont) {
                        utf8strlen += utf8charlen;
                        text += utf8charlen;
                        ew += tmpw;
                    } else {
                        nextfont = curfont;
                    }
                    break;
                }
            }

            if (overflow || !charexists || nextfont) {
                break;
            } else {
                charexists = 0;
            }
        }

        if (utf8strlen) {
            if (render) {
                ty = y + (h - usedfont->h) / 2 + usedfont->xfont->ascent;
                XftDrawStringUtf8(
                    d, &drw->scheme[invert ? ColBg : ColFg],
                    usedfont->xfont, _x, ty, (XftChar8 *)utf8str, utf8strlen
                );
            }
            _x += ew;
            _w -= ew;
        }

        if (render && overflow) {
            drw_text(drw, ellipsis_x, y, ellipsis_w, h, 0, "...", invert);
        }

        if (!*text || overflow) {
            break;
        } else if (nextfont) {
            charexists = 0;
            usedfont = nextfont;
        } else {
            // Regardless of whether or not a fallback font is found, the character must be drawn
            charexists = 1;

            for (int i = 0; i < nomatches_len; ++i) {
                // Avoid calling XftFontMatch if we know we won't find a match
                if (utf8codepoint == nomatches.codepoint[i]) {
                    goto no_match;
                }
            }

            fccharset = FcCharSetCreate();
            FcCharSetAddChar(fccharset, utf8codepoint);

            if (!drw->fonts->pattern) {
                // Refer to the comment in xfont_create for more information
                die("the first font in the cache must be loaded from a font string.");
            }

            fcpattern = FcPatternDuplicate(drw->fonts->pattern);
            FcPatternAddCharSet(fcpattern, FC_CHARSET, fccharset);
            FcPatternAddBool(fcpattern, FC_SCALABLE, FcTrue);

            FcConfigSubstitute(NULL, fcpattern, FcMatchPattern);
            FcDefaultSubstitute(fcpattern);
            match = XftFontMatch(drw->dpy, drw->screen, fcpattern, &result);

            FcCharSetDestroy(fccharset);
            FcPatternDestroy(fcpattern);

            if (match) {
                usedfont = xfont_create(drw, NULL, match);
                if (usedfont && XftCharExists(drw->dpy, usedfont->xfont, utf8codepoint)) {
                    for (curfont = drw->fonts; curfont->next; curfont = curfont->next) ;
                    curfont->next = usedfont;
                } else {
                    xfont_free(usedfont);
                    nomatches.codepoint[++nomatches.idx % nomatches_len] = utf8codepoint;
no_match:
                    usedfont = drw->fonts;
                }
            }
        }
    }

    if (d) {
        XftDrawDestroy(d);
    }

    return _x + (render ? _w : 0);
}

/**
 * Blit pixmap to the screen.
 *
 * @param[in] drw Reference to a Drw context object
 * @param[in] win The window to draw to
 * @param[in] x The x offset into the window
 * @param[in] y The y offset into the window
 * @param[in] w The width of the canvas starting from x offset to copy to dest (win)
 * @param[in] h The height of the canvas starting from y offset to copy to dest (win)
 */
void drw_map(
    const Drw *drw,
    const Window win,
    const int x,
    const int y,
    const unsigned w,
    const unsigned h
) {
    if (!drw) {
        return;
    }

    XCopyArea(drw->dpy, drw->drawable, win, drw->gc, x, y, w, h, x, y);
    XSync(drw->dpy, False);
}
