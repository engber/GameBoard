#include "GameBoard.h"

#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <exception>
#include <iostream>

using namespace std;

#if __cplusplus < 201703L
// mse-evil - clamp added in C++17
#define clamp(v, lo, hi) ((v < lo) ? lo : ((v > hi) ? hi : v))
#endif

GameBoard::GameBoard(int width, int height) {
  _width = clamp(width, 1, 100);
  _height = clamp(height, 1, 100);
  _tiles = new Tile[_width * _height]();
  setDirtyOnAllTiles(true);
}

GameBoard::~GameBoard() { delete[] _tiles; }

string GameBoard::message() const {
  return _message;
}

void GameBoard::setMessage(string newMessage) {
  if (_message != newMessage) {
    _dirtyMessageLineCount = std::count(_message.begin(), _message.end(), '\n') + 1;
    _message = newMessage;
  }
}

int GameBoard::illegalCoord() {
  return std::numeric_limits<int>::max();
}

void GameBoard::rangeCheck(int row, int col) const {
  if (row < 0 || col < 0 || row >= _width || col >= _height) {
    throw std::out_of_range("GameBoard:: illegal row or col");
  }
}

unsigned GameBoard::tileIndex(int row, int col) const {
  rangeCheck(row, col);
  return row * _width + col;
}

Tile GameBoard::tileAt(int row, int col) const {
  unsigned i = tileIndex(row, col);
  return Tile(_tiles[i], false);
}

Tile GameBoard::displayedTileAt(int row, int col) const {
  return _vt100Mode ? tileAt(row, col) : Tile(glyphAt(row, col));
}

void GameBoard::setTileAt(int row, int col, Tile tile) {
  unsigned i = tileIndex(row, col);
  if (_tiles[i] != tile) {
    _tiles[i] = Tile(tile, true);
  }
}

void GameBoard::setTileAt(int row, int col, char glyph,
                          Tile::Color color) {
  setTileAt(row, col, Tile(glyph, color));
}

char GameBoard::glyphAt(int row, int col) const {
  return tileAt(row, col).glyph();
}

void GameBoard::setGlyphAt(int row, int col, char glyph) {
  setTileAt(row, col, Tile(glyph, Tile::Color::vt100White));
}

void GameBoard::clearAllTiles() {
  Tile blank = Tile();
  for (int r = 0; r < _height; ++r) {
    for (int c = 0; c < _width; ++c) {
      setTileAt(r, c, blank);
    }
  }
}

void GameBoard::clearTileAt(int row, int col) {
  setTileAt(row, col, Tile());
}

void GameBoard::setDirtyOnAllTiles(bool dirty) const {
  unsigned tileCount = _width * _height;
  for (unsigned i = 0; i < tileCount; ++i) {
    _tiles[i] = Tile(_tiles[i], dirty);
  }
}

void GameBoard::setHighlightedCoords_(int row, int col) {  
  if (_highlightedRow != row) {
    if (_dirtyHighlightedRow == illegalCoord()) {
      _dirtyHighlightedRow = _highlightedRow;
    }
      _highlightedRow = row;
  }

  if (_highlightedCol != col) {
    if (_dirtyHighlightedCol == illegalCoord()) {
      _dirtyHighlightedCol = _highlightedCol;
    }
    _highlightedCol = col;
  }
};

void GameBoard::setHighlightedCoords(int row, int col) {
  rangeCheck(row, col);
  setHighlightedCoords_(row, col);
}

void GameBoard::setHighlightedCoords() {
  setHighlightedCoords_(illegalCoord(), illegalCoord());
};


// Use vt100GraphicsStart/vt100GraphicsEnd to bracket printing to stdout draw VT100 graphics characters.

void GameBoard::vt100GraphicsStart() const {
  if (_vt100Mode) {
    cout << "\x1B(0";
  }
}

void GameBoard::vt100GraphicsEnd() const {
  if (_vt100Mode) {
    cout << "\x1B(B";
  }
}

enum {
  // A selection of VT100 graphics characters
  vt100TLCorner = '\x6C',
  vt100TRCorner = '\x6B',
  vt100BLCorner = '\x6D',
  vt100BRCorner = '\x6A',
  vt100HLine = '\x71',
  vt100VLine = '\x78',
};

