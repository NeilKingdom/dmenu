/*
   Equivallent to setting dmenu's -b option.
   Expands list from bottom of screen if set to 0.
   Expands list from top of screen if set to 1.
*/
static int topbar = 1;

/*
   Equivallent to setting dmenu's -fn option.
   Setting -fn on the command line overrides fonts
   defined here. Additional fonts proceeding the
   primary font are used as fallbacks. See my ST
   config.def.h for more info on the specific
   fonts that were chosen.
*/
static const char *fonts[2] = {
	"Terminus:style=Bold:size=14:antialias=true",
   "JoyPixels:style=Regular:pixelsize=14"
};

/* Equivallent to setting dmenu's -p  option */
static const char *prompt = NULL;
static const char *colors[SchemeLast][2] = {
/*                              FG         BG       */
	[SchemeNorm]            = { "#BBBBBB", "#222222" },
	[SchemeSel]             = { "#EEEEEE", "#005577" },
   [SchemeSelHighlight]    = { "#FFC978", "#005577" },
   [SchemeNormHighlight]   = { "#FFC978", "#222222" },
	[SchemeOut]             = { "#000000", "#00FFFF" },
   [SchemeOutHighlight]    = { "#FFC978", "#00FFFF" },
};

/*
   Equivallent to setting dmenu's -l and -g options (-g added with grid patch).
   If non-zero, dmenu displays "lines" number of rows, and "columns" number of columns.
*/
static unsigned int lines   = 6;
static unsigned int columns = 4;

/*
   Characters not considered part of a word while
   deleting words. For example: " /?\"&[]"
*/
static const char worddelimiters[] = " ";

/* Size of the window border */
static unsigned int border_width = 5;

/* x/y offsets and window width */
static int dmx          = 0;    /* Put dmenu at this x offset */
static int dmy          = 0;    /* Put dmenu at this y offset (measured from the bottom if topbar is 0) */
static unsigned int dmw = 2000; /* Make dmenu this wide */
