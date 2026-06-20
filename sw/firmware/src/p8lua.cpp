#include "p8lua.h"
#include "luabind.h"
#include "lua/llimits.h"
#include <utility>
#include <cmath>
#include <cstring>
#include <algorithm>

int camera_offset_x = 0;
int camera_offset_y = 0;
int clip_x0 = 0;
int clip_y0 = 0;
int clip_x1 = 128;
int clip_y1 = 128;
int current_color = 0;

static uint8_t p8lua_pal[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
uint8_t p8lua_screen[128*128];
uint8_t p8lua_button_mask;
uint8_t* p8lua_card_data;
float p8lua_time;

static void drawTile(int x, int y, int tile_nr)
{
    if (x < -8) return;
    if (y < -8) return;
    if (x > 127) return;
    if (y > 127) return;
    auto ptr = p8lua_card_data + (tile_nr % 16) * 4 + (tile_nr / 16) * 512;
    for(int py=0; py<8; py++) {
        for(int px=0; px<8; px++) {
            int ppx = px + x;
            int ppy = py + y;
            if (ppx >= clip_x0 && ppx < clip_x1 && ppy >= clip_y0 && ppy < clip_y1) {
                auto c = ptr[px/2 + py*64];
                if (px & 1) c >>= 4; else c &= 0x0F;
                if (c) {
                    p8lua_screen[ppx + ppy * 128] = p8lua_pal[c];
                }
            }
        }
    }
}

LuaBindError lua_p8_load(const char* FILENAME, std::optional<const char*> breadcrumb, std::optional<const char*> param_str) {
    return {"Not implemented"};
}
LuaBindError lua_p8_save(const char* FILENAME) {
    return {"Not implemented"};
}
/*
Load or save a cartridge

When loading from a running cartridge, the loaded cartridge is immediately run with parameter string PARAM_STR (accessible with STAT(6)), and a menu item is inserted and named BREADCRUMB, that returns the user to the previous cartridge.

Filenames that start with '#' are taken to be a BBS cart id, that is immediately downloaded and run:

> LOAD("#MYGAME_LEVEL2", "BACK TO MAP", "LIVES="..LIVES)

If the id is the cart's parent post, or a revision number is not specified, then the latest version is fetched. BBS carts can be loaded from other BBS carts or local carts, but not from exported carts.

FOLDER

Open the carts folder in the host operating system.
*/
LuaBindError lua_p8_ls(std::optional<const char*> directory) {
    return {"Not implemented"};
}
/*
List .p8 and .p8.png files in given directory (folder), relative to the current directory. Items that are directories end in a slash (e.g. "foo/").

When called from a running cartridge, LS can only be used locally and returns a table of the results. When called from a BBS cart, LS returns nil.

Directories can only resolve inside of PICO-8's virtual drive; LS("..") from the root directory will resolve to the root directory.
*/

LuaBindError lua_p8_run(std::optional<const char*> param_str) {
    return {"Not implemented"};
}
/*
Run from the start of the program.

RUN() Can be called from inside a running program to reset.

When PARAM_STR is supplied, it can be accessed during runtime with STAT(6)
*/
LuaBindError lua_p8_stop(std::optional<const char*> message) {
    return {"Not implemented"};
}
/*
Stop the cart and optionally print a message.

RESUME

Resume the program. Use R for short.

Use a single "." from the commandline to advance a single frame. This enters frame-by-frame mode, that can be read with stat(110). While frame-by-frame mode is active, entering an empty command (by pressing enter) advances one frames.
*/
LuaBindError lua_p8_assert(bool condition, std::optional<const char*> message) {
    if (!condition) return {message.value_or("Assertion failed")};
    return {};
}
/*
If CONDITION is false, stop the program and print MESSAGE if it is given. This can be useful for debugging cartridges, by ASSERT()'ing that things that you expect to be true are indeed true.

ASSERT(ADDR >= 0 AND ADDR <= 0x7FFF, "OUT OF RANGE")
POKE(ADDR, 42) -- THE MEMORY ADDRESS IS OK, FOR SURE!

REBOOT

Reboot the machine Useful for starting a new project

RESET()

Reset the values in RAM from 0x5f00..0x5f7f to their default values. This includes the palette, camera position, clipping and fill pattern. If you get lost at the command prompt because the draw state makes viewing text impossible, try typing RESET! It can also be called from a running program.

INFO()

Print out some information about the cartridge: Code size, tokens, compressed size

Also displayed:

UNSAVED CHANGES   When the cartridge in memory differs to the one on disk
EXTERNAL CHANGES  When the cartridge on disk has changed since it was loaded
  (e.g. by editing the program using a separate text editor)

FLIP()

Flip the back buffer to screen and wait for next frame. This call is not needed when there is a _DRAW() or _UPDATE() callback defined, as the flip is performed automatically. But when using a custom main loop, a call to FLIP is normally needed:

::_::
CLS()
FOR I=1,100 DO
  A=I/50 - T()
  X=64+COS(A)*I
  Y=64+SIN(A)*I
  CIRCFILL(X,Y,1,8+(I/4)%8)
END
FLIP()GOTO _

If your program does not call FLIP before a frame is up, and a _DRAW() callback is not in progress, the current contents of the back buffer are copied to screen.

PRINTH(STR, [FILENAME], [OVERWRITE], [SAVE_TO_DESKTOP])

Print a string to the host operating system's console for debugging.

If filename is set, append the string to a file on the host operating system (in the current directory by default -- use FOLDER to view).

Setting OVERWRITE to true causes that file to be overwritten rather than appended.

Setting SAVE_TO_DESKTOP to true saves to the desktop instead of the current path.

Use a filename of "@clip" to write to the host's clipboard.
ⓘ

Use stat(4) to read the clipboard, but the contents of the clipboard are only available after pressing CTRL-V during runtime (for security).
*/
float lua_p8_time() {
    return p8lua_time;
}
float lua_p8_t() {
    return p8lua_time;
}
/*
Returns the number of seconds elapsed since the cartridge was run.

This is not the real-world time, but is calculated by counting the number of times

_UPDATE or @_UPDATE60 is called. Multiple calls of TIME() from the same frame return

the same result.

STAT(X)

Get system status where X is:

0  Memory usage (0..2048)
1  CPU used since last flip (1.0 == 100% CPU)
4  Clipboard contents (after user has pressed CTRL-V)
6  Parameter string
7  Current framerate
 
46..49  Index of currently playing SFX on channels 0..3
50..53  Note number (0..31) on channel 0..3
54      Currently playing pattern index
55      Total patterns played
56      Ticks played on current pattern
57      (Boolean) TRUE when music is playing
 
80..85  UTC time: year, month, day, hour, minute, second
90..95  Local time
 
100     Current breadcrumb label, or nil
110     Returns true when in frame-by-frame mode
ⓘ

Audio values 16..26 are the legacy version of audio state queries 46..56. They only report on the current state of the audio mixer, which changes only ~20 times a second (depending on the host sound driver and other factors). 46..56 instead stores a history of mixer state at each tick to give a higher resolution estimate of the currently audible state.

EXTCMD(CMD_STR, [P1, P2])

Special system command, where CMD_STR is a string:

"pause"         request the pause menu be opened
"reset"         request a cart reset
"go_back"       return to the previous cart if there is one
"label"         set cart label to contents of screen
"screen"        save a screenshot
"rec"           set video start point
"rec_frames"    set video start point in frames mode
"video"         save a .gif to desktop
"audio_rec"     start recording audio
"audio_end"     save recorded audio to desktop (no supported from web)
"shutdown"      quit cartridge (from exported binary)
"folder"        open current working folder on the host operating system
"set_filename"  set the filename for screenshots / gifs / audio recordings
"set_title"     set the host window title

Some commands have optional number parameters:

"video" and "screen": P1: an integer scaling factor that overrides the system setting. P2: when > 0, save to the current folder instead of to desktop

"audio_end" P1: when > 0, save to the current folder instead of to desktop
■ Recording GIFs

EXTCMD("REC"), EXTCMD("VIDEO") is the same as using ctrl-8, ctrl-9 and saves a gif to the desktop using the current GIF_SCALE setting (use CONFIG GIF_SCALE to change).

The two additional parameters can be used to override these defaults:

EXTCMD("VIDEO", 4)    -- SCALE *4 (512 X 512)
EXTCMD("VIDEO", 0, 1) -- DEFAULT SCALING, SAVE TO USER DATA FOLDER

The user data folder can be opened with EXTCMD("FOLDER") and defaults to the same path as the cartridge, or {pico-8 appdata}/appdata/appname for exported binaries.

Due to the nature of the gif format, all gifs are recorded at 33.3fps, and frames produced by PICO-8 are skipped or duplicated in the gif to match roughly what the user is seeing. To record exactly one frame each time FLIP() is called, regardless of the runtime framerate or how long it took to generate the frame, use:

EXTCMD("REC_FRAMES")

The default filename for gifs (and screenshots, audio) is foo_%d, where foo is the name of the cartridge, and %d is a number starting at 0 and automatically incremented until a file of that name does not exist. Use EXTCMD("SET_FILENAME","FOO") to override that default. If the custom filename includes "%d", then the auto- incrementing number behaviour is used, but otherwise files are written even if there is an existing file with the same name.
6.2 Graphics

PICO-8 has a fixed capacity of 128 8x8 sprites, plus another 128 that overlap with the bottom half of the map data ("shared data"). These 256 sprites are collectively called the sprite sheet, and can be thought of as a 128x128 pixel image.

All of PICO-8's drawing operations are subject to the current draw state. The draw state includes a camera position (for adding an offset to all coordinates), palette mapping (for recolouring sprites), clipping rectangle, a drawing colour, and a fill pattern.

The draw state is reset each time a program is run, or by calling RESET().

Colour indexes:
 
   0  black   1  dark_blue   2  dark_purple   3  dark_green  
   4  brown   5  dark_gray   6  light_gray    7  white
   8  red     9  orange     10  yellow       11  green      
  12  blue   13  indigo     14  pink         15  peach
*/
void lua_p8_clip(std::optional<int> x, std::optional<int> y, std::optional<int> w, std::optional<int> h, std::optional<bool> clip_previous)
{
    if (clip_previous.value_or(false)) {
        clip_x0 = std::clamp(x.value_or(0), clip_x0, clip_x1);
        clip_y0 = std::clamp(y.value_or(0), clip_y0, clip_y1);
        clip_x1 = std::clamp(x.value_or(0) + w.value_or(128), clip_x0, clip_x1);
        clip_y1 = std::clamp(y.value_or(0) + h.value_or(128), clip_y0, clip_y1);
    } else {
        clip_x0 = std::clamp(x.value_or(0), 0, 128);
        clip_y0 = std::clamp(y.value_or(0), 0, 128);
        clip_x1 = std::clamp(x.value_or(0) + w.value_or(128), clip_x0, 128);
        clip_y1 = std::clamp(y.value_or(0) + h.value_or(128), clip_y0, 128);
    }
}
/*
Sets the clipping rectangle in pixels. All drawing operations will be clipped to the rectangle at x, y with a width and height of w,h.

CLIP() to reset.

When CLIP_PREVIOUS is true, clip the new clipping region by the old one.
*/
void lua_p8_pset(int x, int y, std::optional<int> col) {
    x += camera_offset_x;
    y += camera_offset_y;
    if (x >= clip_x0 && x < clip_x1 && y >= clip_y0 && y < clip_y1) {
        p8lua_screen[x + y * 128] = p8lua_pal[col.value_or(current_color)];
    }
}
/*
Sets the pixel at x, y to colour index COL (0..15).

When COL is not specified, the current draw colour is used.

FOR Y=0,127 DO
  FOR X=0,127 DO
    PSET(X, Y, X*Y/8)
  END
END

PGET(X, Y)

Returns the colour of a pixel on the screen at (X, Y).

WHILE (TRUE) DO
  X, Y = RND(128), RND(128)
  DX, DY = RND(4)-2, RND(4)-2
  PSET(X, Y, PGET(DX+X, DY+Y))
END

When X and Y are out of bounds, PGET returns 0. A custom return value can be specified with:

POKE(0x5f36, 0x10)
POKE(0x5f5B, NEWVAL)

SGET(X, Y)
SSET(X, Y, [COL])

Get or set the colour (COL) of a sprite sheet pixel.

When X and Y are out of bounds, SGET returns 0. A custom value can be specified with:

POKE(0x5f36, 0x10)
POKE(0x5f59, NEWVAL)
*/
int lua_p8_fget(lua_State* L) {
    auto n = luaL_checkinteger(L, 1);
    auto f = luaL_optinteger(L, 2, -1);
    if (f >= 0 && f < 7)
        lua_pushboolean(L, p8lua_card_data[0x3000 + n] & (1 << f));
    else
        lua_pushinteger(L, p8lua_card_data[0x3000 + n]);
    return 1;
}
/*
FSET(N, [F], VAL)

Get or set the value (VAL) of sprite N's flag F.

F is the flag index 0..7.

VAL is TRUE or FALSE.

The initial state of flags 0..7 are settable in the sprite editor, so can be used to create custom sprite attributes. It is also possible to draw only a subset of map tiles by providing a mask in MAP().

When F is omitted, all flags are retrieved/set as a single bitfield.

FSET(2, 1 | 2 | 8)   -- SETS BITS 0,1 AND 3
FSET(2, 4, TRUE)     -- SETS BIT 4
PRINT(FGET(2))       -- 27 (1 | 2 | 8 | 16)
*/
const uint8_t p8font[] = {
    #include "p8font.inc"
};

void lua_p8_print(const char* str, std::optional<int> x_or_col, std::optional<int> opt_y, std::optional<int> col) {
    //PRINT(STR, X, Y, [COL])
    //PRINT(STR, [COL])
    if (!opt_y.has_value()) {
        printf("%s\n", str);
    } else {
        int x = x_or_col.value();
        int y = opt_y.value();
        int c = col.value_or(15);
        while(*str) {
            if (*str >= 32) {
                auto ptr = p8font + (*str - 32) * 3;
                for(int px=0; px<3; px++) {
                    for(int py=0; py<5; py++) {
                        if (*ptr & (1 << py)) {
                            if (x >= clip_x0 && x < clip_x1 && (y + py) >= clip_y0 && (y + py) < clip_y1) {
                                p8lua_screen[x + (y + py) * 128] = p8lua_pal[c];
                            }
                        }
                    }
                    ptr++;
                    x++;
                }
                x++;
            }
            str++;
        }
    }
}
/*
Print a string STR and optionally set the draw colour to COL.

Shortcut: written on a single line, ? can be used to call print without brackets:

?"HI"

When X, Y are not specified, a newline is automatically appended. This can be omitted by ending the string with an explicit termination control character:

?"THE QUICK BROWN FOX\0"

Additionally, when X, Y are not specified, printing text below 122 causes the console to scroll. This can be disabled during runtime with POKE(0x5f36,0x40).

PRINT returns the right-most x position that occurred while printing. This can be used to find out the width of some text by printing it off-screen:

W = PRINT("HOGE", 0, -20) -- returns 16

See Appendix A (P8SCII) for information about control codes and custom fonts.

CURSOR(X, Y, [COL])

Set the cursor position.

If COL is specified, also set the current colour.

COLOR([COL])

Set the current colour to be used by drawing functions.

If COL is not specified, the current colour is set to 6
*/
void lua_p8_cls(std::optional<int> col) {
    memset(p8lua_screen, p8lua_pal[col.value_or(current_color) & 0x0F], sizeof(p8lua_screen));
}
/*
Clear the screen and reset the clipping rectangle.

COL defaults to 0 (black)
*/
void lua_p8_camera(std::optional<int> x, std::optional<int> y) {
    camera_offset_x = -x.value_or(0);
    camera_offset_y = -y.value_or(0);
}
/*
Set a screen offset of -x, -y for all drawing operations

CAMERA() to reset
*/
void lua_p8_circ(int x, int y, int r, std::optional<int> col) {
    //TODO
}
void lua_p8_circfill(int x, int y, int r, std::optional<int> col) {
    //TODO
}
/*
Draw a circle or filled circle at x,y with radius r

If r is negative, the circle is not drawn.

When bits 0x1800.0000 are set in COL, and 0x5F34 & 2 == 2, the circle is drawn inverted.

OVAL(X0, Y0, X1, Y1, [COL])
OVALFILL(X0, Y0, X1, Y1, [COL])

Draw an oval that is symmetrical in x and y (an ellipse), with the given bounding rectangle.

LINE(X0, Y0, [X1, Y1, [COL]])

Draw a line from (X0, Y0) to (X1, Y1)

If (X1, Y1) are not given, the end of the last drawn line is used.

LINE() with no parameters means that the next call to LINE(X1, Y1) will only set the end points without drawing.

CLS()
LINE()
FOR I=0,6 DO
  LINE(64+COS(I/6)*20, 64+SIN(I/6)*20, 8+I)
END
*/
void lua_p8_rect(int x0, int y0, int x1, int y1, std::optional<int> col) {
    x0 = std::clamp(x0+camera_offset_x, clip_x0, clip_x1);
    x1 = std::clamp(x1+camera_offset_x, clip_x0, clip_x1);
    y0 = std::clamp(y0+camera_offset_y, clip_y0, clip_y1);
    y1 = std::clamp(y1+camera_offset_y, clip_y0, clip_y1);
    auto c = col.value_or(current_color);
    for(int x=x0; x<x1; x++) {
        p8lua_screen[x + y0 * 128] = p8lua_pal[c];
        p8lua_screen[x + (y1-1) * 128] = p8lua_pal[c];
    }
    for(int y=y0; y<y1; y++) {
        p8lua_screen[x0 + y * 128] = p8lua_pal[c];
        p8lua_screen[(x1-1) + y * 128] = p8lua_pal[c];
    }
}
void lua_p8_rectfill(int x0, int y0, int x1, int y1, std::optional<int> col) {
    x0 = std::clamp(x0+camera_offset_x, clip_x0, clip_x1);
    x1 = std::clamp(x1+camera_offset_x, clip_x0, clip_x1);
    y0 = std::clamp(y0+camera_offset_y, clip_y0, clip_y1);
    y1 = std::clamp(y1+camera_offset_y, clip_y0, clip_y1);
    auto c = col.value_or(current_color);
    for(int y=y0; y<y1; y++) {
        for(int x=x0; x<x1; x++) {
            p8lua_screen[x + y * 128] = p8lua_pal[c];
        }
    }
}
/*
Draw a rectangle or filled rectangle with corners at (X0, Y0), (X1, Y1).
*/
void lua_p8_rrect(int x0, int y0, int x1, int y1, int r, std::optional<int> col) {
}
void lua_p8_rrectfill(int x0, int y0, int x1, int y1, int r, std::optional<int> col) {
}
/*
Draw a rounded rectangle or filled rectangle with rounded corners.

The width (W) and height (H) are in pixels, and must both be more than 0 for the shape to be drawn.

Radius (R) defaults to 0, and is the size of the quarter-circle to be drawn at each corner. The radius used is clamped to fall the range 0 .. min(width,height)/2.

Draw a red (colour 8) rounded rectangle 40 pixels wide and 30 pixels talls with 3 pixels missing at each corner (radius 2):

> rrectfill(50,80,40,30,2,8)

When bits 0x1800.0000 are set in COL, and (PEEK(0x5F34) & 2) == 2, RRECTFILL is drawn inverted.
*/
int lua_p8_pal(lua_State* L) {
    if (lua_gettop(L) < 1) {
        for(int n=0; n<16; n++) p8lua_pal[n] = n;
    } else {
        auto idx = luaL_checkinteger(L, 1);
        p8lua_pal[idx] = luaL_optinteger(L, 2, idx);
    }
    return 0;
}
//PAL(C0, C1, [P])
/*
PAL() swaps colour c0 for c1 for one of three palette re-mappings (p defaults to 0):

0: Draw Palette

The draw palette re-maps colours when they are drawn. For example, an orange flower sprite can be drawn as a red flower by setting the 9th palette value to 8:

PAL(9,8)     -- draw subsequent orange (colour 9) pixels as red (colour 8)
SPR(1,70,60) -- any orange pixels in the sprite will be drawn with red instead

Changing the draw palette does not affect anything that was already drawn to the screen.

1: Display Palette

The display palette re-maps the whole screen when it is displayed at the end of a frame. For example, if you boot PICO-8 and then type PAL(6,14,1), you can see all of the gray (colour 6) text immediate change to pink (colour 14) even though it has already been drawn. This is useful for screen-wide effects such as fading in/out.

2: Secondary Palette

Used by FILLP() for drawing sprites. This provides a mapping from a single 4-bit colour index to two 4-bit colour indexes.

PAL()  resets all palettes to system defaults (including transparency values)
PAL(P) resets a particular palette (0..2) to system defaults

PAL(TBL, [P])

When the first parameter of pal is a table, colours are assigned for each entry. For example, to re-map colour 12 and 14 to red:

PAL({[12]=9, [14]=8})

Or to re-colour the whole screen shades of gray (including everything that is already drawn):

PAL({1,1,5,5,5,6,7,13,6,7,7,6,13,6,7,1}, 1)

Because table indexes start at 1, colour 0 is given at the end in this case.

PALT(C, [T])

Set transparency for colour index to T (boolean) Transparency is observed by SPR(), SSPR(), MAP() AND TLINE()

PALT(8, TRUE) -- RED PIXELS NOT DRAWN IN SUBSEQUENT SPRITE/TLINE DRAW CALLS

PALT() resets to default: all colours opaque except colour 0

When C is the only parameter, it is treated as a bitfield used to set all 16 values. For example: to set colours 0 and 1 as transparent:

PALT(0B1100000000000000)
*/
void lua_p8_spr(int tile_nr, int x, int y, std::optional<int> w, std::optional<int> h, std::optional<bool> flip_x, std::optional<bool> flip_y) {
    x += camera_offset_x;
    y += camera_offset_y;
    if (x < -8) return;
    if (y < -8) return;
    if (x > 127) return;
    if (y > 127) return;
    if (tile_nr < 0 || tile_nr > 255) return;
    int flags = 0;
    if (flip_x.value_or(false)) flags |= 1;
    if (flip_y.value_or(false)) flags |= 2;
    auto ptr = p8lua_card_data + (tile_nr % 16) * 4 + (tile_nr / 16) * 512;
    for(int py=0; py<8; py++) {
        int ppy = y + py;
        if (flags & 2) ppy = y + 7 - py;
        for(int px=0; px<8; px++) {
            int ppx = x + px;
            if (flags & 1) ppx = x + 7 - px;
            if (ppx >= clip_x0 && ppx < clip_x1 && ppy >= clip_y0 && ppy < clip_y1) {
                auto c = ptr[px/2 + py*64];
                if (px & 1) c >>= 4; else c &= 0x0F;
                if (c) {
                    p8lua_screen[ppx + ppy * 128] = p8lua_pal[c];
                }
            }
        }
    }
}
/*
Draw sprite N (0..255) at position X,Y

W (width) and H (height) are 1, 1 by default and specify how many sprites wide to blit.

Colour 0 drawn as transparent by default (see PALT())

When FLIP_X is TRUE, flip horizontally.

When FLIP_Y is TRUE, flip vertically.

SSPR(SX, SY, SW, SH, DX, DY, [DW, DH], [FLIP_X], [FLIP_Y]]

Stretch a rectangle of the sprite sheet (sx, sy, sw, sh) to a destination rectangle on the screen (dx, dy, dw, dh). In both cases, the x and y values are coordinates (in pixels) of the rectangle's top left corner, with a width of w, h.

Colour 0 drawn as transparent by default (see PALT())

dw, dh defaults to sw, sh

When FLIP_X is TRUE, flip horizontally.

When FLIP_Y is TRUE, flip vertically.

FILLP(P)

The PICO-8 fill pattern is a 4x4 2-colour tiled pattern observed by: CIRC() CIRCFILL() RECT() RECTFILL() OVAL() OVALFILL() PSET() LINE()

P is a bitfield in reading order starting from the highest bit. To calculate the value of P for a desired pattern, add the bit values together:

  .-----------------------.
  |32768|16384| 8192| 4096|
  |-----|-----|-----|-----|
  | 2048| 1024| 512 | 256 |
  |-----|-----|-----|-----|
  | 128 |  64 |  32 |  16 |
  |-----|-----|-----|-----|
  |  8  |  4  |  2  |  1  |
  '-----------------------'

For example, FILLP(4+8+64+128+ 256+512+4096+8192) would create a checkerboard pattern.

This can be more neatly expressed in binary: FILLP(0b0011001111001100).

The default fill pattern is 0, which means a single solid colour is drawn.

To specify a second colour for the pattern, use the high bits of any colour parameter:

FILLP(0b0011010101101000)
CIRCFILL(64,64,20, 0x4E) -- brown and pink

Additional settings are given in bits 0b0.111:

0b0.100 Transparency

When this bit is set, the second colour is not drawn

-- checkboard with transparent squares
FILLP(0b0011001111001100.1)

0b0.010 Apply to Sprites

When set, the fill pattern is applied to sprites (spr, sspr, map, tline), using a colour mapping provided by the secondary palette.

Each pixel value in the sprite (after applying the draw palette as usual) is taken to be an index into the secondary palette. Each entry in the secondary palette contains the two colours used to render the fill pattern. For example, to draw a white and red (7 and 8) checkerboard pattern for only blue pixels (colour 12) in a sprite:

FOR I=0,15 DO PAL(I, I+I*16, 2) END  --  all other colours map to themselves
PAL(12, 0x87, 2)                     --  remap colour 12 in the secondary palette
 
FILLP(0b0011001111001100.01)         --  checkerboard palette, applied to sprites
SPR(1, 64,64)                        --  draw the sprite

0b0.001 Apply Secondary Palette Globally

When set, the secondary palette mapping is also applied by all draw functions that respect fill patterns (circfill, line etc). This can be useful when used in conjunction with sprite drawing functions, so that the colour index of each sprite pixel means the same thing as the colour index supplied to the drawing functions.

FILLP(0b0011001111001100.001)
PAL(12, 0x87, 2)
CIRCFILL(64,64,20,12)                -- red and white checkerboard circle

The secondary palette mapping is applied after the regular draw palette mapping. So the following would also draw a red and white checkered circle:

PAL(3,12)
CIRCFILL(64,64,20,3)

The fill pattern can also be set by setting bits in any colour parameter (for example, the parameter to COLOR(), or the last parameter to LINE(), RECT() etc.

POKE(0x5F34, 0x3) -- 0x1 enable fillpattern in high bits  0x2 enable inversion mode
CIRCFILL(64,64,20, 0x114E.ABCD) -- sets fill pattern to ABCD

When using the colour parameter to set the fill pattern, the following bits are used:

bit  0x1000.0000 this needs to be set: it means "observe bits 0xf00.ffff"
bit  0x0100.0000 transparency
bit  0x0200.0000 apply to sprites
bit  0x0400.0000 apply secondary palette globally
bit  0x0800.0000 invert the drawing operation (circfill/ovalfill/rectfill/rrectfill)
bits 0x00FF.0000 are the usual colour bits
bits 0x0000.FFFF are interpreted as the fill pattern
6.3 Table Functions
⚠

With the exception of PAIRS(), the following functions and the # operator apply only to tables that are indexed starting from 1 and do not have NIL entries. All other forms of tables can be considered as hash maps or sets, rather than arrays that have a length.
*/
int lua_p8_add(lua_State* L) {
    lua_Integer pos; /* where to insert new element */
    lua_Integer e = luaL_len(L, 1);
    e = luaL_intop(+, e, 1); /* first empty element */
    switch (lua_gettop(L)) {
    case 2:
    {            /* called with only 2 arguments */
        pos = e; /* insert new element at the end */
        break;
    }
    case 3:
    {
        lua_Integer i;
        pos = luaL_checkinteger(L, 2); /* 2nd argument is the position */
        /* check whether 'pos' is in [1, e] */
        luaL_argcheck(L, (lua_Unsigned)pos - 1u < (lua_Unsigned)e, 2,
                      "position out of bounds");
        for (i = e; i > pos; i--)
        { /* move up elements */
            lua_geti(L, 1, i - 1);
            lua_seti(L, 1, i); /* t[i] = t[i - 1] */
        }
        break;
    }
    default:
    {
        return luaL_error(L, "wrong number of arguments to 'ADD'");
    }
    }
    lua_seti(L, 1, pos); /* t[pos] = v */
    return 0;
}
/*
Add value VAL to the end of table TBL. Equivalent to:

TBL[#TBL + 1] = VAL

If index is given then the element is inserted at that position:

FOO={}        -- CREATE EMPTY TABLE
ADD(FOO, 11)
ADD(FOO, 22)
PRINT(FOO[2]) -- 22
*/
int lua_p8_del(lua_State* L) {
    lua_Integer size = luaL_len(L, 1);
    lua_Integer pos = -1;
    luaL_checkany(L, 2);
    for(lua_Integer pos=1; pos<=size; pos++)
    {
        lua_geti(L, 1, pos);  /* result = t[pos] */
        if (lua_equal(L, 2, -1)) {
            for ( ; pos < size; pos++) {
                lua_geti(L, 1, pos + 1);
                lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
            }
            lua_pushnil(L);
            lua_seti(L, 1, pos);  /* remove entry t[pos] */
            return 1;
        }
        lua_pop(L, 1);
    }
    return 0;
}
/*
Delete the first instance of value VAL in table TBL. The remaining entries are shifted left one index to avoid holes.

Note that VAL is the value of the item to be deleted, not the index into the table. (To remove an item at a particular index, use DELI instead). DEL returns the deleted item, or returns no value when nothing was deleted.

A={1,10,2,11,3,12}
FOR ITEM IN ALL(A) DO
  IF (ITEM < 10) THEN DEL(A, ITEM) END
END
FOREACH(A, PRINT) -- 10,11,12
PRINT(A[3])       -- 12
*/

//DELI(TBL, [I])
int lua_p8_deli(lua_State* L) {
    lua_Integer size = luaL_len(L, 1);
    lua_Integer pos = luaL_optinteger(L, 2, size);
    if (pos != size)  /* validate 'pos' if given */
        /* check whether 'pos' is in [1, size + 1] */
        luaL_argcheck(L, (lua_Unsigned)pos - 1u <= (lua_Unsigned)size, 2, "position out of bounds");
    lua_geti(L, 1, pos);  /* result = t[pos] */
    for ( ; pos < size; pos++) {
        lua_geti(L, 1, pos + 1);
        lua_seti(L, 1, pos);  /* t[pos] = t[pos + 1] */
    }
    lua_pushnil(L);
    lua_seti(L, 1, pos);  /* remove entry t[pos] */
    return 1;
}
/*
Like DEL(), but remove the item from table TBL at index I When I is not given, the last element of the table is removed and returned.
*/
int lua_p8_count(lua_State* L)
{
    //COUNT(TBL, [VAL])
    luaL_checkany(L, 1);
    lua_Integer size = luaL_len(L, 1);
    if (lua_gettop(L) < 2) {
        lua_pushnumber(L, size);
        return 1;
    }
    luaL_checkany(L, 2);

    lua_Integer count = 0;
    for (lua_Integer pos=1 ; pos <= size; pos++) {
        lua_geti(L, 1, pos);
        if (lua_equal(L, 2, -1))
            count += 1;
        lua_pop(L, 1);
    }
    return 1;
}
/*
Returns the length of table t (same as #TBL) When VAL is given, returns the number of instances of VAL in that table.
*/

int lua_p8_all(lua_State* L) {
    //ALL(TBL)
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 1);
    lua_pushnil(L);
    lua_pushcclosure(L, [](lua_State* L) -> int {
        auto idx = lua_tointeger(L, lua_upvalueindex(2));
        if (idx > 1) {
            lua_geti(L, lua_upvalueindex(1), idx - 1);
            if (!lua_rawequal(L, -1, lua_upvalueindex(3))) {
                idx -= 1;
            }
            lua_pop(L, 1);
        }
        lua_geti(L, lua_upvalueindex(1), idx);
        lua_pushinteger(L, idx + 1);
        lua_replace(L, lua_upvalueindex(2));
        lua_pushvalue(L, -1);
        lua_replace(L, lua_upvalueindex(3));
        return 1;
    }, 3);
    return 1;
}
/*
Used in FOR loops to iterate over all items in a table (that have a 1-based integer index), in the order they were added.

T = {11,12,13}
ADD(T,14)
ADD(T,"HI")
FOR V IN ALL(T) DO PRINT(V) END -- 11 12 13 14 HI
PRINT(#T) -- 5
*/
int lua_p8_foreach(lua_State* L) {
    //FOREACH(TBL, FUNC)
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    for(auto idx=1; idx<=luaL_len(L, 1); idx++) {
        lua_geti(L, 1, idx);
        lua_pushvalue(L, 2);
        lua_pushvalue(L, -2);
        lua_call(L, 1, 0);
        lua_geti(L, 1, idx);
        if (!lua_equal(L, -2, -1)) {
            idx--;
        }
        lua_pop(L, 2);
    }
    return 0;
}
/*
For each item in table TBL, call function FUNC with the item as a single parameter.

> FOREACH({1,2,3}, PRINT)
*/
int lua_p8_pairs(lua_State* L) {
    luaL_checkany(L, 1);
    if (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
        lua_pushcfunction(L, [](lua_State* L) -> int {
            luaL_checktype(L, 1, LUA_TTABLE);
            lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
            if (lua_next(L, 1))
                return 2;
            lua_pushnil(L);
            return 1;
        });  /* will return generator and */
        lua_pushvalue(L, 1);  /* state */
        lua_pushnil(L);  /* initial value */
        lua_pushnil(L);  /* to-be-closed object */
    }
    else {
        lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
        lua_callk(L, 1, 4, 0, [](lua_State *L, int status, lua_KContext k) -> int { return 4; });  /* get 4 values from metamethod */
    }
    return 4;
}
/*
Used in FOR loops to iterate over table TBL, providing both the key and value for each item. Unlike ALL(), PAIRS() iterates over every item regardless of indexing scheme. Order is not guaranteed.

T = {["HELLO"]=3, [10]="BLAH"}
T.BLUE = 5;
FOR K,V IN PAIRS(T) DO
  PRINT("K: "..K.."  V:"..V)
END

Output:

K: 10  v:BLAH
K: HELLO  v:3
K: BLUE  v:5
6.4 Input
*/
int lua_p8_btn(lua_State* L) {
    if (lua_gettop(L) == 0) {
        lua_pushinteger(L, p8lua_button_mask);
        return 1;
    }
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, p8lua_button_mask & (1 << button));
    return 1;
}
/*
Get button B state for player PL (default 0)

B: 0..5: left right up down button_o button_x
PL: player index 0..7

Instead of using a number for B, it is also possible to use a button glyph. (In the coded editor, use Shift-L R U D O X)

If no parameters supplied, returns a bitfield of all 12 button states for player 0 & 1 // P0: bits 0..5 P1: bits 8..13

Default keyboard mappings to player buttons:
 
  player 0: [DPAD]: cursors, [O]: Z C N   [X]: X V M
  player 1: [DPAD]: SFED,    [O]: LSHIFT  [X]: TAB W  Q A
⚠

Although PICO-8 accepts all button combinations, note that it is generally impossible to press both LEFT and RIGHT at the same time on a physical game controller. On some controllers, UP + LEFT/RIGHT is also awkward if [X] or [O] could be used instead of UP (e.g. to jump / accelerate).
*/
int lua_p8_btnp(lua_State* L) {
    //TODO
    if (lua_gettop(L) == 0) {
        lua_pushinteger(L, p8lua_button_mask);
        return 1;
    }
    int button = luaL_checkinteger(L, 1);
    lua_pushboolean(L, p8lua_button_mask & (1 << button));
    return 1;
}
/*
BTNP is short for "Button Pressed"; Instead of being true when a button is held down, BTNP returns true when a button is down AND it was not down the last frame. It also repeats after 15 frames, returning true every 4 frames after that (at 30fps -- double that at 60fps). This can be used for things like menu navigation or grid-wise player movement.

The state that BTNP reads is reset at the start of each call to _UPDATE or _UPDATE60, so it is preferable to use BTNP from inside one of those functions.

Custom delays (in frames 30fps) can be set by poking the following memory addresses:

POKE(0X5F5C, DELAY) -- SET THE INITIAL DELAY BEFORE REPEATING. 255 MEANS NEVER REPEAT.
POKE(0X5F5D, DELAY) -- SET THE REPEATING DELAY.

In both cases, 0 can be used for the default behaviour (delays 15 and 4)
6.5 Audio

*/
void lua_p8_sfx(int N, std::optional<int> channel, std::optional<int> offset, std::optional<int> length) {

}
/*
SFX(N, [CHANNEL], [OFFSET], [LENGTH])

Play sfx N (0..63) on CHANNEL (0..3) from note OFFSET (0..31 in notes) for LENGTH notes.

Using negative CHANNEL values have special meanings:

CHANNEL -1: (default) to automatically choose a channel that is not being used
CHANNEL -2: to stop the given sound from playing on any channel

N can be a command for the given CHANNEL (or all channels when CHANNEL < 0):

N -1: to stop sound on that channel
N -2: to release sound on that channel from looping

SFX(3)    --  PLAY SFX 3
SFX(3,2)  --  PLAY SFX 3 ON CHANNEL 2
SFX(3,-2) --  STOP SFX 3 FROM PLAYING ON ANY CHANNEL
SFX(-1,2) --  STOP WHATEVER IS PLAYING ON CHANNEL 2
SFX(-2,2) --  RELEASE LOOPING ON CHANNEL 2
SFX(-1)   --  STOP ALL SOUNDS ON ALL CHANNELS
SFX(-2)   --  RELEASE LOOPING ON ALL CHANNELS
*/
void lua_p8_music(int N, std::optional<int> fade_len, std::optional<int> channel_mask) {

}
/*
Play music starting from pattern N (0..63)
N -1 to stop music
 
FADE_LEN is in ms (default: 0). So to fade pattern 0 in over 1 second:

MUSIC(0, 1000)

CHANNEL_MASK specifies which channels to reserve for music only. For example, to play only on channels 0..2:

MUSIC(0, NIL, 7) -- 1 | 2 | 4

Reserved channels can still be used to play sound effects on, but only when that channel index is explicitly requested by SFX().
6.6 Map

The PICO-8 map is a 128x32 grid of 8-bit values, or 128x64 when using the shared memory. When using the map editor, the meaning of each value is taken to be an index into the sprite sheet (0..255). However, it can instead be used as a general block of data.
*/
int lua_p8_mget(int x, int y) {
    if (x < 0 || x >= 128) return 0;
    if (y < 0 || y >= 64) return 0;
    if (y < 32)
        return p8lua_card_data[0x2000 + x + y * 128];
    return p8lua_card_data[0x0000 + x + y * 128];
}

