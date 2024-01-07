// GameBoard version 1.1

#include "GameBoard.h"

#include <termios.h>
#include <unistd.h>

#include <algorithm>
#include <exception>
#include <limits>

using namespace std;

enum {
  kMaxRowCount = 50,
  kMaxColCount = 50,
};

enum : int {
  kIllegalCoord = std::numeric_limits<int>::max(),
};

GameBoard::GameBoard(int rowCount, int colCount) {
  if (rowCount < 0 || colCount < 0 || rowCount > kMaxRowCount ||
      colCount > kMaxColCount) {
    throw std::out_of_range("GameBoard:: rowCount & colCount must be 1..50");
  }
  _rowCount = rowCount;
  _colCount = colCount;

  _highlightedRow = kIllegalCoord;
  _highlightedCol = kIllegalCoord;
  _dirtyHighlightedRow = kIllegalCoord;
  _dirtyHighlightedCol = kIllegalCoord;
  _highlightedCoordsColor = Color::blue;

  _tiles = new Tile[_rowCount * _colCount]();
}

GameBoard::~GameBoard() { delete[] _tiles; }

void GameBoard::setDisplayCoords(bool displayCoords) {
  _redrawNeeded = true;
  _displayCoords = displayCoords;
}

void GameBoard::setHighlightedCoordsColor(Color color) {
  _redrawNeeded = true;
  _highlightedCoordsColor = color;
};

void GameBoard::setVT100Mode(bool vt100Mode) {
  _redrawNeeded = true;
  _vt100Mode = vt100Mode;
}
void GameBoard::setDisplayEmptyTileDots(bool displayEmptyTileDots) {
  _redrawNeeded = true;
  _displayEmptyTileDots = displayEmptyTileDots;
}

string GameBoard::message(int messageLineNumber) const {
  if (messageLineNumber < 0 || messageLineNumber > 1) {
    throw std::out_of_range("GameBoard:: illegal message line number:" +
                            to_string(messageLineNumber));
  }
  return _messageLines[messageLineNumber];
}

void GameBoard::setMessage(string newMessage, int messageLineNumber) {
  if (messageLineNumber < 0 || messageLineNumber > 1) {
    throw std::out_of_range("GameBoard:: illegal message line number:" +
                            to_string(messageLineNumber));
  }

  auto newLineCount = std::count(newMessage.begin(), newMessage.end(), '\n');

  if (messageLineNumber == 0) {
    if (newLineCount > 1) {
      throw std::out_of_range(
          "GameBoard:: illegal message: only one newline allowed.");
    } else if (newLineCount == 0) {
      _messageLines[0] = newMessage;
    } else {
      // A message for line zero containing a _single_ newline is interpreted as
      // two messages.
      size_t index = newMessage.find('\n');
      _messageLines[0] = newMessage.substr(0, index);
      _messageLines[1] = newMessage.substr(index + 1);
    }
  } else {
    if (newLineCount > 0) {
      throw std::out_of_range(
          "GameBoard:: illegal message: newlines not allowed.");
    }
    _messageLines[1] = newMessage;
  }

  drawMessage();
}

void GameBoard::log(vector<string> strings) {
  for (int i = 0; i < strings.size(); i++) {
    _logLines.push_back(strings[i]);
    if (_logLines.size() > _logLineCount) {
      _logLines.erase(_logLines.begin());
    }
  }
  drawLog();
}

void GameBoard::handleInsertion() {
  string str = _stringStream.str();
  size_t strIndex = 0;
  size_t newlineIndex = string::npos;
  vector<string>lines;

  while ((newlineIndex = str.find('\n', strIndex)) != string::npos) {
    lines.push_back(str.substr(strIndex, newlineIndex - strIndex));
    strIndex = newlineIndex + 1;
  }
  log(lines);

  if (strIndex > 0) {
    string remainder = str.substr(strIndex);
    _stringStream.str(remainder);
    _stringStream.rdbuf()->pubseekpos(remainder.length());
  }
}

GameBoard &GameBoard::operator<<(std::ostream &(*func)(std::ostream &)) {
  func(_stringStream);
  handleInsertion();
  return *this;
}

void GameBoard::rangeCheck(int row, int col) const {
  if (row < 0 || col < 0 || row >= _rowCount || col >= _colCount) {
    throw std::out_of_range("GameBoard:: illegal row("s + to_string(row) +
                            ") or col(" + to_string(col) + ")");
  }
}

