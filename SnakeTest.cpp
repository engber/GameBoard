#include "GameBoard.h"

#include <algorithm>>
#include <iostream>
#include <sstream>

using namespace std;

struct SnakeNode {
  unsigned row;
  unsigned col;
  SnakeNode *next;
};

SnakeNode *NewHead(SnakeNode *snake, unsigned row, unsigned col) {
  SnakeNode *newHead = new SnakeNode;
  newHead->row = row;
  newHead->col = col;
  newHead->next = snake->next;
  snake->next = newHead;

  return newHead;
}

void DeleteTail(SnakeNode *snake) {
  SnakeNode *prevNode = snake;
  SnakeNode *node = prevNode->next;
  
  while (node->next != nullptr) {
    prevNode = node;
    node = node->next;
  }
  delete node;
  prevNode->next = nullptr;
}

void SnakeTestMain() {
  bool killed = false;
  bool gameOver = false;
  bool highlightCoords = true;
  int dr = 0;
  int dc = -1;

  GameBoard board(20, 20);

  SnakeNode *snake = new SnakeNode;
  snake->next = nullptr;
  SnakeNode *snakeHead = NewHead(snake, 2, 10);
  for (unsigned i = 0; i < 6; ++i) {
    snakeHead = NewHead(snake, snakeHead->row + dr, snakeHead->col + dc);
  }    

  while (!gameOver) {
    char headGlyph = killed ? 'X' : '@';
    board.clearAllTiles();
    board.setTileAt(snakeHead->row, snakeHead->col, headGlyph, Tile::Color::vt100Red);
    SnakeNode *node = snakeHead->next;
    while (node != nullptr) {
      board.setTileAt(node->row, node->col, 'o', Tile::Color::vt100Red);
      node = node->next;
    }
    
    if (highlightCoords) {
      board.setHighlightedCoords(snakeHead->row, snakeHead->col);
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

    if (killed) {
      break;
    }
    
    char cmd = board.nextCommandKey(4);

    switch (cmd) {
      case arrowUpKey:
        dr = max(-1, dr - 1);
        dc = 0;
        break;
      case arrowDownKey:
        dr = min(1, dr + 1);
        dc = 0;
        break;
      case arrowRightKey:
        dc = min(1, dc + 1);
        dr = 0;
        break;
      case arrowLeftKey:
        dc = max(-1, dc - 1);
        dr = 0;
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
        gameOver = true;
        break;
     default:
        // My eyes! The goggles do nothing.
        break;
    }

    int nextRow = snakeHead->row + dr;
    int nextCol = snakeHead->col + dc;
    if (nextRow >= 0 && nextRow < board.height() && nextCol >= 0 && nextCol < board.width()) {
      if (board.glyphAt(nextRow, nextCol) == '\0') {
        snakeHead = NewHead(snake, nextRow, nextCol);
        DeleteTail(snake);
      } else {
        killed = true;
      }
    } else {
      killed = true;
    }
  }

  if (killed) {
    cout << "snake killed\n";
  }
}