char GameBoard::topLeftCornerGlyph() const {
  return _vt100Mode ? vt100TLCorner : '+';
}

char GameBoard::topRightCornerGlyph() const {
  return _vt100Mode ? vt100TRCorner : '+';
}

char GameBoard::bottomLeftCornerGlyph() const {
  return _vt100Mode ? vt100BLCorner : '+';
}

char GameBoard::bottomRightCornerGlyph() const {
  return _vt100Mode ? vt100BRCorner : '+';
}

char GameBoard::horizontalLineGlyph() const {
  return _vt100Mode ? vt100HLine : '-';
}

char GameBoard::verticalLineGlyph() const {
  return _vt100Mode ? vt100VLine : '|';
}

void GameBoard::clearScreen() const {
  if (_vt100Mode) {
    // Clear screen (\x1B[2J) _and_ position cursor at 0,0 (\x1B[0;0H).
    cout << "\x1B[2J\x1B[0;0H";
  }
}

void GameBoard::drawTop(bool showCoords) const {
  const char *indent = showCoords ? "  " : "";

  if (showCoords) {
    cout << indent << ' ';
    for (int c = 0; c < _width; ++c) {
      bool highlight = _vt100Mode && c == _highlightedCol;
      if (highlight) {
        Tile::colorStart(_highlightedCoordsColor);
      }

      if (c > 9) {
        cout << c/10 << ' ';
      } else {
        cout << "  ";
      }

      if (highlight) {
        Tile::colorEnd(_highlightedCoordsColor);
      }
    }
    cout << endl;

    cout << indent << ' ';
    for (int c = 0; c < _width; ++c) {
      bool highlight = _vt100Mode && c == _highlightedCol;
      if (highlight) {
        Tile::colorStart(_highlightedCoordsColor);
      }

      cout << c % 10 << ' ';

      if (highlight) {
        Tile::colorEnd(_highlightedCoordsColor);
      }
    }
    cout << endl;
  }

  vt100GraphicsStart();
  cout << indent;
  cout << topLeftCornerGlyph();
  char line = horizontalLineGlyph();
  int drawCount = 2 * _width - 1;
  for (int c = 0; c < drawCount; c++) {
    cout << line;
  }
  cout << topRightCornerGlyph();
  vt100GraphicsEnd();
  cout << endl;
}

void GameBoard::drawBottom(bool showCoords) const {
  const char *indent = showCoords ? "  " : "";

  vt100GraphicsStart();
  cout << indent;
  cout << bottomLeftCornerGlyph();
  char line = horizontalLineGlyph();
  int drawCount = 2 * _width - 1;
  for (int c = 0; c < drawCount; c++) {
    cout << line;
  }
  cout << bottomRightCornerGlyph();
  vt100GraphicsEnd();
  cout << endl;

  if (showCoords) {
    cout << indent << ' ';
    for (int c = 0; c < _width; ++c) {
      bool highlight = _vt100Mode && c == _highlightedCol;
      if (highlight) {
        Tile::colorStart(_highlightedCoordsColor);
      }

      cout << ((c < 10) ? c : c/10) << ' ';

      if (highlight) {
        Tile::colorEnd(_highlightedCoordsColor);
      }
    }
    cout << endl;

    cout << indent << ' ';
    for (int c = 0; c < _width; ++c) {
      if (c < 10) {
        cout << "  ";
      } else {
        bool highlight = _vt100Mode && c == _highlightedCol;
        if (highlight) {
          Tile::colorStart(_highlightedCoordsColor);
        }

        cout << c % 10 << ' ';

        if (highlight) {
          Tile::colorEnd(_highlightedCoordsColor);
        }
      }
    }
    cout << endl;
  }
}

