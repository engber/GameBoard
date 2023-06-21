#include <limits>
#include <cstdint>
#include <string>

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

  // Used to bracket printing to stdout to draw in the specified color.
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

  // Highlighted the specified row,col in the displayed coordinates.
  void setHighlightedCoords();
  void setHighlightedCoords(unsigned row, unsigned col);
  void setHighlightedCoordsColor(Tile::Color color) { _highlightedCoordsColor = color; };

  // VT100 mode uses color and special symbols to draw the board.
  bool useVT100Graphics() const { return _useVT100Graphics; }
  void setUseVT100Graphics(bool useVT100Graphics) {
    _useVT100Graphics = useVT100Graphics;
  }

  bool displayEmptyTiles() const { return _displayEmptyTiles; }
  void setDisplayEmptyTiles(bool displayEmptyTiles) {
    _displayEmptyTiles = displayEmptyTiles;
  }

  // Clears the screen and then draws the tiles.
  void draw() const;

  // Use message to display text below the board. Use newlines for multiple lines.
  std::string message() const {return _message; }
  void setMessage(std::string newMessage) { _message = newMessage; }

  // Accessors for the board's tiles.
  Tile tileAt(unsigned row, unsigned col) const;
  void setTileAt(unsigned row, unsigned col, Tile tile);
  void setTileAt(unsigned row, unsigned col, char glyph, Tile::Color color = Tile::Color::vt100Default);

  // Convenience methods for setting tiles to empty.
  void clearAllTiles();
  void clearTileAt(unsigned row, unsigned col);

  // Glyph accessors provide an alternative to the tile accessors, for when you don't care about color.
  char glyphAt(unsigned row, unsigned col) const;
  void setGlyphAt(unsigned row, unsigned col, char glyph);

  // Commands are generally just the character pressed, e.g. 'a', ' ', 'x'.
  // There are constants for special keys, e.g. the arrow keys - listed below.
  enum CommandKey : char;

  // nextCommandKey returns the key pressed by the user. It can be used sychronously or asynchronously.
  // A timeout of zero mean nextCommand won't return until the user presses a key.
  // A non-zero timeout is how long, in tenths of a second, to wait for a key press
  // until giving up and returning noKey.
  char nextCommandKey(unsigned timeout = 0);

  // Prints a command to stdout to help with debugging.
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
