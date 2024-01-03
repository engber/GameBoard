#include "GameBoard.h"

#include <iostream>
#include <sstream>

using namespace std;

string StatusLine(const GameBoard &board, bool highlightCoords, unsigned time) {
  ostringstream messageBuf;
  messageBuf << time << ": ";
  messageBuf << "VT100:" << (board.vt100Mode() ? "on" : "off") << " ";
  messageBuf << "Coords:" << (board.displayCoords() ? "on" : "off") << " ";
  messageBuf << "HCoords:" << (highlightCoords ? "on" : "off") << " ";
  messageBuf << "Dots:" << (board.displayEmptyTileDots() ? "on" : "off") << " ";
  messageBuf << "Nethack:" << (board.nethackKeyMode() ? "on" : "off") << " ";
  messageBuf << "WASD:" << (board.wasdKeyMode() ? "on" : "off") << " ";

  return messageBuf.str();
}

void GameBoardTestMain() {
  bool done = false;
  bool highlightCoords = true;
  
  int myRow = 5;
  int myCol = 5;
  unsigned time = 0;
  
  GameBoard board(20, 20);

  char cmd = noKey;

  while (!done) {
    board.setTileAt(myRow, myCol, '@', Color::red);
    
    if (highlightCoords) {
      board.setHighlightedCoords(myRow, myCol);
    } else {
      board.setHighlightedCoords();
    }

    board.setMessage(StatusLine(board, highlightCoords, time));

    board.updateConsole();

    board.clearTileAt(myRow, myCol);
    ++time;

    // cout << "command key: ";
    // GameBoard::printCommandKey(cmd);
    
    cmd = board.nextCommandKey();

    switch (cmd) {
      case arrowUpKey:
        myRow = max(myRow - 1, 0);
        break;
      case arrowDownKey:
        myRow = min(myRow + 1, (int)board.rowCount() - 1);
        break;
      case arrowRightKey:
        myCol = min(myCol + 1, (int)board.colCount() - 1);
        break;
      case arrowLeftKey:
        myCol = max(myCol - 1, 0);
        break;
      case arrowUpLeftKey:
        if (myRow > 0 && myCol > 0) {
          --myRow;
          --myCol;
        }
        break;
      case arrowUpRightKey:
        if (myRow > 0 && myCol < board.colCount() - 1) {
          --myRow;
          ++myCol;
        }
        break;
      case arrowDownLeftKey:
        if (myRow < board.rowCount() - 1 && myCol > 0) {
          ++myRow;
          --myCol;
        }
        break;
      case arrowDownRightKey:
        if (myRow < board.rowCount() - 1 && myCol < board.colCount() - 1) {
          ++myRow;
          ++myCol;
        }
        break;
      case 'C':
        board.setDisplayCoords(!board.displayCoords());
        break;
      case 'D':
        board.setDisplayEmptyTileDots(!board.displayEmptyTileDots());
        break;
      case 'H':
        highlightCoords = !highlightCoords;
        break;
      case 'N':
        board.setNethackKeyMode(!board.nethackKeyMode());
        break;
      case 'V':
        board.setVT100Mode(!board.vt100Mode());
        break;
      case 'W':
        board.setWASDKeyMode(!board.wasdKeyMode());
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        board << "logging a digit: " << (cmd - '0') << endl;
        break;
case 'q':
      case 'Q':
        done = true;
        break;
     default:
        // My eyes! The goggles do nothing.
        break;
    }
  }
}