void GameBoard::drawRow(int row, bool showCoords) const {
  if (showCoords) {
    bool highlight = _vt100Mode && row == _highlightedRow;
    if (highlight) {
      Tile::colorStart(_highlightedCoordsColor);
    }

    printf("%2d", row);

    if (highlight) {
      Tile::colorEnd(_highlightedCoordsColor);
    }
  }

  vt100GraphicsStart();
  cout << verticalLineGlyph();

  displayedTileAt(row, 0).draw(_displayEmptyTiles);
  for (int c = 1; c < _width; c++) {
    cout << ' '; // a space between cols makes the board appear more "square."
    Tile tile = displayedTileAt(row, c);

    // Escape mode interprets chars as special vt100 graphic glyphs.
    vt100GraphicsEnd();
    tile.draw(_displayEmptyTiles);
    vt100GraphicsStart();
  }
  cout << verticalLineGlyph();
  vt100GraphicsEnd();

  if (showCoords) {
    bool highlight = _vt100Mode && row == _highlightedRow;
    if (highlight) {
      Tile::colorStart(_highlightedCoordsColor);
    }

    printf("%-2d", row);

    if (highlight) {
      Tile::colorEnd(_highlightedCoordsColor);
    }
  }

  cout << endl;
}

void GameBoard::draw() const {
  clearScreen();

  drawTop(_displayCoords);

  for (int r = 0; r < _height; r++) {
    drawRow(r, _displayCoords);
  }

  drawBottom(_displayCoords);

  if (_message.length() > 0) {
    cout << _message << endl;
  }

  setDirtyOnAllTiles(false);
  _dirtyMessageLineCount = 0;
  _dirtyHighlightedRow = illegalCoord();
  _dirtyHighlightedCol = illegalCoord();
}

void GameBoard::update() const {
  cout << "\x1B" "7"; // save cursor & attrs
  
  int vt100CoordOffset = _displayCoords ? 2 : 0;

  unsigned tileIndex = 0;
  for (int r = 0; r < _height; ++r) {
    for (int c = 0; c < _width; ++c) {
      Tile tile(_tiles[tileIndex]);
      if (tile.isDirty()) {
        // vt100 numbers rows/cols starting with one.
        int vt100Row = r + 2 + vt100CoordOffset;
        int vt100Col = 2 * c + 2 + vt100CoordOffset;
        printf("\x1B[%d;%dH", vt100Row, vt100Col); // position cursor

        tile.draw(_displayEmptyTiles);

        _tiles[tileIndex] = Tile(tile, false);
      }
      ++tileIndex;
    }
  }

  if (_displayCoords) {
    updateHighlightedCoords();
  }

  updateMessage();

  cout << "\x1B" "8"; // restore cursor & attrs
}

void GameBoard::updateMessage() const {
  int coordRowCount = _displayCoords ? 4 : 0;

  if (_dirtyMessageLineCount > 0) {
      for (int i = 0; i < _dirtyMessageLineCount; ++i) {
        // vt100 numbers rows/cols starting with one.
        int vt100Row = _height + i + 3 + coordRowCount;
        int vt100Col = 0;
        printf("\x1B[%u;%uH\x1B[2K", vt100Row, vt100Col); // position cursor & erase line
        }
      _dirtyMessageLineCount = 0;
  }

  if (_message.length() > 0) {
    // vt100 numbers rows/cols starting with one.
    unsigned vt100Row = _height + 3 + coordRowCount;
    unsigned vt100Col = 0;
    printf("\x1B[%d;%dH", vt100Row, vt100Col); // position cursor
    cout << _message << endl;
  }
}

void GameBoard::updateRowCoords(int row) const {
  int vt100Row = row + 4;
  int vt100ColLeft = 1;
  int vt100ColRight = _width*2 + 4;
  printf("\x1B[%d;%dH%2d", vt100Row, vt100ColLeft, row);
  printf("\x1B[%d;%dH%-2d", vt100Row, vt100ColRight, row);
}

void GameBoard::updateColCoords(int col) const {
  int vt100Row0 = 1;
  int vt100Row1 = 2;
  int vt100Row3 = _height + 5;
  int vt100Row4 = _height + 6;
  int vt100Col = col*2 + 4;

  if (col > 9) {
      printf("\x1B[%d;%dH%-2d", vt100Row0, vt100Col, col/10);
      printf("\x1B[%d;%dH%-2d", vt100Row4, vt100Col, col%10);
  }
  
  printf("\x1B[%d;%dH%-2d", vt100Row1, vt100Col, col%10);
  printf("\x1B[%d;%dH%-2d", vt100Row3, vt100Col, (col > 9) ? col/10 : col);
}

