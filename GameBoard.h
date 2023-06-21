#include <limits>
#include <cstdint>
#include <string>

/*
Introduction

This file provides the GameBoard and Tile classes. A GameBoard represents a 2D grid of tiles, with 0,0 being the top-left corner. A Tile represents a character and the color in which it will be displayed.

GameBoard

GameBoards are 2D grids up to size 100 x 100.

The simplest usage pattern is to have your main loop redraw the entire board each round:
  1. Call clearAllTiles to start with an empty board.
  2. Use setTileAt to place tiles at the desired locations.
  3. Call draw to display the board.
  4. Call nextCommandKey to determine what to do next.

If you prefer, instead of using clearAllTiles, you can selectively update the tiles that changed. This might be slightly more efficient, but will probably be more complicated to implement.


CommandKeys

GameBoard define the enum, CommandKey, which represent the keys the user presses to interact with the GameBoard. CommandKeys are either regular characters from the keyboard like 'a' or 'B' or a special named enum constant like arrowUpKey or deleteKey.

Gameboard's nextCommandKey is used to find out which key the user pressed. If nextCommanKey is passed a zero timeout it waits until the user to presses a key before returning. If nextCommanKey is passed a non-zero timeout it stops wating after that many tenths of a second and returns noKey.


Tile

The tile class is basically a wrapper for 4 bytes of data. One byte is the glyph (char) displayed. A glyph of 0 means the tile is empty. The remaining bytes represent up to three VT100 attributes which determine how a character is displayed - e.g. its color.

Tile also provides an enum, Color, which represent the supported groups of VT100 attributes.

Helpful information can be found at these links:
  https://viewsourcecode.org/snaptoken/kilo/
  https://vt100.net/docs/vt100-ug/chapter3.html
  https://www2.ccs.neu.edu/research/gpc/VonaUtils/vona/terminal/vtansi.htm
  https://en.wikipedia.org/wiki/Box-drawing_character#Unix,_CP/M,_BBS
*/

class GameBoard;

class Tile {
public:

  enum class Color : uint32_t {
    vt100Default = 0,

    vt100Black = 0x1E,
    vt100Red = 0x1F,
    vt100Green = 0x20,
    vt100Yellow = 0x21,
    vt100Blue = 0x22,
    vt100Magenta = 0x23,
    vt100Cyan = 0x24,
    vt100White = 0x25,

    // Dark colors are the dim attribute, followed by one of the, above, color attributes.
    vt100DarkRed = 0x021F,
    vt100DarkGreen = 0x0220,
    vt100DarkYellow = 0x0221,
    vt100DarkBlue = 0x0222,
    vt100DarkMagenta = 0x0223,
    vt100DarkCyan = 0x0224,
    vt100Gray = 0x0225,
  };
    
  Tile(char glyph = 0, Color color = Color::vt100Default) {
    _tileBytes = (glyph & 0xFF) | (static_cast<uint32_t>(color) << 8);
  };

  char glyph() const { return _tileBytes & 0xFF; }
  Color color() const { return Color(_tileBytes >> 8); }

  void draw(bool displayEmptyTiles = true) const;

  friend GameBoard;

private:
  uint32_t _tileBytes;

  // Delimit printing to stdout out by these calls to use the specified color.
  static void colorStart(Color color);
  static void colorEnd(Color color);
};


class GameBoard {
public:
  GameBoard(unsigned width = 10, unsigned height = 10);
  ~GameBoard();

  unsigned width() const { return _width; };
  unsigned height() const { return _height; };

  bool displayCoords() const { return _displayCoords; }
  void setDisplayCoords(bool displayCoords) { _displayCoords = displayCoords; }

  void setHighlightedCoords();
  void setHighlightedCoords(unsigned row, unsigned col);
  void setHighlightedCoordsColor(Tile::Color color) { _highlightedCoordsColor = color; };

  bool useVT100Graphics() const { return _useVT100Graphics; }
  void setUseVT100Graphics(bool useVT100Graphics) {
    _useVT100Graphics = useVT100Graphics;
  }

