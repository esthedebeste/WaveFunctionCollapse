// creates a sudoku using wave function collapse
#include "wfc.cpp"
#include <iostream>

int main() {
  WaveFunctionCollapse wfc(9, 9, 9, [=](Cell *cell, int picked, auto *wfc) {
    for (int i = 0; i < wfc->width; i++) {
      Cell *horizontal = &wfc->at(i, cell->y);
      if (horizontal != cell) std::erase(horizontal->options, picked);

      Cell *vertical = &wfc->at(cell->x, i);
      if (vertical != cell) std::erase(vertical->options, picked);
    }
    // 3x3 block
    int block_x = cell->x / 3 * 3;
    int block_y = cell->y / 3 * 3;
    for (int y = 0; y < 3; y++) {
      for (int x = 0; x < 3; x++) {
        Cell *c = &wfc->at(block_x + x, block_y + y);
        if (c != cell) std::erase(c->options, picked);
      }
    }
  });
  std::cout << "generating...\n";
  int count = 0;
  do {
    wfc.init();
    while (!wfc.is_done())
      wfc.step();
    // keep going until we find a valid sudoku.
    std::cout << "try " << ++count << '\r';
  } while (wfc.broken());
  std::cout << "\a\nfound a valid sudoku!\n";
  for (int y = 0; y < wfc.width; y++) {
    for (int x = 0; x < wfc.height; x++) {
      Cell &cell = wfc.at(x, y);
      std::cout << cell.options[0] + 1 << " ";
    }
    std::cout << std::endl;
  }
  return 0;
}