#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using mat = std::vector<std::vector<char>>;
namespace fs = std::filesystem;

struct position {
    int64_t m;    // row
    int64_t n;    // col
    bool operator==(const position& rhs) const {
        return (rhs.m == m) && (rhs.n == n);
    }

    friend bool operator<(const position& lhs, const position& rhs);
};

bool operator<(const position& lhs, const position& rhs) {
    return lhs.m < rhs.m || (lhs.m == rhs.m && lhs.n < rhs.n);
}

struct antenna_pair {
    const char frequency;
    std::vector<position> positions;
};
struct antinode_pair {
    const char frequency;
    std::vector<position> positions;
};

// forward declaration to put utility functions at file end;
mat get_map(const fs::path& path);
void print_map(const mat& map);
std::vector<antenna_pair> get_antenna_pairs(const mat& map);

[[nodiscard]] bool
    is_position_in_bounds(const position& pos, const int64_t width, const int64_t height) {
    return (pos.m >= 0) && (pos.n >= 0) && (pos.m < height) && (pos.n < width);
}

[[nodiscard]] bool is_duplicate_position(const std::vector<position>& positions,
                                         const position& pos) {
    return std::find(positions.cbegin(), positions.cend(), pos) != positions.cend();
}
void print_positions(const std::vector<position>& pos, const fs::path& s) {
    std::ofstream ifs {fs::canonical(s)};
    if(!ifs.good())
        std::cout << "[ERROR]: Couldn't open file " << s.string() << "\n";
    for(const auto& p: pos)
        ifs << std::format("[{}, {}]\n", p.m, p.n);
}

namespace stage_1 {

int64_t map_width = 0;
int64_t map_height = 0;

antinode_pair calculate_antinode_positions(const antenna_pair& pair) {
    const std::size_t antennas_sz = pair.positions.size();
    const std::vector<position>& antennas = pair.positions;

    antinode_pair antinodes {.frequency = pair.frequency, .positions = {}};

    for(std::size_t antenna_i = 0U; antenna_i < (antennas_sz - 1U); ++antenna_i) {
        for(std::size_t antenna_j = antenna_i + 1U; antenna_j < antennas_sz; ++antenna_j) {
            const int64_t distance_m = antennas.at(antenna_j).m - antennas.at(antenna_i).m;
            const int64_t distance_n = antennas.at(antenna_j).n - antennas.at(antenna_i).n;

            const position antinode_1 {.m = antennas.at(antenna_i).m - distance_m,
                                       .n = antennas.at(antenna_i).n - distance_n};
            const position antinode_2 {.m = antennas.at(antenna_j).m + distance_m,
                                       .n = antennas.at(antenna_j).n + distance_n};
            if(::is_position_in_bounds(antinode_1, map_width, map_height))
                antinodes.positions.emplace_back(antinode_1);
            if(::is_position_in_bounds(antinode_2, map_width, map_height))
                antinodes.positions.emplace_back(antinode_2);
        }
    }
    return antinodes;
}

std::size_t solve(const mat& map) {
    map_width = map[0].size();
    map_height = map.size();
    std::vector<antenna_pair> antenna_pairs = ::get_antenna_pairs(map);
    std::vector<antinode_pair> antinode_pairs;
    mat map_new = map;
    for(const auto& pair: antenna_pairs) {
        antinode_pair anti_pair = calculate_antinode_positions(pair);
        antinode_pairs.emplace_back(anti_pair);
    }

    std::vector<position> antenna_positions;
    std::vector<position> antinode_positions;

    for(const auto& pair: antenna_pairs) {
        antenna_positions.insert(
            antenna_positions.end(), pair.positions.begin(), pair.positions.end());
    }
    for(const auto& pair: antinode_pairs) {
        antinode_positions.insert(
            antinode_positions.end(), pair.positions.begin(), pair.positions.end());
    }
    // remove duplicates among antinodes
    std::sort(antinode_positions.begin(), antinode_positions.end());
    // print_positions(antinode_positions, "after_sort.txt");
    antinode_positions.erase(std::unique(antinode_positions.begin(), antinode_positions.end()),
                             antinode_positions.end());

    /*for(const position pos: antinode_positions) {
        // if(map_new.at(pos.m).at(pos.n) == '.')
        map_new[pos.m][pos.n] = '#';
    }
    print_map(map_new);*/
    return antinode_positions.size();
}
}
namespace stage_2 {
int64_t map_width = 0;
int64_t map_height = 0;
int64_t map_diagonal = 0;

antinode_pair calculate_antinode_positions(const antenna_pair& pair) {
    const std::size_t antennas_sz = pair.positions.size();
    const std::vector<position>& antennas = pair.positions;

    antinode_pair antinodes {.frequency = pair.frequency, .positions = {}};

    for(std::size_t antenna_i = 0U; antenna_i < (antennas_sz - 1U); ++antenna_i) {
        for(std::size_t antenna_j = antenna_i + 1U; antenna_j < antennas_sz; ++antenna_j) {
            const int64_t distance_m = antennas.at(antenna_j).m - antennas.at(antenna_i).m;
            const int64_t distance_n = antennas.at(antenna_j).n - antennas.at(antenna_i).n;

            for(int64_t n = 1;; ++n) {
                const position antinode_1 {.m = antennas.at(antenna_i).m - n * distance_m,
                                           .n = antennas.at(antenna_i).n - n * distance_n};
                if(!::is_position_in_bounds(antinode_1, map_width, map_height))
                    break;
                antinodes.positions.emplace_back(antinode_1);
            }
            for(int64_t n = 1;; ++n) {
                const position antinode_2 {.m = antennas.at(antenna_j).m + n * distance_m,
                                           .n = antennas.at(antenna_j).n + n * distance_n};
                if(!::is_position_in_bounds(antinode_2, map_width, map_height))
                    break;
                antinodes.positions.emplace_back(antinode_2);
            }
        }
    }
    return antinodes;
}

std::size_t solve(const mat& map) {
    map_width = map[0].size();
    map_height = map.size();

    map_diagonal = std::sqrt(map_width * map_width + map_height * map_height);
    std::vector<antenna_pair> antenna_pairs = ::get_antenna_pairs(map);
    std::vector<antinode_pair> antinode_pairs;
    mat map_new = map;
    for(const auto& pair: antenna_pairs) {
        antinode_pair anti_pair = calculate_antinode_positions(pair);
        antinode_pairs.emplace_back(anti_pair);
    }

    std::vector<position> antinode_positions;

    // antenna's also become antinodes in this example
    for(const auto& pair: antenna_pairs) {
        antinode_positions.insert(
            antinode_positions.end(), pair.positions.begin(), pair.positions.end());
    }
    for(const auto& pair: antinode_pairs) {
        antinode_positions.insert(
            antinode_positions.end(), pair.positions.begin(), pair.positions.end());
    }

    // remove duplicates among antinodes
    std::sort(antinode_positions.begin(), antinode_positions.end());
    antinode_positions.erase(std::unique(antinode_positions.begin(), antinode_positions.end()),
                             antinode_positions.end());

    /*for(const position pos: antinode_positions) {
        // if(map_new.at(pos.m).at(pos.n) == '.')
        map_new[pos.m][pos.n] = '#';
    }
    print_map(map_new);*/

    return antinode_positions.size();
}
}