unsigned GameBoard::tileIndex(int row, int col) const {
  rangeCheck(row, col);
  return row * _colCount + col;
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

void GameBoard::setTileAt(int row, int col, char glyph, Color color) {
  setTileAt(row, col, Tile(glyph, color));
}

char GameBoard::glyphAt(int row, int col) const {
  return tileAt(row, col).glyph();
}

void GameBoard::setGlyphAt(int row, int col, char glyph) {
  setTileAt(row, col, Tile(glyph, Color::white));
}

void GameBoard::clearAllTiles() {
  Tile blank = Tile();
  for (int r = 0; r < _rowCount; ++r) {
    for (int c = 0; c < _colCount; ++c) {
      setTileAt(r, c, blank);
    }
  }
}

void GameBoard::clearTileAt(int row, int col) { setTileAt(row, col, Tile()); }

void GameBoard::setDirtyOnAllTiles(bool dirty) const {
  unsigned tileCount = _rowCount * _colCount;
  for (unsigned i = 0; i < tileCount; ++i) {
    _tiles[i] = Tile(_tiles[i], dirty);
  }
}

void GameBoard::setHighlightedCoords_(int row, int col) {
  /* Setting new coords to be highlighted requires remember to previous coords
  so they can be redrawn un-highlighted. The data members, _dirtyHighlightedRow
  and _dirtyHighlightedCol are used to remember them. A value of kIllegalCoord
  indicate there is no previous coord to unhighlight. Redrawing will reset
  _dirtyHighlightedRow and _dirtyHighlightedCol to kIllegalCoord.

  Note: only set _dirtyHighlightedRow and _dirtyHighlightedCol once.
  When the highlighted coords are changed multiple times before redrawing,
  we only need to handle the _first_ set of previous coords.
   */
  if (_highlightedRow != row) {
    if (_dirtyHighlightedRow == kIllegalCoord) {
      _dirtyHighlightedRow = _highlightedRow;
    }
    _highlightedRow = row;
  }

  if (_highlightedCol != col) {
    if (_dirtyHighlightedCol == kIllegalCoord) {
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
  setHighlightedCoords_(kIllegalCoord, kIllegalCoord);
};

// Use vt100GraphicsStart/vt100GraphicsEnd to bracket printing to stdout draw
// VT100 graphics characters.

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

enum : char {
  // A selection of VT100 graphics characters
  kVT100TLCorner = '\x6C',
  kVT100TRCorner = '\x6B',
  kVT100BLCorner = '\x6D',
  kVT100BRCorner = '\x6A',
  kVT100HLine = '\x71',
  kVT100VLine = '\x78',
};

char GameBoard::topLeftCornerGlyph() const {
  return _vt100Mode ? kVT100TLCorner : '+';
}

char GameBoard::topRightCornerGlyph() const {
  return _vt100Mode ? kVT100TRCorner : '+';
}

char GameBoard::bottomLeftCornerGlyph() const {
  return _vt100Mode ? kVT100BLCorner : '+';
}

char GameBoard::bottomRightCornerGlyph() const {
  return _vt100Mode ? kVT100BRCorner : '+';
}

char GameBoard::horizontalLineGlyph() const {
  return _vt100Mode ? kVT100HLine : '-';
}

char GameBoard::verticalLineGlyph() const {
  return _vt100Mode ? kVT100VLine : '|';
}

void GameBoard::updateConsole() const {
  if (_redrawNeeded || !_vt100Mode) {
    redraw();
    _redrawNeeded = false;
  } else {
    update();
  }
}

void GameBoard::redrawConsole() const {
  _redrawNeeded = true;
  updateConsole();
}

void GameBoard::drawTop(bool showCoords) const {
  const char *indent = showCoords ? "  " : "";

  if (showCoords) {
    cout << indent << ' ';
    for (int c = 0; c < _colCount; ++c) {
      bool highlight = _vt100Mode && c == _highlightedCol;
      if (highlight) {
        Tile::colorStart(_highlightedCoordsColor);
      }

      if (c > 9) {
        cout << c / 10 << ' ';
      } else {
        cout << "  ";
      }

      if (highlight) {
        Tile::colorEnd(_highlightedCoordsColor);
      }
    }
    cout << endl;

    cout << indent << ' ';
    for (int c = 0; c < _colCount; ++c) {
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
  int drawCount = 2 * _colCount - 1;
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
  int drawCount = 2 * _colCount - 1;
  for (int c = 0; c < drawCount; c++) {
    cout << line;
  }
  cout << bottomRightCornerGlyph();
  vt100GraphicsEnd();
  cout << endl;

  if (showCoords) {
    cout << indent << ' ';
    for (int c = 0; c < _colCount; ++c) {
      bool highlight = _vt100Mode && c == _highlightedCol;
      if (highlight) {
        Tile::colorStart(_highlightedCoordsColor);
      }

      cout << ((c < 10) ? c : c / 10) << ' ';

      if (highlight) {
        Tile::colorEnd(_highlightedCoordsColor);
      }
    }
    cout << endl;

    cout << indent << ' ';
    for (int c = 0; c < _colCount; ++c) {
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

  // Escape mode interprets chars as special vt100 graphic glyphs.
  vt100GraphicsStart();
  cout << verticalLineGlyph();
  vt100GraphicsEnd();

  displayedTileAt(row, 0).draw(_displayEmptyTileDots);
  for (int c = 1; c < _colCount; c++) {
    cout << ' '; // a space between cols makes the board appear more "square."
    Tile tile = displayedTileAt(row, c);
    tile.draw(_displayEmptyTileDots);
  }

  // Escape mode interprets chars as special vt100 graphic glyphs.
  vt100GraphicsStart();
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

void GameBoard::clearScreen() const {
  if (_vt100Mode) {
    // Clear screens (\x1B[2J) _and_ positions cursor at 0,0 (\x1B[0;0H).
    cout << "\x1B[2J\x1B[0;0H";
  }
}

void GameBoard::redraw() const {
  clearScreen();

  drawTop(_displayCoords);

  for (int r = 0; r < _rowCount; r++) {
    drawRow(r, _displayCoords);
  }

  drawBottom(_displayCoords);

  drawMessage();
  drawLog();

  setDirtyOnAllTiles(false);
  _dirtyHighlightedRow = kIllegalCoord;
  _dirtyHighlightedCol = kIllegalCoord;
}

void GameBoard::update() const {
  cout << "\x1B"
          "7"; // save cursor & attrs

  int vt100CoordOffset = _displayCoords ? 2 : 0;

  unsigned tileIndex = 0;
  for (int r = 0; r < _rowCount; ++r) {
    for (int c = 0; c < _colCount; ++c) {
      Tile tile(_tiles[tileIndex]);
      if (tile.isDirty()) {
        // vt100 numbers rows/cols starting with one.
        int vt100Row = r + 2 + vt100CoordOffset;
        int vt100Col = 2 * c + 2 + vt100CoordOffset;
        printf("\x1B[%d;%dH", vt100Row, vt100Col); // position cursor

        tile.draw(_displayEmptyTileDots);

        _tiles[tileIndex] = Tile(tile, false);
      }
      ++tileIndex;
    }
  }

  if (_displayCoords) {
    updateHighlightedCoords();
  }

  cout << "\x1B"
          "8"; // restore cursor & attrs
}

int GameBoard::firstLogLineVT100Row() const {
  return firstMessageLineVT100Row() + _messageLines.size();
}

int GameBoard::firstMessageLineVT100Row() const {
  return _rowCount + 3 + (_displayCoords ? 4 : 0);
}

void GameBoard::drawMessage() const {
  cout << "\x1B"
          "7"; // save cursor & attrs

  int firstRow = firstMessageLineVT100Row();
  size_t messageCount = _messageLines.size();
  for (int i = 0; i < messageCount; ++i) {
    printf("\x1B[%u;%uH\x1B[2K", firstRow + i,
           0); // position cursor & erase line
    cout << _messageLines[i] << endl;
  }

  cout << "\x1B"
          "8"; // restore cursor & attrs
}

void GameBoard::drawLog() const {
  int firstRow = firstLogLineVT100Row();
  size_t logCount = _logLines.size();
  for (int i = 0; i < logCount; ++i) {
    printf("\x1B[%u;%uH\x1B[2K", firstRow + i,
           0); // position cursor & erase line
    cout << _logLines[i] << endl;
  }
}

void GameBoard::clearLog() {
  int firstRow = firstLogLineVT100Row();
  for (int i = 0; i < _logLineCount; ++i) {
    printf("\x1B[%u;%uH\x1B[2K", firstRow + i,
           0); // position cursor & erase line
  }
  _logLines.clear();
}

void GameBoard::setLogLineCount(int count) {
  // Intended to be called _before_ the board is first drawn.
  // No attempt is made to update existing log lines in the console.
  while (_logLines.size() > count) {
    _logLines.erase(_logLines.begin());
  }
  _logLineCount = count;
}

void GameBoard::updateRowCoords(int row) const {
  int vt100Row = row + 4;
  int vt100ColLeft = 1;
  int vt100ColRight = _colCount * 2 + 4;
  printf("\x1B[%d;%dH%2d", vt100Row, vt100ColLeft, row);
  printf("\x1B[%d;%dH%-2d", vt100Row, vt100ColRight, row);
}

void GameBoard::updateColCoords(int col) const {
  int vt100Row0 = 1;
  int vt100Row1 = 2;
  int vt100Row3 = _rowCount + 5;
  int vt100Row4 = _rowCount + 6;
  int vt100Col = col * 2 + 4;

  if (col > 9) {
    printf("\x1B[%d;%dH%-2d", vt100Row0, vt100Col, col / 10);
    printf("\x1B[%d;%dH%-2d", vt100Row4, vt100Col, col % 10);
  }

  printf("\x1B[%d;%dH%-2d", vt100Row1, vt100Col, col % 10);
  printf("\x1B[%d;%dH%-2d", vt100Row3, vt100Col, (col > 9) ? col / 10 : col);
}

void GameBoard::updateHighlightedCoords() const {
  if (_highlightedRow != kIllegalCoord || _highlightedCol != kIllegalCoord) {
    Tile::colorStart(_highlightedCoordsColor);
    updateRowCoords(_highlightedRow);
    updateColCoords(_highlightedCol);
    Tile::colorEnd(_highlightedCoordsColor);
  }

  if (_dirtyHighlightedRow != kIllegalCoord) {
    Tile::colorStart(Color::defaultColor);
    updateRowCoords(_dirtyHighlightedRow);
    Tile::colorEnd(Color::defaultColor);
    _dirtyHighlightedRow = kIllegalCoord;
  }

  if (_dirtyHighlightedCol != kIllegalCoord) {
    Tile::colorStart(Color::defaultColor);
    updateColCoords(_dirtyHighlightedCol);
    Tile::colorEnd(Color::defaultColor);
    _dirtyHighlightedCol = kIllegalCoord;
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

/*****************************************************************************/
/*****************************************************************************/

Tile::Tile(char glyph, Color color)
    : _glyph(glyph), _color(color), _dirty(false) {}

Tile::Tile(char glyph) : Tile::Tile(glyph, Color::defaultColor) {}

Tile::Tile(const Tile &tile, bool dirty) : Tile(tile) { _dirty = dirty; }

bool Tile::operator==(const Tile &rhs) {
  return _glyph == rhs._glyph && _color == rhs._color;
}

bool Tile::operator!=(const Tile &rhs) { return !(*this == rhs); }

// Use colorStart/colorEnd to bracket printing to stdout to draw in the
// specified color.

void Tile::colorStart(Color color) {
  static const char *vt100ColorCodes[] = {
      // Display attribute syntax: <ESC>[{attr1};...;{attrn}m

      "\x1B[0m",    // default
      "\x1B[30m",   // black
      "\x1B[31m",   // red
      "\x1B[32m",   // green
      "\x1B[33m",   // yellow
      "\x1B[34m",   // blue
      "\x1B[35m",   // magenta
      "\x1B[36m",   // cyan
      "\x1B[37m",   // white
      "\x1B[2;31m", // dark red
      "\x1B[2;34m", // dark blue
      "\x1B[2;37m", // dark white
  };

  cout << vt100ColorCodes[color];
}

void Tile::colorEnd(Color color) {
  if (color != Color::defaultColor) {
    cout << "\x1B[0m"; // reset attributes
  }
}

// Called only by GameBoard to draw at the current cursor.
void Tile::draw(bool displayEmptyTileDots) const {
  if (_glyph != '\0') {
    colorStart(_color);
    cout << _glyph;
    colorEnd(_color);
  } else if (displayEmptyTileDots) {
    cout << "\x1B[2m•\x1B[0m"; // dim, dot, reset
  } else {
    cout << ' ';
  }
}
