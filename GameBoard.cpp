#include "GameBoard.h"

#include <termios.h>
#include <unistd.h>

#include <algorithm>>
#include <iostream>

using namespace std;

#if __cplusplus < 201703L
// mse-evil - clamp added in C++17
#define clamp(v, lo, hi) ((v < lo) ? lo : ((v > hi) ? hi : v))
#endif

GameBoard::GameBoard(unsigned width, unsigned height) {
  _width = clamp(width, 1u, 100u);
  _height = clamp(height, 1u, 100u);
  _tiles = new Tile[_width*_height]();
}

GameBoard::~GameBoard() {
  delete [] _tiles;
}

unsigned GameBoard::tileIndex(unsigned row, unsigned col) const {
  return row*_width + col;
}

Tile GameBoard::tileAt(unsigned row, unsigned col) const {
  unsigned i = tileIndex(row, col);
  return _tiles[i];
}

Tile GameBoard::displayedTileAt(unsigned row, unsigned col) const {
  return _useVT100Graphics ? tileAt(row, col) : Tile(glyphAt(row, col));
}

void GameBoard::setTileAt(unsigned row, unsigned col, Tile tile) {
  unsigned i = tileIndex(row, col);
  _tiles[i] = tile;
}

void GameBoard::setTileAt(unsigned row, unsigned col, char glyph, Tile::Color color) {
  unsigned i = tileIndex(row, col);
  _tiles[i] = Tile(glyph, color);
}

char GameBoard::glyphAt(unsigned row, unsigned col) const {
  unsigned i = tileIndex(row, col);
  return _tiles[i].glyph();
}

void GameBoard::setGlyphAt(unsigned row, unsigned col, char glyph) {
  unsigned i = tileIndex(row, col);
  _tiles[i] = Tile(glyph, Tile::Color::vt100White);
}

void GameBoard::clearAllTiles() {
  Tile blank;
  unsigned tileCount = _width*_height;
  for (unsigned i = 0; i < tileCount; ++i) {
    _tiles[i] = blank;
  }
}

void GameBoard::clearTileAt(unsigned row, unsigned col) {
  _tiles[row*_width + col] = Tile();
}

void GameBoard::setHighlightedCoords(unsigned row, unsigned col) {
  _highlightedRow = row;
  _highlightedCol = col;
};

void GameBoard::setHighlightedCoords() {
  _highlightedRow = std::numeric_limits<unsigned>::max();
  _highlightedCol = std::numeric_limits<unsigned>::max();
};


void GameBoard::vt100EscStart() const {
  if (_useVT100Graphics) {
    cout << "\x1B(0";
  }
}

void GameBoard::vt100EscEnd() const {
  if (_useVT100Graphics) {
    cout << "\x1B(B";
  }
}

enum {
  vt100TLCorner = '\x6C',
  vt100TRCorner = '\x6B',
  vt100BLCorner = '\x6D',
  vt100BRCorner = '\x6A',
  vt100HLine = '\x71',
  vt100VLine = '\x78',
};

char GameBoard::topLeftCornerGlyph() const {
  return _useVT100Graphics ? vt100TLCorner : '+';
}

char GameBoard::topRightCornerGlyph() const {
  return _useVT100Graphics ? vt100TRCorner : '+';
}

char GameBoard::bottomLeftCornerGlyph() const {
  return _useVT100Graphics ? vt100BLCorner : '+';
}

char GameBoard::bottomRightCornerGlyph() const {
  return _useVT100Graphics ? vt100BRCorner : '+';
}

char GameBoard::horizontalLineGlyph() const {
  return _useVT100Graphics ? vt100HLine : '-';
}

char GameBoard::verticalLineGlyph() const {
  return _useVT100Graphics ? vt100VLine : '|';
}

void GameBoard::clearScreen() const {
  if (_useVT100Graphics) {
    // Clear screen (\x1B[2J) _and_ position cursor at 0,0 (\x1B[0;0H).
    cout << "\x1B[2J\x1B[0;0H";
  }
}

