
# Introduction

GameBoard.h/GameBoard.cpp provides the `GameBoard` and `Tile` classes which visually display a grid of characters in the console. Additionally, `GameBoard` also provides a method to read user keystrokes in the console.

Currently, this is being devloped/tested for the console in [Replit](https://replict.com) but, in principle, it should work other consoles that supports VT100 escape codes.

# Basic Usage

```
  // Construct a board with 15 rows & 20 columns.
  GameBoard board(15, 20);

  bool done = false;
  int row = 5, col = 3;
  while (!done) {
    // Sets a char at a position with a color.
    board.setTileAt(row, col, '@', Tile::Color::vt100DarkMagenta);
    // Draws all tiles, or all that changed since the last draw().
    board.updateConsole();

    // Erases the tile at row, col (providing the illusion of motion).
    board.clearTileAt(row, col);
    row = (row + 1) % board.rowCount();
    col = (col + 1) % board.colCount();
    // Wait 0.2s for user input (the param / 10 seconds).
    char cmd = board.nextCommandKey(2);
    done = cmd == 'q' || cmd == 'Q';
  }
```

# GameBoard

GameBoards are 2D grids of tiles. The top-left is row = 0, col = 0. The bottom-right tile is row = rowCount - 1, col = colCount - 1.

The simplest usage pattern is to have a main loop draw the board each round:
  1. Use `setTileAt` to place tiles at the desired locations.
  2. Call `updateConsole` to display the board.
  3. Call `nextCommandKey` to determine what to do next.

## Console Size Considerations

It's important the console be large enough, in terms of rows/columns, to hold the gameboard. If it's too small it won't draw correctly. If you have a small screen and are having trouble resizing the console to be large engouh. In Replit you can try adjusting the console's font size using cmd +/-, on a Mac, or ctrl +/-, on Windows.

Currently, the max size of a `GameBoard` is limited to 50x50 by the private enum constants, `kMaxRowCount` and `kMaxColCount`.

## Tiles & Colors

The `Tile` class represents the contents of a position on a `GameBoard`. It specifies a glyph (`char`) and its color.

Colors are specified using the `Color` `enum` constants (e.g. `Color::blue`).

`Tile` defines the `color()` and `glyph()` methods, but there are no corresponding setter methods. Tiles are immutable; instead of modifying a tile, create a new one using a diffrent color or glyph.

- Tiles can be compared using the comparison operators, `==` and `!=`.
- The default constuctor, `Tile()`, returns an empty tile.
- To test if a tile is empty, use `someTile == Tile()`. 

## CommandKeys

`GameBoard` use _CommandKeys_, to represent keys the user presses when interacting with the `GameBoard`. _CommandKeys_ can be either a regular `char` (e.g. 'a' or 'B') or one of the `CommandKey` `enum` constants (e.g. `arrowUpKey` or `deleteKey`).

The `nextCommandKey` method returns the key a user pressed. The `timeout` parameter determines how long to wait for the keypress. A `timeout` of zero means wait indefinitely - only returning once a key has been pressed. A non-zero `timeout` specifies the maximum time, in tenths of a second, to wait for a keypress. When a key is pressed it immediately returns that key. If after the timeout elapses, no key was pressed, it stops wating and returns `noKey`.

## Message Lines and Logging

`Gameboard` suports messages lines, displayed in a fixed location, underneath the board, which you can use to display information to the player. See `setMessage` below.

In addition, `Gameboard` also supports logging transitory information below the message lines. There are a fixed number of lines of logging. Once that number is exceeded, the oldest line is scrolled out.

To accomplish `Gameboard` overrides the instertion operator, `<<`, to support logging. E.g.
```
  myGameBoard << "this line will be logged" << endl;
```

The number of log lines can be configured using `setLogLineCount`.

## GameBoard Methods

`void updateConsole()`  
The first time the `draw` method is called it clears the console and displays the board.
Subsequent calls will try to be _smart_, only drawing the parts that need updating, but some changes (e.g. turning on/off displaying coordinates) require a full redrawing, including clearing the console.


`void redrawConsole()`  
The `redrawConsole` method _always_ clears the console and draws the board. This method may be helpful in debugging drawing problems; determining if the _smart_ update logic is the cause.


`Tile tileAt(int row, int col) const;`  
`void setTileAt(int row, int col, Tile tile);`  
`void setTileAt(int row, int col, char glyph, Color color);`  
These methods place characters on the board.


`void clearAllTiles();`  
`void clearTileAt(int row, int col);`  
These methods are convenient alternatives to setting empty tiles.


`char glyphAt(int row, int col) const;`  
`void setGlyphAt(int row, int col, char glyph);`  
Glyph accessors provide an alternative to the tile accessors, for when you don't care about color.


`std::string message(int messageLineNumber = 0) const;`  
`void setMessage(std::string newMessage = "", int messageLineNumber = 0);`  
Messages are displayed below the board. Currently, only two message lines are supported, `0` and `1`. 

`void setLogLineCount(int count);`
This method configures how many lines are logged, below the board _and_ below the message lines, before the lines start scrolling. The default line count is 5. 

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


# Information On VT100 Terminal Programming:

Convential printing to `stdout` just prints a sequence of single color characters to the console which scroll towards the bottom.

The ability to draw at an specified location and in a specified color are features of the venerable VT100 terminal which Replit's console emulates. This is accomplished by interpreting special character sequences as terminal commands rather than characters to be printed to the console. Below are links to some articles explaining the details:

  - <https://viewsourcecode.org/snaptoken/kilo/>
  - <https://vt100.net/docs/vt100-ug/chapter3.html>
  - <https://www2.ccs.neu.edu/research/gpc/VonaUtils/vona/terminal/vtansi.htm>
  - <https://en.wikipedia.org/wiki/Box-drawing_character#Unix,_CP/M,_BBS>