void GameBoard::updateHighlightedCoords() const {  
  if (_highlightedRow != illegalCoord() || _highlightedCol != illegalCoord()) {
    Tile::colorStart(_highlightedCoordsColor);
    updateRowCoords(_highlightedRow);
    updateColCoords(_highlightedCol);
    Tile::colorEnd(_highlightedCoordsColor);
  }
  
  if (_dirtyHighlightedRow != illegalCoord()) {
    Tile::colorStart(Tile::Color::vt100Default);
    updateRowCoords(_dirtyHighlightedRow);
    Tile::colorEnd(Tile::Color::vt100Default);
    _dirtyHighlightedRow = illegalCoord();
  }
  
  if (_dirtyHighlightedCol != illegalCoord()) {
    Tile::colorStart(Tile::Color::vt100Default);
    updateColCoords(_dirtyHighlightedCol);
    Tile::colorEnd(Tile::Color::vt100Default);
    _dirtyHighlightedCol = illegalCoord();
  }
}

char GameBoard::nethackCommandKey(char c) {
  // https://nethackwiki.com/wiki/Direction
  switch (c) {
  case 'k':
    return arrowUpKey;
  case 'j':
    return arrowDownKey;
  case 'l':
    return arrowRightKey;
  case 'h':
    return arrowLeftKey;
  case 'y':
    return arrowUpLeftKey;
  case 'u':
    return arrowUpRightKey;
  case 'b':
    return arrowDownLeftKey;
  case 'n':
    return arrowDownRightKey;
  default:
    return noKey;
  }
}

char GameBoard::wasdCommandKey(char c) {
  switch (c) {
  case 'w':
    return arrowUpKey;
  case 's':
    return arrowDownKey;
  case 'd':
    return arrowRightKey;
  case 'a':
    return arrowLeftKey;
  default:
    return noKey;
  }
}

char GameBoard::escapedCommandKey(char c) {
  // Interpret special keys based on the 3rd char of 3 or 4.
  // "\x1B[{c}" or "\x1B[{c}~"
  switch (c) {
  case 'A':
    return arrowUpKey;
  case 'B':
    return arrowDownKey;
  case 'C':
    return arrowRightKey;
  case 'D':
    return arrowLeftKey;
  case '3':
    return deleteForwardKey;
  case '5':
    return pageUpKey;
  case '6':
    return pageDownKey;
  default:
    return unknownKey;
  }
}

char GameBoard::normalCommandKey(char c) const {
  char result = c;

  // Nethack and WASD keys _don't_ overlap.

  if (_nethackKeyMode) {
    if (char newC = nethackCommandKey(c)) {
      result = newC;
    }
  }

  if (_wasdKeyMode) {
    if (char newC = wasdCommandKey(c)) {
      result = newC;
    }
  }

  return result;
}

char GameBoard::nextCommandKey(unsigned timeout) {
  char result = noKey;

  fflush(stdout);

  struct termios oldAttrs = {0};
  if (tcgetattr(0, &oldAttrs) < 0) {
    perror("tcgetattr()");
  }

  struct termios newAttrs = oldAttrs;

  cc_t vMin = (timeout == 0) ? 1 : 0; // sychronous, wait for at least one char
  cc_t vTime = timeout;
  newAttrs.c_cc[VMIN] = vMin;
  newAttrs.c_cc[VTIME] = vTime;
  newAttrs.c_lflag &= (~ICANON) & (~ECHO);

  if (tcsetattr(0, TCSANOW, &newAttrs) < 0) {
    perror("tcsetattr newAttrs");
  }

  // Discards queued up keys by only processing one key from buf.
  // The alternative approach, only reading one key's worth, is complicated
  // Esc sequences require looking ahead and pushing back chars if it turns
  // out not to be an esc sequence.
  char buf[32];
  ssize_t readCount = read(0, buf, sizeof(buf));
  if (readCount < 0) {
    perror("read()");
  }

  if (readCount > 0) {

    if (readCount > 2 && buf[0] == '\x1B' && buf[1] == '[') {
      result = escapedCommandKey(buf[2]);
    } else {
      result = normalCommandKey(buf[0]);
    }

    if (result == unknownKey) {
      printf("Unrecognized key %zu bytes:", readCount);
      for (int i = 0; i < readCount; ++i) {
        printf(" \\x%X ", buf[i]);
      }
      cout << endl;
    }
  }

  if (tcsetattr(0, TCSADRAIN, &oldAttrs) < 0) {
    perror("tcsetattr oldAttrs");
  }

  return result;
}

