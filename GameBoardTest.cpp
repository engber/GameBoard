#include "GameBoard.h"

#include <algorithm>>
#include <iostream>
#include <sstream>

using namespace std;

void GameBoardTestMain() {
  bool done = false;
  bool highlightCoords = true;
  int myRow = 5;
  int myCol = 5;
  
  GameBoard board(20, 20);

  board.setHighlightedCoordsColor(Tile::Color::vt100DarkBlue);

  char cmd = noKey;

  while (!done) {
    board.clearAllTiles();
    board.setTileAt(myRow, myCol, '@', Tile::Color::vt100Red);
    if (highlightCoords) {
      board.setHighlightedCoords(myRow, myCol);
    } else {
      board.setHighlightedCoords();
    }

    ostringstream messageBuf;
    messageBuf << "VT100:" << (board.useVT100Graphics() ? "on" : "off") << " ";
    messageBuf << "Coords:" << (board.displayCoords() ? "on" : "off") << " ";
    messageBuf << "HCoords:" << (highlightCoords ? "on" : "off") << " ";
    messageBuf << "Dots:" << (board.displayEmptyTiles() ? "on" : "off") << " ";
    messageBuf << "Nethack:" << (board.nethackKeyMode() ? "on" : "off") << " ";
    messageBuf << "WASD:" << (board.wasdKeyMode() ? "on" : "off") << " ";
    messageBuf << endl;
    board.setMessage(messageBuf.str());
  
    board.draw();

    cout << "command key: ";
    GameBoard::printCommandKey(cmd);
    
    cmd = board.nextCommandKey();

    switch (cmd) {
      case arrowUpKey:
        myRow = max(myRow - 1, 0);
        break;
      case arrowDownKey:
        myRow = min(myRow + 1, (int)board.height() - 1);
        break;
      case arrowRightKey:
        myCol = min(myCol + 1, (int)board.width() - 1);
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
        if (myRow > 0 && myCol < board.width() - 1) {
          --myRow;
          ++myCol;
        }
        break;
      case arrowDownLeftKey:
        if (myRow < board.height() - 1 && myCol > 0) {
          ++myRow;
          --myCol;
        }
        break;
      case arrowDownRightKey:
        if (myRow < board.height() - 1 && myCol < board.width() - 1) {
          ++myRow;
          ++myCol;
        }
        break;
      case 'C':
        board.setDisplayCoords(!board.displayCoords());
        break;
      case 'D':
        board.setDisplayEmptyTiles(!board.displayEmptyTiles());
        break;
      case 'H':
        highlightCoords = !highlightCoords;
        break;
      case 'N':
        board.setNethackKeyMode(!board.nethackKeyMode());
        break;
      case 'V':
        board.setUseVT100Graphics(!board.useVT100Graphics());
        break;
      case 'W':
        board.setWASDKeyMode(!board.wasdKeyMode());
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
