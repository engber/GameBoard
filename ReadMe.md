
# Introduction

GameBoard.h/GameBoard.cpp provides the `GameBoard` and `Tile` classes which visually display a grid of characters in the console. Additionally, `GameBoard` also provides a method to read user keystrokes in the console.

Currently, this is being devloped/tested for the console in [Replit](https://replict.com) but, in principle, it should work other consoles that supports VT100 escape codes.


# GameBoard

GameBoards are 2D grids of tiles. The top-left is row = 0, col = 0. The bottom-right tile is row = height - 1, col = width - 1.

The constants, `GameBoard::maxWidth` and `GameBoard::maxHeight`, specify the maximum size, currently 50x50.

The simplest usage pattern is to have a main loop draw the board each round:
  1. Use `setTileAt` to place tiles at the desired locations.
  2. Call `draw` to display the board.
  3. Call `nextCommandKey` to determine what to do next.

## Tile

The `Tile` class represents the contents of a position on a `GameBoard`. It specifies a glyph (`char`) and its color. `Tile` defines `Color` `enum` constants (e.g. `Tile::Color::vt100Blue`) which are used to specify the color of a `Tile`.

`Tile` defines the `color()` and `glyph()` methods, but there are no corresponding setter methods. Tiles are immutable; instead of modifying a tile, create a new one using a diffrent color or glyph.

- Tiles can be compared using the comparison operators, `==` and `!=`.
- The default constuctor, `Tile()`, returns an empty tile.
- To test if a tile is empty, use `someTile == Tile()`. 

## CommandKeys

`GameBoard` use _CommandKeys_, to represent keys the user presses when interacting with the `GameBoard`. _CommandKeys_ can be either a regular `char` (e.g. 'a' or 'B') or one of the `CommandKey` `enum` constants (e.g. `arrowUpKey` or `deleteKey`).

The `nextCommandKey` method returns the key a user pressed. The `timeout` parameter determines how long to wait for the keypress. A `timeout` of zero means wait indefinitely - only returning once a key has been pressed. A non-zero `timeout` specifies the maximum time, in tenths of a second, to wait for a keypress. When a key is pressed it immediately returns that key. If after the timeout elapses, no key was pressed, it stops wating and returns `noKey`.

## GameBoard Methods

`void draw()`  
The first time the `draw` method is called it clears the console and draws the board.
Subsequent will try to be _smart_, only drawing the parts that need updating, but some changes (e.g. turning on/off displaying coordinates) require a full redrawing, including clearing the console.


`void forceRedraw()`  
The `forceRedraw` method _always_ clears the console and draws the board. This method may be helpful in debugging drawing problems; determining if the _smart_ update logic is the cause.


`Tile tileAt(int row, int col) const;`  
`void setTileAt(int row, int col, Tile tile);`  
`void setTileAt(int row, int col, char glyph, Tile::Color color = Tile::Color::vt100Default);`  
These methods place characters on the board.


`void clearAllTiles();`  
`void clearTileAt(int row, int col);`  
These methods are convenient alternatives to setting empty tiles.


`char glyphAt(int row, int col) const;`  
`void setGlyphAt(int row, int col, char glyph);`  
Glyph accessors provide an alternative to the tile accessors, for when you don't care about color.


`std::string message() const;`  
`void setMessage(std::string newMessage = "");`  
Messages are displayed below the board. Use newlines for multiple lines.


`bool displayCoords() const`  
`void setDisplayCoords(bool displayCoords);`  
Allows turning off/on the display of row,col numbers at the edges of the board.
Defaults to on.


`void setHighlightedCoords();`  
`void setHighlightedCoords(int row, int col);`  
Allows specifiying a row,col to be highlighted in the displayed coordinates.
Defaults to none.


`bool vt100Mode() const`  
`void setVT100Mode(bool vt100Mode);`  
VT100 mode uses color, special symbols, and cursor control to draw the board
  at a fixed location in the console. Defaults to on. For debugging, it may be useful to turn VT100 mode off so that:
- Drawing appends to the console rather than drawing in a fixed location.
- Scrolling back will show previously drawn boards.
- Debug printing messages will be more easily viewable.

`bool displayEmptyTileDots() const`  
`void setDisplayEmptyTileDots(bool displayEmptyTileDotss);`  
Allows specifiying that a dot, instead of nothing, is displayed for empty tiles. Defaults to on.


`char nextCommandKey(unsigned timeout = 0);`  
`nextCommandKey` returns the key a user pressed. The `timeout` parameter determines how long to wait for the keypress.
- A `timeout` of zero means wait indefinitely; only returning once a key has been pressed.
- A non-zero `timeout` specifies the maximum time, in tenths of a second, to wait for a keypress. When a key is pressed it immediately returns that key. If after the timeout elapses, no key was pressed, it stops wating and returns `noKey`.

`static void printCommandKey(char cmd);`  
Prints a command key to stdout to aid in debugging.


`bool nethackKeyMode() const;`  
`setNethackKeyMode(bool nethackKeyMode);`  
Nethack mode interprets the nethack movement keys as arrow keys.
- `h` = left, `j` = down, `k` = up, `l` = right (a la the vi editor)
- `y` = up-left, `u` = up-right, `b` = down-left, `n` = down-right


`bool wasdKeyMode() const;`  
`void setWASDKeyMode(bool wasdKeyMode);`  
WASD mode interprets the w, a, s, d keys as arrow keys.
  - `w` = up, `a` = left, `s` = down, `r` = right


# Implementation Notes

## Tile

The tile class is basically a wrapper for 4 bytes of data. The low byte is the glyph (char) displayed. A glyph of 0 means the tile is empty. The next 18 bits hold, up to three, Color enum values. Of the remaining 6 bits, the high bit is a private flag to mark tiles as dirty. The rest are currently unused.

## Color

The `Color` `enum` constants encode, up to three, VT100 attributes which determine how a character is displayed.

Currently there are constants for the 8 VT100 colors, for the _dark_ colors (dim attribute + color), and for the default (no attributes).

## Information On VT100 Terminal Programming:

Convential printing to `stdout` just prints a sequence of single color characters to the console which scroll towards the bottom.

The ability to draw at an specified location and in a specified color are features of the venerable VT100 terminal which Replit's console emulates. This is accomplished by interpreting special character sequences as terminal commands rather than characters to be printed to the console. Below are links to some articles explaining the details:

  - <https://viewsourcecode.org/snaptoken/kilo/>
  - <https://vt100.net/docs/vt100-ug/chapter3.html>
  - <https://www2.ccs.neu.edu/research/gpc/VonaUtils/vona/terminal/vtansi.htm>
  - <https://en.wikipedia.org/wiki/Box-drawing_character#Unix,_CP/M,_BBS>