void lua_p8_mset(int x, int y, int val) {
    if (x < 0 || x >= 128) return;
    if (y < 0 || y >= 64) return;
    if (y < 32)
        p8lua_card_data[0x2000 + x + y * 128] = val;
    else
        p8lua_card_data[0x0000 + x + y * 128] = val;
}
/*
Get or set map value (VAL) at X,Y

When X and Y are out of bounds, MGET returns 0, or a custom return value that can be specified with:

POKE(0x5f36, 0x10)
POKE(0x5f5a, NEWVAL)

*/
void lua_p8_map(std::optional<int> tile_x, std::optional<int> tile_y, std::optional<int> sx, std::optional<int> sy, std::optional<int> tile_w, std::optional<int> tile_h, std::optional<int> layers) {
    int rx = sx.value_or(0) + camera_offset_x;
    int ry = sy.value_or(0) + camera_offset_y;
    int tx = tile_x.value_or(0);
    int ty = tile_y.value_or(0);
    int mask = layers.value_or(0xFF);

    for(int y=0; y<tile_h.value_or(16); y++) {
        for(int x=0; x<0+tile_w.value_or(16); x++) {
            int tile_nr;
            if (ty + y < 32)
                tile_nr = p8lua_card_data[0x2000 + tx + x + (ty + y) * 128];
            else
                tile_nr = p8lua_card_data[0x0000 + tx + x + (ty + y) * 128];
            if (tile_nr && p8lua_card_data[0x3000 + tile_nr] & mask)
                drawTile(rx + x * 8, ry + y * 8, tile_nr);
        }
    }
}
/*
MAP(TILE_X, TILE_Y, [SX, SY], [TILE_W, TILE_H], [LAYERS])

Draw section of map (starting from TILE_X, TILE_Y) at screen position SX, SY (pixels).

To draw a 4x2 blocks of tiles starting from 0,0 in the map, to the screen at 20,20:

MAP(0, 0, 20, 20, 4, 2)

TILE_W and TILE_H default to the entire map (including shared space when applicable).

MAP() is often used in conjunction with CAMERA(). To draw the map so that a player object (at PL.X in PL.Y in pixels) is centered:

CAMERA(PL.X - 64, PL.Y - 64)
MAP()

LAYERS is a bitfield. When given, only sprites with matching sprite flags are drawn. For example, when LAYERS is 0x5, only sprites with flag 0 and 2 are drawn.

Sprite 0 is taken to mean "empty" and is not drawn. To disable this behaviour, use: POKE(0x5F36, 0x8)

TLINE(X0, Y0, X1, Y1, MX, MY, [MDX, MDY], [LAYERS])

Draw a textured line from (X0,Y0) to (X1,Y1), sampling colour values from the map. When LAYERS is specified, only sprites with matching flags are drawn (similar to MAP())

MX, MY are map coordinates to sample from, given in tiles. Colour values are sampled from the 8x8 sprite present at each map tile. For example:

2.0, 1.0  means the top left corner of the sprite at position 2,1 on the map
2.5, 1.5  means pixel (4,4) of the same sprite

MDX, MDY are deltas added to mx, my after each pixel is drawn. (Defaults to 0.125, 0)

The map coordinates (MX, MY) are masked by values calculated by subtracting 0x0.0001 from the values at address 0x5F38 and 0x5F39. In simpler terms, this means you can loop a section of the map by poking the width and height you want to loop within, as long as they are powers of 2 (2,4,8,16..)

For example, to loop every 8 tiles horizontally, and every 4 tiles vertically:

POKE(0x5F38, 8)
POKE(0x5F39, 4)
TLINE(...)

The default values (0,0) gives a masks of 0xff.ffff, which means that the samples will loop every 256 tiles.

An offset to sample from (also in tiles) can also be specified at addresses 0x5f3a, 0x5f3b:

POKE(0x5F3A, OFFSET_X)
POKE(0x5F3B, OFFSET_Y)

Sprite 0 is taken to mean "empty" and not drawn. To disable this behaviour, use: POKE(0x5F36, 0x8)
■ Setting TLINE Precision

By default, tline coordinates (mx,my,mdx,mdy) are expressed in tiles. This means that 1 pixel is 0.125, and only 13 bits are used for the fractional part. If more precision is needed, the coordinate space can be adjusted to allow more bits for the fractional part. This can be useful for things like textured walls, where the accumulated error from mdx,mdy rounding maybe become visible when viewed up close.

The number of bits used for the fractional part of each pixel is stored in a special register that can be adjusted by calling TLINE once with a single argument:

TLINE(16) -- MX,MY,MDX,MDY expressed in pixels
6.7 Memory

PICO-8 has 3 types of memory:

1. Base RAM (64k): see layout below. Access with PEEK() POKE() MEMCPY() MEMSET()
2. Cart ROM (32k): same layout as base ram until 0x4300
3. Lua RAM (2MB): compiled program + variables
ⓘ


Technical note: While using the editor, the data being modified is in cart rom, but api functions such as SPR() and SFX() only operate on base ram. PICO-8 automatically copies cart rom to base ram (i.e. calls RELOAD()) in 3 cases:
1. When a cartridge is loaded
2. When a cartridge is run
3. When exiting any of the editor modes // can turn off with: poke(0x5f37,1)
■ Base RAM Memory Layout

0X0    GFX
0X1000 GFX2/MAP2 (SHARED)
0X2000 MAP
0X3000 GFX FLAGS
0X3100 SONG
0X3200 SFX
0X4300 USER DATA
0X5600 CUSTOM FONT (IF ONE IS DEFINED)
0X5E00 PERSISTENT CART DATA (256 BYTES)
0X5F00 DRAW STATE
0X5F40 HARDWARE STATE
0X5F80 GPIO PINS (128 BYTES)
0X6000 SCREEN (8K)
0x8000 USER DATA

User data has no particular meaning and can be used for anything via MEMCPY(), PEEK() & POKE(). Persistent cart data is mapped to 0x5e00..0x5eff but only stored if CARTDATA() has been called. Colour format (gfx/screen) is 2 pixels per byte: low bits encode the left pixel of each pair. Map format is one byte per tile, where each byte normally encodes a sprite index.
■ Remapping Graphics and Map Data

The GFX, MAP and SCREEN memory areas can be reassigned by setting values at the following addresses:

0X5F54 GFX:    can be 0x00 (default) or 0x60 (use the screen memory as the spritesheet)
0X5F55 SCREEN: can be 0x60 (default) or 0x00 (use the spritesheet as screen memory)
0X5F56 MAP:    can be 0x20 (default) or 0x10..0x2f, or 0x80 and above.
0X5F57 MAP SIZE: map width. 0 means 256. Defaults to 128.

Addresses can be expressed in 256 byte increments. So 0x20 means 0x2000, 0x21 means 0x2100 etc. Map addresses 0x30..0x3f are taken to mean 0x10..0x1f (shared memory area). Map data can only be contained inside the memory regions 0x1000..0x2fff, 0x8000..0xffff, and the map height is determined to be the largest possible size that fits in the given region.

GFX and SCREEN addresses can additionally be mapped to upper memory locations 0x80, 0xA0, 0xC0, 0xE0, with the constraint that MAP can not overlap with that address (in this case, the conflicting GFX and/or SCREEN mappings are kicked back to their default mapping).
ⓘ

GFX and SCREEN memory mapping happens at a low level which also affects memory access functions (peek, poke, memcpy). The 8k memory blocks starting at 0x0 and 0x6000 can be thought of as pointers to a separate video ram, and setting the values at 0X5F54 and 0X5F56 alters those pointers.

PEEK(ADDR, [N])

Read a byte from an address in base ram. If N is specified, PEEK() returns that number of results (max: 8192). For example, to read the first 2 bytes of video memory:

A, B = PEEK(0x6000, 2)
*/
int lua_p8_poke(lua_State* L) {
    auto addr = luaL_checkinteger(L, 1);
    //POKE(ADDR, VAL1, VAL2, ...)
    return 0;
}
/*
Write one or more bytes to an address in base ram. If more than one parameter is provided, they are written sequentially (max: 8192).

PEEK2(ADDR)
POKE2(ADDR, VAL)
PEEK4(ADDR)
POKE4(ADDR, VAL)

16-bit and 32-bit versions of PEEK and POKE. Read and write one number (VAL) in little-endian format:

  16 bit: 0xffff.0000
  32 bit: 0xffff.ffff

ADDR does not need to be aligned to 2 or 4-byte boundaries.

Alternatively, the following operators can be used to peek (but not poke), and are slightly faster:

@ADDR  -- PEEK(ADDR)
%ADDR  -- PEEK2(ADDR)
$ADDR  -- PEEK4(ADDR)
*/
void lua_p8_memcpy(int dst_addr, int src_addr, int len) {
    //TODO
}
/*
Copy LEN bytes of base ram from source to dest. Sections can be overlapping

RELOAD(DEST_ADDR, SOURCE_ADDR LEN, [FILENAME])

Same as MEMCPY, but copies from cart rom.

The code section ( >= 0x4300) is protected and can not be read.

If filename specified, load data from a separate cartridge. In this case, the cartridge must be local (BBS carts can not be read in this way).

CSTORE(DEST_ADDR, SOURCE_ADDR, LEN, [FILENAME])

Same as memcpy, but copies from base ram to cart rom.

CSTORE() is equivalent to CSTORE(0, 0, 0x4300)

The code section ( >= 0x4300) is protected and can not be written to.

If FILENAME is specified, the data is written directly to that cartridge on disk. Up to 64 cartridges can be written in one session. See Cartridge Data for more information.

MEMSET(DEST_ADDR, VAL, LEN)

Write the 8-bit value VAL into memory starting at DEST_ADDR, for LEN bytes.

For example, to fill half of video memory with 0xC8:

> MEMSET(0x6000, 0xC8, 0x1000)
6.8 Math
*/
lua_Number lua_p8_max(lua_Number x, lua_Number y) { return std::max(x, y); }
lua_Number lua_p8_min(lua_Number x, lua_Number y) { return std::min(x, y); }
lua_Number lua_p8_mid(lua_Number x, lua_Number y, lua_Number z) { return x > y ? y > z ? y : std::min(x, z) : x > z ? x : std::min(y, z); }
/*
Returns the maximum, minimum, or middle value of parameters

> ?MID(7,5,10) -- 7
*/
lua_Number lua_p8_flr(lua_Number f) { return std::floor(f); }
/*
> ?FLR ( 4.1) -->  4
> ?FLR (-2.3) --> -3
*/
lua_Number lua_p8_ceil(lua_Number f) { return std::ceil(f); }
/*
Returns the closest integer that is equal to or below x

> ?CEIL( 4.1) -->  5
> ?CEIL(-2.3) --> -2
*/
lua_Number lua_p8_cos(lua_Number f) { return std::cosf(f * (M_PI * 2.0f)); }
lua_Number lua_p8_sin(lua_Number f) { return -std::sinf(f * (M_PI * 2.0f)); }
/*
Returns the cosine or sine of x, where 1.0 means a full turn. For example, to animate a dial that turns once every second:

FUNCTION _DRAW()
  CLS()
  CIRC(64, 64, 20, 7)
  X = 64 + COS(T()) * 20
  Y = 64 + SIN(T()) * 20
  LINE(64, 64, X, Y)
END

PICO-8's SIN() returns an inverted result to suit screenspace (where Y means "DOWN", as opposed to mathematical diagrams where Y typically means "UP").

> SIN(0.25) -- RETURNS -1

To get conventional radian-based trig functions without the y inversion, paste the following snippet near the start of your program:

P8COS = COS FUNCTION COS(ANGLE) RETURN P8COS(ANGLE/(3.1415*2)) END
P8SIN = SIN FUNCTION SIN(ANGLE) RETURN -P8SIN(ANGLE/(3.1415*2)) END

ATAN2(DX, DY)

Converts DX, DY into an angle from 0..1

As with cos/sin, angle is taken to run anticlockwise in screenspace. For example:

> ?ATAN(0, -1) -- RETURNS 0.25

ATAN2 can be used to find the direction between two points:

X=20 Y=30
FUNCTION _UPDATE()
  IF (BTN(0)) X-=2
  IF (BTN(1)) X+=2
  IF (BTN(2)) Y-=2
  IF (BTN(3)) Y+=2
END
 
FUNCTION _DRAW()
  CLS()
  CIRCFILL(X,Y,2,14)
  CIRCFILL(64,64,2,7)
   
  A=ATAN2(X-64, Y-64)
  PRINT("ANGLE: "..A)
  LINE(64,64,
    64+COS(A)*10,
    64+SIN(A)*10,7)
END

SQRT(X)

Return the square root of x

ABS(X)
*/
lua_Number lua_p8_abs(lua_Number n) { 
    return std::abs(n);
}
/*
Returns the absolute (positive) value of x
*/

