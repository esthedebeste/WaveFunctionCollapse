// creates an image using wave function collapse with random rules
#include "wfc.cpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <ranges>
#include <string>
#include <vector>

struct RGB {
  uint8_t r, g, b;
  RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
  RGB() : r(0), g(0), b(0) {}
  bool operator==(const RGB &other) const {
    return r == other.r && g == other.g && b == other.b;
  }
};

enum Direction {
  TOP_LEFT,
  ABOVE,
  TOP_RIGHT,
  LEFT,
  RIGHT,
  BOTTOM_LEFT,
  BELOW,
  BOTTOM_RIGHT,
  MAX_DIRECTION
};
std::string directions[MAX_DIRECTION] = {
    "top left", "top",         "top right", "left",
    "right",    "bottom left", "below",     "bottom right"};
int rule_offsets[MAX_DIRECTION][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0},
                                      {1, 0},   {-1, 1}, {0, 1},  {1, 1}};
std::vector<RGB> all_colors = {
    {127, 127, 127}, {180, 60, 60},  {60, 180, 60},  {60, 60, 180},
    {180, 180, 60},  {60, 180, 180}, {180, 60, 180},
};
std::vector<RGB> colors;
int COLOR_COUNT = 3; // limit to 3 colors
int RULE_COUNT = 4;
std::vector<std::string> all_color_names = {"grey",   "red",  "green",  "blue",
                                            "yellow", "cyan", "magenta"};
std::vector<std::string> color_names;
struct Rule {
  int col1, col2; // indices in colors array
  Direction dir;
  Rule(int col1, int col2, Direction dir) : col1(col1), col2(col2), dir(dir) {}
  std::string stringify() {
    return color_names[col1] + " is not allowed to the " + directions[dir] +
           " of " + color_names[col2];
  }
};

int generate(std::string ppm_file, std::string rule_file,
             unsigned int seed = std::random_device{}()) {
  std::vector<Rule> rules;
  std::mt19937 rng(seed);
  colors.clear();
  color_names.clear();
  for (int i = 0; i < COLOR_COUNT; i++) {
    int col;
    do
      col = rng() % all_colors.size();
    while (std::ranges::count(colors, all_colors[col]));
    colors.push_back(all_colors[col]);
    color_names.push_back(all_color_names[col]);
  }
  for (int i = 0; i < RULE_COUNT; i++) {
    // generate random rule
    int col1 = rng() % COLOR_COUNT;
    int col2 = rng() % COLOR_COUNT;
    Direction rule = static_cast<Direction>(rng() % MAX_DIRECTION);
    rules.push_back(Rule(col1, col2, rule));
  }
  long width = 20, height = 20;
  bool done = false;
  auto change_handler = [=](Cell *cell, int picked, auto *wfc) mutable {
    for (Rule rule : rules) {
      if (picked == rule.col1)
        wfc->remove_option(cell->x - rule_offsets[rule.dir][0],
                           cell->y - rule_offsets[rule.dir][1], rule.col2);
      if (picked == rule.col2)
        wfc->remove_option(cell->x + rule_offsets[rule.dir][0],
                           cell->y + rule_offsets[rule.dir][1], rule.col1);
    }
  };
  WaveFunctionCollapse wfc(width, height, COLOR_COUNT, change_handler);
  std::cout << "colors: ";
  for (std::string color : color_names)
    std::cout << color << " ";
  std::cout << "\nrules:\n";
  for (Rule rule : rules)
    std::cout << " - " << rule.stringify() << "\n";
  std::cout << "generating..." << std::endl;
  int count = 0;
  do {
    wfc.init();
    while (!wfc.is_done())
      wfc.step();
    // keep going until we find a valid image.
    std::cout << "try " << ++count << '\r';
    if (count > 2000)
      return generate(ppm_file, rule_file); // give up on this ruleset
  } while (wfc.broken());
  std::cout << "done!\n";
  std::vector<RGB> image;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Cell &cell = wfc.at(x, y);
      image.push_back(cell.options.size() == 1 ? colors[cell.options[0]]
                                               : RGB(255, 0, 0));
    }
  }
  std::filesystem::create_directories(
      std::filesystem::path(ppm_file).parent_path());
  std::ofstream ppm(ppm_file);
  ppm << "P6\n" << width << " " << height << "\n255\n";
  for (RGB rgb : image)
    ppm << rgb.r << rgb.g << rgb.b;
  ppm.close();
  std::cout << "wrote image to " << ppm_file << "\n";
  std::filesystem::create_directories(
      std::filesystem::path(rule_file).parent_path());
  std::ofstream rules_file(rule_file);
  for (Rule rule : rules)
    rules_file << rule.stringify() << "\n";
  rules_file.close();
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc == 3)
    return generate(argv[1], argv[2]);
  else if (argc == 1)
    return generate("output.ppm", "rules.txt");
  else if (argc == 4) {
    std::string ppm_file = argv[1];
    std::string rule_file = argv[2];
    int amount = std::atoi(argv[3]);
    std::cout << "generating " << amount << " images\n";
    size_t ppm_i = ppm_file.find('%');
    size_t rule_i = rule_file.find('%');
    int prev_i_len = 1;
    for (int i = 1; i <= amount; i++) {
      std::cout << "generating image " << i << " of " << amount << '\n';
      std::string i_str = std::to_string(i);
      std::string ppm_file_i = ppm_file.replace(ppm_i, prev_i_len, i_str);
      std::string rule_file_i = rule_file.replace(rule_i, prev_i_len, i_str);
      generate(ppm_file_i, rule_file_i);
      std::cout << "done with " << i << "\n";
      prev_i_len = i_str.size();
    }
    std::cout << "done!\a\n";
    return 0;
  } else {
    std::cout << "usage: " << argv[0] << " <ppm_file> <rule_file> [amount]\n";
    return 1;
  }
  return 0;
}