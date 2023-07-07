#include <cstdint>
#include <string>

class GameBoard;

/*****************************************************************************/
/*****************************************************************************/

class Tile {
public:
  enum class Color : uint32_t;

  Tile(const Tile &tile);

  Tile(char glyph, Color color);
  Tile(char glyph = 0);

  char glyph() const;
  Color color() const;

  bool operator== (const Tile &rhs);
  bool operator!= (const Tile &rhs);

  friend GameBoard;

private:
  uint32_t _4bytes;

  Tile(const Tile &tile, bool dirty);
  bool isDirty() const;

  static void colorEnd(Color color);
  static void colorStart(Color color);

  void draw(bool displayEmptyTiles = true) const;
};

/*****************************************************************************/
/*****************************************************************************/

class GameBoard {
public:

enum {
  maxWidth = 50,
  maxHeight = 50,
};

  GameBoard(int width = 10, int height = 10);
  ~GameBoard();

  int width() const { return _width; };
  int height() const { return _height; };

  bool displayCoords() const { return _displayCoords; }
  void setDisplayCoords(bool displayCoords);

  void setHighlightedCoords();
  void setHighlightedCoords(int row, int col);
  void setHighlightedCoordsColor(Tile::Color color);

  bool vt100Mode() const { return _vt100Mode; }
  void setVT100Mode(bool vt100Mode);

  bool displayEmptyTileDots() const { return _displayEmptyTileDots; }
  void setDisplayEmptyTileDots(bool displayEmptyTileDots);

  void draw() const;
  void forceRedraw() const;

  std::string message() const;
  void setMessage(std::string newMessage = "");

  Tile tileAt(int row, int col) const;
  void setTileAt(int row, int col, Tile tile);
  void setTileAt(int row, int col, char glyph, Tile::Color color);

  void clearAllTiles();
  void clearTileAt(int row, int col);

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

private:
  bool _vt100Mode = true;
  bool _wasdKeyMode = false;
  bool _displayCoords = true;
  bool _nethackKeyMode = false;
  bool _displayEmptyTileDots = true;
  mutable bool _redrawNeeded = true;
  mutable unsigned _dirtyMessageLineCount = 0;
  unsigned _defaultTileGlyph = ' ';
  unsigned _defaultTileColor = 0;
  int _width;
  int _height;
  int _highlightedRow;
  int _highlightedCol;
  mutable int _dirtyHighlightedRow;
  mutable int _dirtyHighlightedCol;
  Tile::Color _highlightedCoordsColor;
  std::string _message;
  Tile *_tiles;

  void clearScreen() const;
  void setDirtyOnAllTiles(bool dirty) const;

  void redraw() const;
  void update() const;

  void drawTop(bool showCoords) const;
  void drawBottom(bool showCoords) const;
  void drawRow(int row, bool showCoords) const;

  void updateMessage() const;
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

// Colors encode up to 3, 6-bit attribute values.

enum class Tile::Color : uint32_t {
  vt100Default = 0,

  // Single attribute values - foreground colors
  vt100Black   = 30,
  vt100Red     = 31,
  vt100Green   = 32,
  vt100Yellow  = 33,
  vt100Blue    = 34,
  vt100Magenta = 35,
  vt100Cyan    = 36,
  vt100White   = 37,

  // Two attribute values - dim + foreground color
  vt100DarkRed     = (2 << 6) | 31,
  vt100DarkGreen   = (2 << 6) | 32,
  vt100DarkYellow  = (2 << 6) | 33,
  vt100DarkBlue    = (2 << 6) | 34,
  vt100DarkMagenta = (2 << 6) | 35,
  vt100DarkCyan    = (2 << 6) | 36,
  vt100Gray        = (2 << 6) | 37,
};
