// GameBoard version 1.1
#ifndef __GAME_BOARD_H__
#define __GAME_BOARD_H__

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

class Tile;
enum Color : unsigned char;

/*****************************************************************************/
/*****************************************************************************/

class GameBoard {
public:

  GameBoard(int rowCount = 10, int colCount = 10);
  ~GameBoard();

  int rowCount() const { return _rowCount; };
  int colCount() const { return _colCount; };

  bool displayCoords() const { return _displayCoords; }
  void setDisplayCoords(bool displayCoords);

  void setHighlightedCoords();
  void setHighlightedCoords(int row, int col);
  void setHighlightedCoordsColor(Color color);

  bool vt100Mode() const { return _vt100Mode; }
  void setVT100Mode(bool vt100Mode);

  bool displayEmptyTileDots() const { return _displayEmptyTileDots; }
  void setDisplayEmptyTileDots(bool displayEmptyTileDots);

  void updateConsole() const;
  void redrawConsole() const;

  std::string message(int messageLineNumber = 0) const;
  void setMessage(std::string newMessage = "", int messageLineNumber = 0);

  void setLogLineCount(int count);

  Tile tileAt(int row, int col) const;
  void setTileAt(int row, int col, Tile tile);
  void setTileAt(int row, int col, char glyph, Color color);

  void clearAllTiles();
  void clearTileAt(int row, int col);
  void clearLog();

  char glyphAt(int row, int col) const;
  void setGlyphAt(int row, int col, char glyph);

  // Commands are generally just the character pressed, e.g. 'a', ' ', 'x'.
  // This enum provides constants representing special keys, e.g. the arrow keys - listed below.
  enum CommandKey : char;

  // nextCommandKey returns the key pressed by the user. It can be used sychronously or asynchronously.
  // A timeout of zero mean nextCommand won't return until the user presses a key.
  // A non-zero timeout is how long, in tenths of a second, to wait for a key press
  // until giving up and returning noKey.
  char nextCommandKey(unsigned timeout = 0);

  static void printCommandKey(char cmd);

  // Nethack mode interprets the nethack movement keys as arrow keys.
  // h = left, j = down, k = up, l = right (a la the vi editor)
  // y = up-left, u = up-right, b = down-left, n = down-right
  bool nethackKeyMode() const { return _nethackKeyMode; }
  void setNethackKeyMode(bool nethackKeyMode) { _nethackKeyMode = nethackKeyMode; }

  // WASD mode interprets the w, a, s, d keys as arrow keys.
  // w = up, a = left, s = down, r = right
  bool wasdKeyMode() const { return _wasdKeyMode; }
  void setWASDKeyMode(bool wasdKeyMode) { _wasdKeyMode = wasdKeyMode; }

  template <typename T>
  GameBoard& operator<<(T const& value) {
    _stringStream << value;
    handleInsertion();
    return *this;
  }

  // Special case to handle endl - which is a function.
  GameBoard& operator<<(std::ostream& (*func)(std::ostream&));

private:
  bool _vt100Mode = true;
  bool _wasdKeyMode = false;
  bool _displayCoords = true;
  bool _nethackKeyMode = false;
  bool _displayEmptyTileDots = true;
  mutable bool _redrawNeeded = true;
  int _rowCount;
  int _colCount;
  int _highlightedRow;
  int _highlightedCol;
  int _logLineCount = 5;
  mutable int _dirtyHighlightedRow;
  mutable int _dirtyHighlightedCol;
  Color _highlightedCoordsColor;
  std::vector<std::string> _logLines;
  std::vector<std::string> _messageLines = {"", ""};
  std::ostringstream _stringStream;
  Tile *_tiles;

  void clearScreen() const;
  void setDirtyOnAllTiles(bool dirty) const;

  void redraw() const;
  void update() const;

  void drawTop(bool showCoords) const;
  void drawBottom(bool showCoords) const;
  void drawRow(int row, bool showCoords) const;
  void drawLog() const;
  void drawMessage() const;

  // void log(std::string str);
  void log(std::vector<std::string> strings);
  void handleInsertion();

  void updateRowCoords(int row) const;
  void updateColCoords(int row) const;
  void updateHighlightedCoords() const;
  void setHighlightedCoords_(int row, int col); 

  void rangeCheck(int row, int col) const;

  unsigned tileIndex(int row, int col) const;
  Tile displayedTileAt(int row, int col) const;

  void vt100GraphicsEnd() const;
  void vt100GraphicsStart() const;

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

  int firstLogLineVT100Row() const;
  int firstMessageLineVT100Row() const;
};

enum GameBoard::CommandKey : char {
  noKey = 0, // we rely on this being zero, i.e. "false"

  tabKey = 0x09,
  enterKey = 0x0A,
  escapeKey = 0x1B,
  deleteKey = 0x7F,

  // Use high-bit values for special keys.
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

/*****************************************************************************/
/*****************************************************************************/

class Tile {
public:
  Tile(const Tile &tile) = default;

  Tile(char glyph, Color color);
  Tile(char glyph = 0);

  char glyph() const { return _glyph; };
  Color color() const { return _color; };

  bool operator== (const Tile &rhs);
  bool operator!= (const Tile &rhs);

  friend GameBoard;

private:
  char _glyph;
  Color _color;
  bool _dirty;

  Tile(const Tile &tile, bool dirty);
  bool isDirty() const { return _dirty; };

  static void colorEnd(Color color);
  static void colorStart(Color color);

  void draw(bool displayEmptyTiles = true) const;
};

/*****************************************************************************/
/*****************************************************************************/

enum Color : unsigned char {
  defaultColor,
  black,
  red,
  green,
  yellow,
  blue,
  magenta,
  cyan,
  white,
  darkRed,
  darkBlue,
  gray, // dark white
};

#endif
