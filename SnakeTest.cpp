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

SnakeNode *NewFirstNode(SnakeNode *head, unsigned row, unsigned col) {
  SnakeNode *newNode = new SnakeNode;
  newNode->row = row;
  newNode->col = col;
  newNode->next = head->next;
  head->next = newNode;

  return newNode;
}

SnakeNode *RemoveLastNode(SnakeNode *head) {
  // Returns, splices out, but does _not_ delete, the last node.
  SnakeNode *result = nullptr;
  SnakeNode *prevNode = head;
  SnakeNode *node = prevNode->next;
  
  while (node->next != nullptr) {
    prevNode = node;
    node = node->next;
  }
  result = node;
  prevNode->next = nullptr;

  return result;
}

void SnakeTestMain() {
  bool killed = false;
  bool gameOver = false;
  bool highlightCoords = true;
  int dr = 0;
  int dc = 1;

  GameBoard board(40, 40);

  ostringstream messageBuf;
  messageBuf << "VT100:" << (board.vt100Mode() ? "on" : "off") << " ";
  messageBuf << "Coords:" << (board.displayCoords() ? "on" : "off") << " ";
  messageBuf << "HCoords:" << (highlightCoords ? "on" : "off") << " ";
  messageBuf << "Dots:" << (board.displayEmptyTiles() ? "on" : "off") << " ";
  messageBuf << "Nethack:" << (board.nethackKeyMode() ? "on" : "off") << " ";
  messageBuf << "WASD:" << (board.wasdKeyMode() ? "on" : "off") << " ";
  messageBuf << endl;
  board.setMessage(messageBuf.str());

  SnakeNode *snake = new SnakeNode;
  snake->next = nullptr;
  SnakeNode *firstNode = NewFirstNode(snake, 2, 2);
  for (unsigned i = 0; i < 6; ++i) {
    firstNode = NewFirstNode(snake, firstNode->row + dr, firstNode->col + dc);
  }

  cout << "Press Any Key to Start\n";
  board.nextCommandKey(0);
  
  board.draw();

  while (!gameOver) {
    
    char firstGlyph = killed ? 'X' : '@';
    board.setTileAt(firstNode->row, firstNode->col, firstGlyph, Tile::Color::vt100Red);
    SnakeNode *node = firstNode->next;
    while (node != nullptr) {
      board.setTileAt(node->row, node->col, 'o', Tile::Color::vt100Red);
      node = node->next;
    }
    
    if (highlightCoords) {
      board.setHighlightedCoords(firstNode->row, firstNode->col);
    } else {
      board.setHighlightedCoords();
    }

    board.update();

    if (killed) {
      break;
    }
    
    char cmd = board.nextCommandKey(1);

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
        board.setVT100Mode(!board.vt100Mode());
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

    int nextRow = firstNode->row + dr;
    int nextCol = firstNode->col + dc;
    if (nextRow >= 0 && nextRow < board.height() && nextCol >= 0 && nextCol < board.width()) {
      if (board.glyphAt(nextRow, nextCol) == '\0') {
        firstNode = NewFirstNode(snake, nextRow, nextCol);
        SnakeNode *lastNode = RemoveLastNode(snake);
        board.clearTileAt(lastNode->row, lastNode->col);
        delete lastNode;
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