static int32_t p8_rng_state = 1;

lua_Number lua_p8_rnd(lua_Number n) {
    /* Returns a random number n, where 0 <= n < x

    If you want an integer, use flr(rnd(x)). If x is an array-style table, return a random element between table[1] and table[#table].
    */
    p8_rng_state = ((p8_rng_state * 1103515245) + 12345) & 0x7fffffff;
    float res;
    *(uint32_t*)&res = 0x3f800000 | (p8_rng_state & 0x007FFFFF);
    return (res - 1.0f) * n;
}

void lua_p8_srand(lua_Number x) {
    p8_rng_state = x;
}
/*
Sets the random number seed. The seed is automatically randomized on cart startup.

FUNCTION _DRAW()
  CLS()
  SRAND(33)
  FOR I=1,100 DO
    PSET(RND(128),RND(128),7)
  END
END
■ Bitwise Operations

Bitwise operations are similar to logical expressions, except that they work at the bit level.

Say you have two numbers (written here in binary using the "0b" prefix):

X = 0b1010
Y = 0b0110

A bitwise AND will give you bits set when the corresponding bits in X /and/ Y are both set

> PRINT(BAND(X,Y)) -- RESULT:0B0010 (2 IN DECIMAL)

There are 9 bitwise functions available in PICO-8:

BAND(X, Y) -- BOTH BITS ARE SET
BOR(X, Y)  -- EITHER BIT IS SET
BXOR(X, Y) -- EITHER BIT IS SET, BUT NOT BOTH OF THEM
BNOT(X)    -- EACH BIT IS NOT SET
SHL(X, N)  -- SHIFT LEFT N BITS (ZEROS COME IN FROM THE RIGHT)
SHR(X, N)  -- ARITHMETIC RIGHT SHIFT (THE LEFT-MOST BIT STATE IS DUPLICATED)
LSHR(X, N) -- LOGICAL RIGHT SHIFT (ZEROS COMES IN FROM THE LEFT)
ROTL(X, N) -- ROTATE ALL BITS IN X LEFT BY N PLACES
ROTR(X, N) -- ROTATE ALL BITS IN X RIGHT BY N PLACES

Operator versions are also available: & | ^^ ~ << >> >>> <<> >><

For example: PRINT(67 & 63) -- result:3 equivalent to BAND(67,63)

Operators are slightly faster than their corresponding functions. They behave exactly the same, except that if any operands are not numbers the result is a runtime error (the function versions instead default to a value of 0).
■ Integer Division

Integer division can be performed with a \

> PRINT(9\2) -- RESULT:4  EQUIVALENT TO FLR(9/2)
6.9 Custom Menu Items
*/
int lua_p8_menuitem(lua_State* L) {
    auto index = luaL_checkinteger(L, 1);
    auto label = luaL_optstring(L, 2, nullptr);
    //3 == function
    return 0;
}
/*
Add or update an item to the pause menu.

INDEX should be 1..5 and determines the order each menu item is displayed.

LABEL should be a string up to 16 characters long

CALLBACK is a function called when the item is selected by the user. If the callback returns true, the pause menu remains open.

When no label or function is supplied, the menu item is removed.

MENUITEM(1, "RESTART PUZZLE",
  FUNCTION() RESET_PUZZLE() SFX(10) END
)

The callback takes a single argument that is a bitfield of L,R,X button presses.

MENUITEM(1, "FOO",
  FUNCTION(B) IF (B&1 > 0) THEN PRINTH("LEFT WAS PRESSED") END END
)

To filter button presses that are able to trigger the callback, a mask can be supplied in bits 0xff00 of INDEX. For example, to disable L, R for a particular menu item, set bits 0x300 in the index:

MENUITEM(2 | 0x300, "RESET PROGRESS",
  FUNCTION() DSET(0,0) END
)

Menu items can be updated, added or removed from within callbacks:

MENUITEM(3, "SCREENSHAKE: OFF",
  FUNCTION()
    SCREENSHAKE = NOT SCREENSHAKE
    MENUITEM(NIL, "SCREENSHAKE: "..(SCREENSHAKE AND "ON" OR "OFF"))
    RETURN TRUE -- DON'T CLOSE
  END
)
6.10 Strings and Type Conversion

Strings in Lua are written either in single or double quotes or with matching [[ ]] brackets:

S = "THE QUICK"
S = 'BROWN FOX';
S = [[
  JUMPS OVER
  MULTIPLE LINES
]]

The length of a string (number of characters) can be retrieved using the # operator:

>PRINT(#S)

Strings can be joined using the .. operator. Joining numbers converts them to strings.

>PRINT("THREE "..4) --> "THREE 4"

When used as part of an arithmetic expression, string values are converted to numbers:

>PRINT(2+"3")   --> 5
*/
int lua_p8_tostr(lua_State* L) {
    if (lua_gettop(L) < 1) return 0;
    lua_pushstring(L, luaL_tolstring(L, 1, nullptr));
    //TOSTR(VAL, [FORMAT_FLAGS])
    return 1;
}
/*
Convert VAL to a string.

FORMAT_FLAGS is a bitfield:

  0x1: Write the raw hexadecimal value of numbers, functions or tables.
  0x2: Write VAL as a signed 32-bit integer by shifting it left by 16 bits.

TOSTR(NIL) returns "[nil]"

TOSTR() returns ""

TOSTR(17)       -- "17"
TOSTR(17,0x1)   -- "0x0011.0000"
TOSTR(17,0x3)   -- "0x00110000"
TOSTR(17,0x2)   -- "1114112"

TONUM(VAL, [FORMAT_FLAGS])

Converts VAL to a number.

TONUM("17.5")  -- 17.5
TONUM(17.5)    -- 17.5
TONUM("HOGE")  -- NO RETURN VALUE

FORMAT_FLAGS is a bitfield:

  0x1: Read the string as written in (unsigned, integer) hexadecimal without the "0x" prefix
       Non-hexadecimal characters are taken to be '0'.
  0x2: Read the string as a signed 32-bit integer, and shift right 16 bits.
  0x4: When VAL can not be converted to a number, return 0

TONUM("FF",       0x1)  -- 255
TONUM("1114112",  0x2)  -- 17
TONUM("1234abcd", 0x3)  -- 0x1234.abcd

CHR(VAL0, VAL1, ...)

Convert one or more ordinal character codes to a string.

CHR(64)                    -- "@"
CHR(104,101,108,108,111)   -- "hello"

ORD(STR, [INDEX], [NUM_RESULTS])

Convert one or more characters from string STR to their ordinal (0..255) character codes.

Use the INDEX parameter to specify which character in the string to use. When INDEX is out of range or str is not a string, ORD returns nil.

When NUM_RESULTS is given, ORD returns multiple values starting from INDEX.

ORD("@")         -- 64
ORD("123",2)     -- 50 (THE SECOND CHARACTER: "2")
ORD("123",2,3)   -- 50,51,52

SUB(STR, POS0, [POS1])

Grab a substring from string str, from pos0 up to and including pos1. When POS1 is not specified, the remainder of the string is returned. When POS1 is specified, but not a number, a single character at POS0 is returned.

S = "THE QUICK BROWN FOX"
PRINT(SUB(S,5,9))    --> "QUICK"
PRINT(SUB(S,5))      --> "QUICK BROWN FOX"
PRINT(SUB(S,5,TRUE)) --> "Q"
*/
int lua_p8_split(lua_State* L) {
    //SPLIT(STR, [SEPARATOR], [CONVERT_NUMBERS])
    auto str = luaL_checkstring(L, 1);
    auto sep = luaL_optstring(L, 2, ",");
    lua_newtable(L);
    int idx = 1;
    auto start = str;
    auto convert_num = true;
    auto sep_len = strlen(sep);
    auto add_entry = [&](const char* start, size_t len) {
        if (convert_num) {
            char* end;
            auto f = strtof(start, &end);
            if (end == start + len) {
                lua_pushnumber(L, f);
                lua_seti(L, -2, idx);
                return;
            }
        }
        lua_pushlstring(L, start, len);
        lua_seti(L, -2, idx);
    };
    while(auto end = strstr(start, sep)) {
        add_entry(start, end - start);
        idx += 1;
        start = end + sep_len;
    }
    add_entry(start, strlen(start));
    return 1;
}
/*
Split a string into a table of elements delimited by the given separator (defaults to ","). When separator is a number n, the string is split into n-character groups. When convert_numbers is true, numerical tokens are stored as numbers (defaults to true). Empty elements are stored as empty strings.

SPLIT("1,2,3")               -- {1,2,3}
SPLIT("ONE:TWO:3",":",FALSE) -- {"ONE","TWO","3"}
SPLIT("1,,2,")               -- {1,"",2,""}

TYPE(VAL)

Returns the type of val as a string.

> PRINT(TYPE(3))
NUMBER
> PRINT(TYPE("3"))
STRING
6.11 Cartridge Data

Using CARTDATA(), DSET(), AND DGET(), 64 numbers (256 bytes) of persistent data can be stored on the user's PICO-8 that persists after the cart is unloaded or PICO-8 is shutdown. This can be used as a lightweight way to store things like high scores or to save player progress. It can also be used to share data across cartridges / cartridge versions.

If more than 256 bytes is needed, it is also possible to write directly to the cartridge using CSTORE(). The disadvantage is that the data is tied to that particular version of the cartridge. e.g. if a game is updated, players will lose their savegames. Also, some space in the data sections of the cartridge need to be left available to use as storage.

Another alternative is to write directly to a second cartridge by specifying a fourth parameter to CSTORE(). This requires a cart swap (which in reality only means the user needs to watch a spinny cart animation for 1 second).

CSTORE(0,0,0X2000, "SPRITE SHEET.P8")
-- LATER, RESTORE THE SAVED DATA:
RELOAD(0,0,0X2000, "SPRITE SHEET.P8")

CARTDATA(ID)

Opens a permanent data storage slot indexed by ID that can be used to store and retrieve up to 256 bytes (64 numbers) worth of data using DSET() and DGET().

CARTDATA("ZEP_DARK_FOREST")
DSET(0, SCORE)

ID is a string up to 64 characters long, and should be unusual enough that other cartridges do not accidentally use the same id. Legal characters are a..z, 0..9 and underscore (_)

Returns true if data was loaded, otherwise false.

CARTDATA can be called once per cartridge execution, and so only a single data slot can be used.

Once a cartdata ID has been set, the area of memory 0X5E00..0X5EFF is mapped to permanent storage, and can either be accessed directly or via DGET()/@DSET().

There is no need to flush written data -- it is automatically saved to permanent storage even if modified by directly POKE()'ing 0X5E00..0X5EFF.

DGET(INDEX)

Get the number stored at INDEX (0..63)

Use this only after you have called CARTDATA()

DSET(INDEX, VALUE)

Set the number stored at index (0..63)

Use this only after you have called CARTDATA()
6.12 GPIO

GPIO stands for "General Purpose Input Output", and allows machines to communicate with each other. PICO-8 maps bytes in the range 0x5f80..0x5fff to gpio pins that can be

POKE()ed (to output a value -- e.g. to make an LED light up) or @PEEK()ed (e.g. to read

the state of a switch).

GPIO means different things for different host platforms:

CHIP:         0x5f80..0x5f87 mapped to xio-p0..xio-p7
Pocket CHIP:  0x5f82..0x5f87 mapped to GPIO1..GPIO6
  // xio-p0 & p1 are exposed inside the prototyping area inside the case.
Raspberry Pi: 0x5f80..0x5f9f mapped to wiringPi pins 0..31
  // see http://wiringpi.com/pins/ for mappings on different models.
  // also: watch out for BCM vs. WiringPi GPIO indexing!

CHIP and Raspberry Pi values are all digital: 0 (LOW) and 255 (HIGH)

A program to blink any LEDs attached on and off:

T = 0
FUNCTION _DRAW()
 CLS(5)
 FOR I=0,7 DO
  VAL = 0
  IF (T % 2 < 1) VAL = 255
  POKE(0X5F80 + I, VAL)
  CIRCFILL(20+I*12,64,4,VAL/11)
 END
 T += 0.1
END
■ Serial

For more precise timing, the SERIAL() command can be used. GPIO writes are buffered and dispatched at the end of each frame, allowing clock cycling at higher and/or more regular speeds than is possible by manually bit-banging using POKE() calls.

SERIAL(CHANNEL, ADDRESS, LENGTH)

CHANNEL:
  0x000..0x0fe    corresponds to gpio pin numbers; send 0x00 for LOW or 0xFF for HIGH
  0x0ff           delay; length is taken to mean "duration" in microseconds (excl. overhead)
  0x400..0x401    ws281x LED string (experimental)

ADDRESS: The PICO-8 memory location to read from / write to.

LENGTH: Number of bytes to send. 1/8ths are allowed to send partial bit strings.

For example, to send a byte one bit at a time to a typical APA102 LED string:

VAL = 42          -- VALUE TO SEND
DAT = 16 CLK = 15 -- DATA AND CLOCK PINS DEPEND ON DEVICE
POKE(0X4300,0)    -- DATA TO SEND (SINGLE BYTES: 0 OR 0XFF)
POKE(0X4301,0XFF)
FOR B=0,7 DO
  -- SEND THE BIT (HIGH FIRST)
  SERIAL(DAT, BAND(VAL, SHL(1,7-B))>0 AND 0X4301 OR 0X4300, 1)
  -- CYCLE THE CLOCK
  SERIAL(CLK, 0X4301)
  SERIAL(0XFF, 5) -- DELAY 5
  SERIAL(CLK, 0X4300)
  SERIAL(0XFF, 5) -- DELAY 5
END

Additional channels are available for bytestreams to and from the host operating system. These are intended to be most useful for UNIX-like environments while developing toolchains, and are not available while running a BBS or exported cart [1]. Maximum transfer rate in all cases is 64k/sec (blocks cpu).

0x800  dropped file   //  stat(120) returns TRUE when data is available
0x802  dropped image  //  stat(121) returns TRUE when data is available
0x804  stdin
0x805  stdout
0x806  file specified with: pico8 -i filename
0x807  file specified with: pico8 -o filename

Image files dropped into PICO-8 show up on channel 0x802 as a bytestream with a special format: The first 4 bytes are the image's width and height (2 bytes each little-endian, like PEEK2), followed by the image in reading order, one byte per pixel, colour-fitted to the display palette at the time the file was dropped.

[1] Channels 0x800 and 0x802 are available from exported binaries, but with a maximum file size of 256k, or 128x128 for images.
■ HTML

Cartridges exported as HTML / .js use a global array of integers (pico8_gpio) to represent gpio pins. The shell HTML should define the array:

var pico8_gpio = Array(128);
6.13 Mouse and Keyboard Input

// EXPERIMENTAL -- but mostly working on all platforms

Mouse and keyboard input can be achieved by enabling devkit input mode:

POKE(0x5F2D, flags) -- where flags are:

0x1 Enable
0x2 Mouse buttons trigger btn(4)..btn(6)
0x4 Pointer lock (use stat 38..39 to read movements)

Note that not every PICO-8 will have a keyboard or mouse attached to it, so when posting carts to the Lexaloffle BBS, it is encouraged to make keyboard and/or mouse control optional and off by default, if possible. When devkit input mode is enabled, a message is displayed to BBS users warning them that the program may be expecting input beyond the standard 6-button controllers.

The state of the mouse and keyboard can be found in stat(x):

STAT(30) -- (Boolean) True when a keypress is available
STAT(31) -- (String) character returned by keyboard
STAT(32) -- Mouse X
STAT(33) -- Mouse Y
STAT(34) -- Mouse buttons (bitfield)
STAT(36) -- Mouse wheel event
STAT(38) -- Relative x movement (in host desktop pixels) -- requires flag 0x4
STAT(39) -- Relative y movement (in host desktop pixels) -- requires flag 0x4
6.14 Additional Lua Features

PICO-8 also exposes 2 features of Lua for advanced users: Metatables and Coroutines.

For more information, please refer to the Lua 5.2 manual.
▨ Metatables

Metatables can be used to define the behaviour of objects under particular operations. For example, to use tables to represent 2D vectors that can be added together, the '+' operator is redefined by defining an "__add" function for the metatable:

VEC2D={
 __ADD=FUNCTION(A,B)
  RETURN {X=(A.X+B.X), Y=(A.Y+B.Y)}
 END
}
 
V1={X=2,Y=9} SETMETATABLE(V1, VEC2D)
V2={X=1,Y=5} SETMETATABLE(V2, VEC2D)
V3 = V1+V2
PRINT(V3.X..","..V3.Y) -- 3,14

SETMETATABLE(TBL, M)

Set table TBL metatable to M

GETMETATABLE(TBL)

return the current metatable for table t, or nil if none is set

RAWSET(TBL, KEY, VALUE)
RAWGET(TBL, KEY)
RAWEQUAL(TBL1,TBL2
RAWLEN(TBL)

Raw access to the table, as if no metamethods were defined.
▨ Function Arguments

The list of function arguments can be specifed with ...

FUNCTION PREPRINT(PRE, S, ...)
  LOCAL S2 = PRE..TOSTR(S)
  PRINT(S2, ...) -- PASS THE REMAINING ARGUMENTS ON TO PRINT()
END

To accept a variable number of arguments, use them to define a table and/or use Lua's select() function. select(index, ...) returns all of the arguments after index.

FUNCTION FOO(...)
  LOCAL ARGS={...} -- BECOMES A TABLE OF ARGUMENTS
  FOREACH(ARGS, PRINT)
  ?SELECT("#",...)    -- ALTERNATIVE WAY TO COUNT THE NUMBER OF ARGUMENTS
  FOO2(SELECT(3,...)) -- PASS ARGUMENTS FROM 3 ONWARDS TO FOO2()
END
▨ Coroutines

Coroutines offer a way to run different parts of a program in a somewhat concurrent way, similar to threads. A function can be called as a coroutine, suspended with

YIELD() any number of times, and then resumed again at the same points.

FUNCTION HEY()
  PRINT("DOING SOMETHING")
  YIELD()
  PRINT("DOING THE NEXT THING")
  YIELD()
  PRINT("FINISHED")
END
 
C = COCREATE(HEY)
FOR I=1,3 DO CORESUME(C) END

COCREATE(F)

Create a coroutine for function f.

CORESUME(C, [P0, P1 ..])

Run or continue the coroutine c. Parameters p0, p1.. are passed to the coroutine's function.

Returns true if the coroutine completes without any errors Returns false, error_message if there is an error.

** Runtime errors that occur inside coroutines do not cause the program to stop running. It is a good idea to wrap CORESUME() inside an ASSERT(). If the assert fails, it will print the error message generated by coresume.

ASSERT(CORESUME(C))

COSTATUS(C)

Return the status of coroutine C as a string:
  "running"
  "suspended"
  "dead"

YIELD

Suspend execution and return to the caller.
*/