int main(int argc, char** argv) {
    if(argc < 2U) {
        std::cout << "Invalid number of arguments\n";
        return 1;
    }

    mat map = get_map(argv[1]);

    try {
        std::chrono::time_point<std::chrono::high_resolution_clock> start =
            std::chrono::high_resolution_clock::now();
        uint32_t result_stage_1 = stage_1::solve(map);
        std::chrono::microseconds t_stage_1 = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start);

        start = std::chrono::high_resolution_clock::now();
        uint32_t result_stage_2 = stage_2::solve(map);
        std::chrono::microseconds t_stage_2 = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start);
        std::cout << std::format(
            "Map dimension (wxh): {}x{}\n", stage_1::map_width, stage_1::map_height);
        std::cout << std::format("Stage 1 result: {} solved in {} us\n", result_stage_1, t_stage_1);
        std::cout << std::format("Stage 2 result: {} solved in {} us\n", result_stage_2, t_stage_2);
    } catch(const std::exception& e) {
        std::cout << e.what() << "\n";
    }
}

std::vector<antenna_pair> get_antenna_pairs(const mat& map) {
    std::vector<antenna_pair> pairs;
    const std::size_t rows = map.size();
    const std::size_t cols = map[0].size();

    for(std::size_t m = 0U; m < rows; ++m) {
        for(std::size_t n = 0U; n < cols; ++n) {
            if(auto it = std::find_if(pairs.begin(),
                                      pairs.end(),
                                      [&](const antenna_pair& a_pair) {
                                          return map.at(m).at(n) == a_pair.frequency;
                                      });
               it != pairs.end()) {
                it->positions.push_back(
                    position {.m = static_cast<int64_t>(m), .n = static_cast<int64_t>(n)});
            } else {
                if(!(map.at(m).at(n) == '.'))
                    pairs.push_back(
                        antenna_pair {.frequency = map[m][n],
                                      .positions = {position {.m = static_cast<int64_t>(m),
                                                              .n = static_cast<int64_t>(n)}}});
            }
        }
    }
    return pairs;
}

mat get_map(const fs::path& path) {
    const fs::path ws_path = fs::canonical(path);
    std::ifstream ifs {ws_path};

    if(!ifs.good())
        throw std::runtime_error("Unable to open " + ws_path.string());
    std::string line;

    mat map;
    while(std::getline(ifs, line)) {
        std::vector<char> chars {line.cbegin(), line.cend()};
        if(!chars.empty())
            map.emplace_back(chars);
    }
    return map;
}

void print_map(const mat& map) {
    for(const auto line: map) {
        for(const auto ch: line)
            std::cout << ch;
        std::cout << "\n";
    }
    std::cout << "\n";
}
