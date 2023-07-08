#include "GameBoard.h"

#include <iostream>
#include <sstream>

using namespace std;

void SimpleTestMain() {
  GameBoard board(20, 15);

  bool done = false;
  int row = 5, col = 3;
  while (!done) {
    // Sets a char at a position with a color.
    board.setTileAt(row, col, '@', Color::magenta);
    // Draws all tiles, or all that changed since the last draw().
    board.draw();

    // Erases the tile at row, col (providing the illusion of motion).
    board.clearTileAt(row, col);
    // The board can answer its height and width.
    row = (row + 1) % board.height();
    col = (col + 1) % board.width();
    // Wait 0.2s for user input (the param / 10 seconds).
    char cmd = board.nextCommandKey(2);
    done = cmd == 'q' || cmd == 'Q';
  }
}