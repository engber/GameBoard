# To Do

## Features


## Issues

### Switch Tile to be 8 bytes, that way we can have all the fields be "normal" types and avoid bit twiddling.

### Move Color out of Tile so we don't have to declar Tile first in GameBoard.h

### Switch Color to be a regular enum.

### Switch Color to be an index into a _private_ array of encoded attribute values.

### Turning off VT100mode prints a board on _every_ update. The resulting boards scrolling in the console makes it look like there are a few board being displayed, all updating simultaneously. It would nice to figure a way to make it more obvious what's really happening. 

### When message line count changes it can overlap cursor & existing stdout.

### Hide cursor?