  bool displayEmptyTiles() const { return _displayEmptyTiles; }
  void setDisplayEmptyTiles(bool displayEmptyTiles) {
    _displayEmptyTiles = displayEmptyTiles;
  }

  void draw() const;

  std::string message() const {return _message; }
  void setMessage(std::string newMessage) { _message = newMessage; }

  Tile tileAt(unsigned row, unsigned col) const;
  void setTileAt(unsigned row, unsigned col, char glyph, Tile::Color color = Tile::Color::vt100Default);

  void clearAllTiles();
  void clearTileAt(unsigned row, unsigned col);

  char glyphAt(unsigned row, unsigned col) const;
  void setGlyphAt(unsigned row, unsigned col, char glyph);

  // Commands are generally just the character pressed, e.g. 'a', ' ', 'x'.
  // There are constants for some of the special keys, e.g. the arrow keys. See below.
  enum CommandKey : char;

  // nextCommandKey returns the key pressed by the user. It can be used sychronously or asynchronously.
  // A timeout of zero mean nextCommand won't return until the user presses a key.
  // A non-zero timeout is how long, in tenths of seconds, to wait for a key press
  // until giving up and returning noKey.
  char nextCommandKey(unsigned timeout = 0);

  // Prints a command to stdout to help with debugging.
  static void printCommandKey(char cmd);

  // In Nethack key mode, the nethack movement keys are interpreted as arrow keys.
  // h = left, j = down, k = up, l = right (a la the vi editor)
  // y = up-left, u = up-right, b = down-left, n = down-right
  bool nethackKeyMode() const { return _nethackKeyMode; }
  void setNethackKeyMode(bool nethackKeyMode) { _nethackKeyMode = nethackKeyMode; }

  // In WASD key mode, the w, a, s, d keys are interpreted as arrow keys.
  // w = up, a = left, s = down, r = right
  bool wasdKeyMode() const { return _wasdKeyMode; }
  void setWASDKeyMode(bool wasdKeyMode) { _wasdKeyMode = wasdKeyMode; }

private:
  bool _displayCoords = true;
  bool _useVT100Graphics = true;
  bool _displayEmptyTiles = true;
  bool _nethackKeyMode = false;
  bool _wasdKeyMode = false;
  unsigned _defaultTileGlyph = ' ';
  unsigned _defaultTileColor = 0;
  unsigned _width;
  unsigned _height;
  unsigned _highlightedRow = std::numeric_limits<unsigned>::max();
  unsigned _highlightedCol = std::numeric_limits<unsigned>::max();
  Tile::Color _highlightedCoordsColor = Tile::Color::vt100Blue;
  std::string _message;
  Tile *_tiles;

  void clearScreen() const;
  void drawTop(bool showCoords) const;
  void drawBottom(bool showCoords) const;
  void drawRow(unsigned row, bool showCoords) const;

  unsigned tileIndex(unsigned row, unsigned col) const;
  Tile displayedTileAt(unsigned row, unsigned col) const;

  void vt100EscStart() const;
  void vt100EscEnd() const;

  char topLeftCornerGlyph() const;
  char topRightCornerGlyph() const;
  char bottomLeftCornerGlyph() const;
  char bottomRightCornerGlyph() const;
  char horizontalLineGlyph() const;
  char verticalLineGlyph() const;

  char normalCommandKey(char c) const;
  static char nethackCommandKey(char c);
  static char wasdCommandKey(char c);
  static char escapedCommandKey(char c);
};

enum GameBoard::CommandKey : char {
  noKey = 0, // we rely on this being zero, i.e. "false"

  tabKey = 0x09,
  enterKey = 0x0A,
  escapeKey = 0x1B,
  deleteKey = 0x7F,

  // Using high-bit values for special keys.
  unknownKey = char(0x80),
  arrowUpKey,
  arrowDownKey,
  arrowRightKey,
  arrowLeftKey,
  arrowUpLeftKey,
  arrowUpRightKey,
  arrowDownLeftKey,
  arrowDownRightKey,
  pageUpKey,
  pageDownKey,
  deleteForwardKey,
};
