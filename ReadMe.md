
## Introduction

This file provides the GameBoard and Tile classes. A GameBoard represents a 2D grid of tiles, with 0,0 being the top-left corner. A Tile represents a character and the color in which it will be displayed.

### GameBoard

GameBoards are 2D grids up to size 100 x 100.

The simplest usage pattern is to have your main loop redraw the entire board each round:
  1. Call `clearAllTiles` to start with an empty board.
  2. Use `setTileAt` to place tiles at the desired locations.
  3. Call `draw` to display the board.
  4. Call `nextCommandKey` to determine what to do next.

If you prefer, instead of using `clearAllTiles`, you can selectively update only the tiles that changed. This might be slightly more efficient, but may be more complicated to implement.


### CommandKeys

`GameBoard` use _CommandKeys_, to represents keys the user presses when interacting with the `GameBoard`. _CommandKeys_ can be either a regular `char` (e.g. 'a' or 'B') or one of the `CommandKey` `enum` constants (e.g. `arrowUpKey` or `deleteKey`).

`Gameboard`'s `nextCommandKey` method is used to determine which key the user presses. If `nextCommanKey` is passed a zero timeout it waits until the user to presses a key before returning it. When `nextCommanKey` is passed a non-zero timeout, if no key has been pressed it will stop wating, and return `noKey`.


### Tile

The Tile class is use to represent the contents of a position on the GameBoard. It specifies a glyph (char) and its color.

The tile class is basically a wrapper for 4 bytes of data. The low byte is the glyph (char) displayed. A glyph of 0 means the tile is empty. The next 18 bits hold, up to three, Color enum values. Of the remaining 6 bits, the high bit is a private flag to mark tiles as dirty. The rest are currently unused.

The `Color` `enum` constants encode, up to three, VT100 attributes which determine how a character is displayed.

Currently there are constants for the 8 VT100 colors, for the _dark_ colors (dim attribute + color), and for the default (no attributes).


### Information On VT100 Terminal Programming:

  - <https://viewsourcecode.org/snaptoken/kilo/>
  - <https://vt100.net/docs/vt100-ug/chapter3.html>
  - <https://www2.ccs.neu.edu/research/gpc/VonaUtils/vona/terminal/vtansi.htm>
  - <https://en.wikipedia.org/wiki/Box-drawing_character#Unix,_CP/M,_BBS>