void GameBoard::drawTop(bool showCoords) const {
  const char *indent = showCoords ? "  " : "";

  if (showCoords) {
    cout << indent << ' ';
    for (unsigned c = 0; c < _width; ++c) {
      bool highlight = _useVT100Graphics && c == _highlightedCol;
      if (highlight) {
        Tile::colorStart(_highlightedCoordsColor);
      }
      
      cout << ((c < 10) ? ' ' : '1') << ' ';
      
      if (highlight) {
        Tile::colorEnd(_highlightedCoordsColor);
      }
    }
    cout << endl;

    cout << indent << ' ';
    for (unsigned c = 0; c < _width; ++c) {
      bool highlight = _useVT100Graphics && c == _highlightedCol;
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

  vt100EscStart();
  cout << indent;
  cout << topLeftCornerGlyph();
  char line = horizontalLineGlyph();
  unsigned drawCount = 2 * _width - 1;
  for (unsigned c = 0; c < drawCount; c++) {
    cout << line;
  }
  cout << topRightCornerGlyph();
  vt100EscEnd();
  cout << endl;
}

void GameBoard::drawBottom(bool showCoords) const {
  const char *indent = showCoords ? "  " : "";

  vt100EscStart();
  cout << indent;
  cout << bottomLeftCornerGlyph();
  char line = horizontalLineGlyph();
  unsigned drawCount = 2 * _width - 1;
  for (unsigned c = 0; c < drawCount; c++) {
    cout << line;
  }
  cout << bottomRightCornerGlyph();
  vt100EscEnd();
  cout << endl;

  if (showCoords) {
    cout << indent << ' ';
    for (unsigned c = 0; c < _width; ++c) {
      bool highlight = _useVT100Graphics && c == _highlightedCol;
      if (highlight) {
        Tile::colorStart(_highlightedCoordsColor);
      }
      
      cout << ((c < 10) ? c : 1) << ' ';
      
      if (highlight) {
        Tile::colorEnd(_highlightedCoordsColor);
      }
    }
    cout << endl;

    cout << indent << ' ';
    for (unsigned c = 0; c < _width; ++c) {
      if (c < 10) {
        cout << "  ";
      } else {
        bool highlight = _useVT100Graphics && c == _highlightedCol;
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

void GameBoard::drawRow(unsigned row, bool showCoords) const {
  if (showCoords) {
    bool highlight = _useVT100Graphics && row == _highlightedRow;
    if (highlight) {
      Tile::colorStart(_highlightedCoordsColor);
    }

    // printf("%2u", row);
    cout << ((row < 10) ? ' ' : '1') << row % 10;
    
    if (highlight) {
      Tile::colorEnd(_highlightedCoordsColor);
    }
  }

  vt100EscStart();
  cout << verticalLineGlyph();

  displayedTileAt(row, 0).draw(_displayEmptyTiles);
  for (unsigned c = 1; c < _width; c++) {
    cout << ' '; // a space between cols makes the board appear more "square."
    Tile tile = displayedTileAt(row, c);

    // Escape mode interprets chars as special vt100 graphic glyphs.
    vt100EscEnd();
    tile.draw(_displayEmptyTiles);
    vt100EscStart();
  }
  cout << verticalLineGlyph();
  vt100EscEnd();

  if (showCoords) {
    bool highlight = _useVT100Graphics && row == _highlightedRow;
    if (highlight) {
      Tile::colorStart(_highlightedCoordsColor);
    }
    
    // printf("%-2u", row);
    if (row > 9) {
      cout << '1';
    }
    cout << row % 10;

    if (highlight) {
      Tile::colorEnd(_highlightedCoordsColor);
    }
  }

  cout << endl;
}

void GameBoard::draw() const {
  clearScreen();

  drawTop(_displayCoords);

  for (unsigned r = 0; r < _height; r++) {
    drawRow(r, _displayCoords);
  }

  drawBottom(_displayCoords);

  if (_message.length() > 0) {
    cout << _message << endl;
  }
}

char GameBoard::nethackCommandKey(char c) {
/* https://nethackwiki.com/wiki/Direction
🅈 🄺 🅄
🄷 🅇 🄻
🄱 🄹 🄽
*/
  switch (c) {
    case 'k': return arrowUpKey;
    case 'j': return arrowDownKey;
    case 'l': return arrowRightKey;
    case 'h': return arrowLeftKey;
    case 'y': return arrowUpLeftKey;
    case 'u': return arrowUpRightKey;
    case 'b': return arrowDownLeftKey;
    case 'n': return arrowDownRightKey;
    default: return noKey;
  }
}

char GameBoard::wasdCommandKey(char c) {
  switch (c) {
    case 'w': return arrowUpKey;
    case 's': return arrowDownKey;
    case 'd': return arrowRightKey;
    case 'a': return arrowLeftKey;
    default: return noKey;
  }
}

char GameBoard::escapedCommandKey(char c) {
// Interpret special keys based on the 3rd char of 3 or 4.
// "\x1B[{c}" or "\x1B[{c}~"
    switch (c) {
      case 'A': return arrowUpKey;
      case 'B': return arrowDownKey;
      case 'C': return arrowRightKey;
      case 'D': return arrowLeftKey;
      case '3': return deleteForwardKey;
      case '5': return pageUpKey;
      case '6': return pageDownKey;
      default: return unknownKey;
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
  #define NAMED_KEY_CASE(key) case key: printf("%s\n", #key); break
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

void Tile::colorStart(Color color) {
  if (color != Color::vt100Default) {
    cout << "\x1B[";

    uint32_t colorBytes = static_cast<uint32_t>(color);
    unsigned color0 = 0xFF & (colorBytes >> 16);
    unsigned color1 = 0xFF & (colorBytes >> 8);
    unsigned color2 = 0xFF & colorBytes;

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

bool Tile::operator== (const Tile &rhs)
{
   return this->_tileBytes == rhs._tileBytes;
}
