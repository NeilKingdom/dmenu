```
     _
  __| |_ __ ___   ___ _ __  _   _
 / _` | '_ ` _ \ / _ \ '_ \| | | |
| (_| | | | | | |  __/ | | | |_| |
 \__,_|_| |_| |_|\___|_| |_|\__,_|
```

---

# About

dmenu is a program created by the [suckless](suckless.org) team. You can pipe strings into it to select from.
dmenu delimits tokens on newline characters. By default the option which is selected gets printed to stdout.
The dmenu_run script will run the user's selection in the shell as a daemon.

> [!NOTE]
> This is my own heavily edited build of dmenu. The patch command will likely not work for applying new patches.
> I add patches manually line by line.

# Build Instructions

### To build dmenu:

```console
make clean
sudo make install
```

Binaries are output to /usr/local/bin (this includes dmenu, dmenu_run, dmenu_path, and stest).

### To uninstall dmenu:

```console
sudo make uninstall
```

# My Patches

Here's a list of the patches I've applied:

- [border](https://tools.suckless.org/dmenu/patches/border/): Allows user to specify a border width for the
dmenu context window.
- [caseinsensitive](https://tools.suckless.org/dmenu/patches/case-insensitive/): Let's dmenu ignore casing
when matching text entry.
- [grid](https://tools.suckless.org/dmenu/patches/grid/): Allows for adding additional columns.
- [gridnav](https://tools.suckless.org/dmenu/patches/gridnav/): Let's user navigate left/right when more than
one columns are present (must be applied after grid patch).
- [highlight](https://tools.suckless.org/dmenu/patches/highlight/): Highlights text for incremental matches as
user enters text.
- [linesbelowprompt](https://tools.suckless.org/dmenu/patches/lines-below-prompt/): Aligns options to the
left-hand side of the window, below the prompt.
- [numbers](https://tools.suckless.org/dmenu/patches/numbers/): Prints the number of matches at the top right.
- [xyw](https://tools.suckless.org/dmenu/patches/xyw/): Let's you specify the (x, y) offset of the window, as
well as the width.

# Usage

Assuming /usr/local/bin is in your $PATH, you should just be able to run dmenu or dmenu_run from the CLI.
Check the man page for specific options that are available to you (my build contains non-standard options from
patches that I've applied).

Example usage:

```console
printf "%s\n%s\n%s\n" "Option 1" "Option 2" "Option 3" | dmenu -p "Select an option"
```

---

# Acknowledging Our Lord

✝ May these works glorify and honour Christ, our Lord ✝

_“So whether you eat or drink or whatever you do, do it all for the glory of God.”_ – 1 Corinthians 10:31