void GameBoard::printCommandKey(char cmd) {
#define NAMED_KEY_CASE(key)                                                    \
  case key:                                                                    \
    printf("%s\n", #key);                                                      \
    break
  switch (cmd) {
    NAMED_KEY_CASE(noKey);
    NAMED_KEY_CASE(unknownKey);
    NAMED_KEY_CASE(arrowUpKey);
    NAMED_KEY_CASE(arrowDownKey);
    NAMED_KEY_CASE(arrowRightKey);
    NAMED_KEY_CASE(arrowLeftKey);
    NAMED_KEY_CASE(arrowUpLeftKey);
    NAMED_KEY_CASE(arrowUpRightKey);
    NAMED_KEY_CASE(arrowDownLeftKey);
    NAMED_KEY_CASE(arrowDownRightKey);
    NAMED_KEY_CASE(tabKey);
    NAMED_KEY_CASE(enterKey);
    NAMED_KEY_CASE(escapeKey);
    NAMED_KEY_CASE(deleteKey);
    NAMED_KEY_CASE(pageUpKey);
    NAMED_KEY_CASE(pageDownKey);
    NAMED_KEY_CASE(deleteForwardKey);
  default:
    printf("\\x%X (%u) '%c'\n", cmd, cmd, cmd);
    break;
  }
#undef NAMED_KEY_CASE
}

// Tiles encode in the lower 26 bits: 3, 6-bit attribute values, and a 8-bit char/glyph.
// The upper 6 bits are private - the high bit is used by GameBoard to mark the tile dirty.
// DXXX XX00 0000 1111 1122 2222 GGGG GGGG
// dirty  attr0   attr1  attr2   glyph

enum : uint32_t {
  tileValueMask = 0x03FFFFFF,
  tileGlyphMask = 0x000000FF,
  tileColorMask = 0x03FFFF00,
  tileDirtyMask = 0x80000000,
};

Tile::Tile(const Tile &tile) { _4bytes = tile._4bytes; }

Tile::Tile(char glyph, Color color) {
  _4bytes = (glyph & tileGlyphMask) | (static_cast<uint32_t>(color) << 8);
};

Tile::Tile(const Tile &tile, bool dirty) {
  _4bytes = (tile._4bytes & tileValueMask) | (dirty ? tileDirtyMask : 0x0);
}

bool Tile::isDirty() const {
  return _4bytes & tileDirtyMask;
}

char Tile::glyph() const {
  return _4bytes & tileGlyphMask;
}

Tile::Color Tile::color() const {
  return Color((_4bytes & tileColorMask) >> 8);
}

// Compare the lower 26 bits.
bool Tile::operator== (const Tile &rhs) {
  return (this->_4bytes & tileValueMask) == (rhs._4bytes & tileValueMask);
}

bool Tile::operator!= (const Tile &rhs) {
  return (this->_4bytes & tileValueMask) != (rhs._4bytes & tileValueMask);
}

// Use colorStart/colorEnd to bracket printing to stdout to draw in the specified color.

void Tile::colorStart(Color color) {
  if (color != Color::vt100Default) {
    cout << "\x1B[";

    uint32_t colorBytes = static_cast<uint32_t>(color);
    int color0 = 0x3F & (colorBytes >> 12);
    int color1 = 0x3F & (colorBytes >> 6);
    int color2 = 0x3F & colorBytes;

    if (color0 != 0) {
      cout << color0 << ';';
    }

    if (color1 != 0) {
      cout << color1 << ';';
    }

    cout << color2 << 'm';
  }
}

void Tile::colorEnd(Color color) {
  if (color != Color::vt100Default) {
    cout << "\x1B[0m"; // reset attributes
  }
}

void Tile::draw(bool displayEmptyTiles) const {
  // Display attribute syntax: <ESC>[{attr1};...;{attrn}m
  char tileChar = glyph();

  if (tileChar != '\0') {
    Color tileColor = color();
    colorStart(tileColor);
    cout << tileChar;
    colorEnd(tileColor);
  } else if (displayEmptyTiles) {
    cout << "\x1B[2m•\x1B[0m"; // dim, dot, reset
  } else {
    cout << ' ';
  }
}