static int lua_p8_pack (lua_State *L) {
  int i;
  int n = lua_gettop(L);  /* number of elements to pack */
  lua_createtable(L, n, 1);  /* create result table */
  lua_insert(L, 1);  /* put it at index 1 */
  for (i = n; i >= 1; i--)  /* assign elements */
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");  /* t.n = number of elements */
  return 1;  /* return table */
}


static int lua_p8_unpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = luaL_opt(L, luaL_checkinteger, 3, luaL_len(L, 1));
  if (i > e) return 0;  /* empty range */
  n = l_castS2U(e) - l_castS2U(i);  /* number of elements minus 1 */
  if (l_unlikely(n >= (unsigned int)INT_MAX  ||
                 !lua_checkstack(L, (int)(++n))))
    return luaL_error(L, "too many results to unpack");
  for (; i < e; i++) {  /* push arg[i..e - 1] (to avoid overflows) */
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);  /* push last element */
  return (int)n;
}


#define STR_(s) #s
#define STR(s) STR_(s)
#define P8_BIND(f) LUA_BIND(L, lua_p8_ ## f, STR(f))
void setupP8LuaEnv(lua_State* L)
{
    P8_BIND(save);
    P8_BIND(load);
    P8_BIND(ls);
    P8_BIND(run);
    P8_BIND(stop);
    P8_BIND(assert);

    P8_BIND(print);
    P8_BIND(time);
    P8_BIND(t);
    P8_BIND(clip);
    P8_BIND(pset);
    P8_BIND(fget);
    P8_BIND(cls);
    P8_BIND(camera);
    P8_BIND(circ);
    P8_BIND(circfill);
    P8_BIND(rect);
    P8_BIND(rectfill);
    P8_BIND(rrect);
    P8_BIND(rrectfill);
    P8_BIND(pal);
    P8_BIND(spr);

    P8_BIND(add);
    P8_BIND(del);
    P8_BIND(deli);
    P8_BIND(count);
    P8_BIND(all);
    P8_BIND(foreach);
    P8_BIND(pairs);
    P8_BIND(btn);
    P8_BIND(btnp);

    P8_BIND(sfx);
    P8_BIND(music);
    P8_BIND(mget);
    P8_BIND(mset);
    P8_BIND(map);

    P8_BIND(poke);
    P8_BIND(memcpy);

    P8_BIND(min);
    P8_BIND(max);
    P8_BIND(mid);
    P8_BIND(flr);
    P8_BIND(ceil);
    P8_BIND(cos);
    P8_BIND(sin);
    P8_BIND(abs);
    P8_BIND(rnd);
    P8_BIND(srand);

    P8_BIND(menuitem);
    P8_BIND(tostr);
    P8_BIND(split);

    P8_BIND(pack);
    P8_BIND(unpack);
}