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
   primary font are used as fallbacks.
*/
static const char *fonts[] = {
	"Terminus:size=28:antialias=true:autohint=true",
   "Noto Color Emoji:size=28:antialias=true:autohint=true",
};

/* Equivallent to setting dmenu's -p  option */
static const char *prompt = NULL;
static const char *colors[SchemeLast][2] = {
/*                  FG         BG       */
	[SchemeNorm] = { "#BBBBBB", "#222222" },
	[SchemeSel]  = { "#EEEEEE", "#005577" },
	[SchemeOut]  = { "#000000", "#00FFFF" },
};

/*
   Equivallent to setting dmenu's -l option.
   If non-zero, dmenu uses vertical list with given number of lines.
*/
static unsigned int lines = 0;

/*
   Characters not considered part of a word while
   deleting words. For example: " /?\"&[]"
*/
static const char worddelimiters[] = " ";

/* Size of the window border */
static unsigned int border_width = 5;
