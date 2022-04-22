#include <iostream>
#include <random>
#include <ranges>
#include <vector>

struct Cell {
  std::vector<int> options;
  int x, y;
};
template <typename ChangeHandler> class WaveFunctionCollapse {
public:
  int opt_count;
  int width, height;
  std::vector<Cell> cells;
  std::mt19937 rng;
  ChangeHandler change_handler;
  void init() {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        Cell &cell = cells[y * width + x];
        cell.x = x;
        cell.y = y;
        cell.options.resize(opt_count);
        for (int i = 0; i < opt_count; i++)
          cell.options[i] = i;
      }
    }
  }
  WaveFunctionCollapse(int width, int height, int opt_count,
                       ChangeHandler change_handler)
      : width(width), height(height), opt_count(opt_count),
        cells(width * height), change_handler(change_handler),
        rng(std::random_device{}()) {
    init();
  }
  Cell &operator()(int x, int y) { return cells[y * width + x]; }
  Cell &at(int x, int y) { return cells[y * width + x]; }
  void remove_option(int x, int y, int opt) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    Cell &cell = cells[y * width + x];
    if (std::ranges::count(cell.options, opt)) {
      std::erase(cell.options, opt);
      if (cell.options.size() == 1)
        change_handler(&cell, cell.options[0], this);
    }
  }
  bool is_done() {
    for (int y = 0; y < height; y++)
      for (int x = 0; x < width; x++)
        if (at(x, y).options.size() > 1) return false;
    return true;
  }
  bool broken() {
    for (int y = 0; y < height; y++)
      for (int x = 0; x < width; x++)
        if (at(x, y).options.size() == 0) return true;
    return false;
  }
  void step() {
    std::vector<Cell *> lowest_cells;
    int min_opt = opt_count + 1;
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        Cell *cell = &at(x, y);
        int opts = cell->options.size();
        if (opts <= 1) continue;
        if (opts < min_opt) {
          min_opt = opts;
          lowest_cells.clear();
          lowest_cells.push_back(cell);
        } else if (opts == min_opt)
          lowest_cells.push_back(cell);
      }
    }
    if (lowest_cells.size() == 0) return;
    Cell *cell = lowest_cells[rng() % lowest_cells.size()];
    int picked = cell->options[rng() % cell->options.size()];
    cell->options.clear();
    cell->options.push_back(picked);

    change_handler(cell, picked, this);
    return;
  }
};