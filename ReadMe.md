
# Introduction

This file provides the GameBoard and Tile classes. A GameBoard represents a 2D grid of tiles, with 0,0 being the top-left corner. A Tile represents a character and the color in which it will be displayed.

## GameBoard

### GameBoard Grid

GameBoards are 2D grids up to size 100 x 100. The top-left tile is row = 0, col = 0. The bottom-right tile is row = height - 1, col = width - 1.

The constants, `GameBoard::maxWidth` and `GameBoard::maxHeight`, specify the maximum size, currently 50x50.

The simplest usage pattern is to have your main loop draw the board each round:
  2. Use `setTileAt` to place tiles at the desired locations.
  3. Call `draw` to display the board.
  4. Call `nextCommandKey` to determine what to do next.

### Gameboard CommandKeys

`GameBoard` use _CommandKeys_, to represents keys the user presses when interacting with the `GameBoard`. _CommandKeys_ can be either a regular `char` (e.g. 'a' or 'B') or one of the `CommandKey` `enum` constants (e.g. `arrowUpKey` or `deleteKey`).

`Gameboard`'s `nextCommandKey` method returns the key a user presses. The `timeout` parameter determines how long to waits for the keypress. A `timeout` of zero means wait indefinitely - only returning once a key has been pressed. A non-zero `timeout` specifies the maximum time, in tenths of a second, to wait for a keypress. When a key is pressed it immediately returns that key. If after the timeout elapses, no key was pressed, it stops wating and returns `noKey`.


## Tile

The Tile class is use to represent the contents of a position on the GameBoard. It specifies a glyph (char) and its color. `Tile` defines `Color` `enum` constants used to specify the color of a `Tile`.

# Implementation Notes

## Tile

The tile class is basically a wrapper for 4 bytes of data. The low byte is the glyph (char) displayed. A glyph of 0 means the tile is empty. The next 18 bits hold, up to three, Color enum values. Of the remaining 6 bits, the high bit is a private flag to mark tiles as dirty. The rest are currently unused.

The `Color` `enum` constants encode, up to three, VT100 attributes which determine how a character is displayed.

Currently there are constants for the 8 VT100 colors, for the _dark_ colors (dim attribute + color), and for the default (no attributes).


## Information On VT100 Terminal Programming:

Convential printing to `stdout` just prints a sequence of single color characters to the console which scroll towards the bottom.

The ability to draw at an specified location and in a specified color are features of the venerable VT100 terminal which Replit's console emulates. This is accomplished by interpretting special character sequences as terminal commands rather than characters to be printed in the console. Below are links to some articles explaining the details.

  - <https://viewsourcecode.org/snaptoken/kilo/>
  - <https://vt100.net/docs/vt100-ug/chapter3.html>
  - <https://www2.ccs.neu.edu/research/gpc/VonaUtils/vona/terminal/vtansi.htm>
  - <https://en.wikipedia.org/wiki/Box-drawing_character#Unix,_CP/M,_BBS>
