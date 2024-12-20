#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

namespace fs = std::filesystem;
using mat = std::vector<std::vector<char>>;

mat get_input(const fs::path& path);
void print_map(const mat& map);

struct vertex {
    int64_t m;
    int64_t n;
    char height;
    bool operator==(const vertex& rhs) const {
        return (rhs.m == m) && (rhs.n == n);
    }
    friend bool operator<(const vertex& lhs, const vertex& rhs);
};

bool operator<(const vertex& lhs, const vertex& rhs) {
    return (lhs.m < rhs.m) || ((lhs.m == rhs.m) && (lhs.n < rhs.n));
}

template<>
struct std::formatter<vertex> : std::formatter<std::string> {
    auto format(vertex v, format_context& ctx) const {
        return formatter<std::string>::format(
            std::format("[{}, {}, {}]", v.m, v.n, static_cast<uint32_t>(v.height)), ctx);
    }
};

static constexpr std::array<std::array<int64_t, 2>, 4> directions {
    {{-1, 0} /*UP*/, {0, 1} /*RIGHT*/, {1, 0} /*DOWN*/, {0, -1} /*LEFT*/}};

bool is_in_bounds(const vertex& v, const int64_t w, const int64_t h) {
    return (v.m > -1) && (v.n > -1) && (v.m < h) && (v.n < w);
}
std::vector<vertex> find_height_points(const mat& map, const char height) {
    const std::size_t rows = map.size();
    const std::size_t cols = map[0].size();
    std::vector<vertex> height_points;
    for(std::size_t m = 0U; m < rows; ++m) {
        for(std::size_t n = 0U; n < cols; ++n) {
            if(map[m][n] == height) {
                height_points.push_back(vertex {
                    .m = static_cast<int64_t>(m), .n = static_cast<int64_t>(n), .height = height});
            }
        }
    }
    return height_points;
}

namespace stage_1 {

int64_t map_height {};
int64_t map_width {};

struct node {
    vertex data;
    std::vector<node> children;
};

void dfs(const std::optional<vertex>& parent,
         const vertex& current,
         const mat& map,
         std::vector<vertex>& peaks) {
    // parent is needed, current vertex is need. Find adjacent vertices that satisfy
    // condition of not being equal to the parent and having a height + 1
    if(current.height == 9U) {
        peaks.push_back(current);
        return;
    }

    std::vector<vertex> adjacent_vertices;
    for(const std::array<int64_t, 2U> direction: ::directions) {
        const int64_t new_m = current.m + direction[0U];
        const int64_t new_n = current.n + direction[1U];
        vertex v {.m = new_m, .n = new_n};
        if(!::is_in_bounds(v, map_width, map_height) || ((map[new_m][new_n] - 1) != current.height)
           || (parent && (new_m == parent->m) && (new_n == parent->n)))
            continue;
        v.height = current.height + 1U;
        adjacent_vertices.push_back(v);
    }
    for(const vertex v: adjacent_vertices)
        dfs(current, v, map, peaks);
}

uint64_t solve(const mat& map) {
    map_height = map.size();
    map_width = map[0].size();
    std::vector<vertex> trailheads = ::find_height_points(map, 0U);
    // std::cout << std::format("Exploring {} trailheads\n", trailheads.size());
    uint64_t score {};
    for(const vertex& trailhead: trailheads) {
        std::vector<vertex> peaks;
        dfs(std::nullopt, trailhead, map, peaks);
        std::sort(peaks.begin(), peaks.end());
        peaks.erase(std::unique(peaks.begin(), peaks.end()), peaks.end());
        score += peaks.size();
        // std::cout << std::format("\n{} peaks counted for trailhead {}\n", peaks.size(),
        // trailhead);
    }
    return score;
}
}

namespace stage_2 {
int64_t map_height {};
int64_t map_width {};

void dfs(const std::optional<vertex>& parent,
         const vertex& current,
         const mat& map,
         std::vector<vertex>& peaks) {
    // parent is needed, current vertex is need. Find adjacent vertices that satisfy
    // condition of not being equal to the parent and having a height + 1
    if(current.height == 9U) {
        peaks.push_back(current);
        return;
    }

    std::vector<vertex> adjacent_vertices;
    for(const std::array<int64_t, 2U> direction: ::directions) {
        const int64_t new_m = current.m + direction[0U];
        const int64_t new_n = current.n + direction[1U];
        vertex v {.m = new_m, .n = new_n};
        if(!::is_in_bounds(v, map_width, map_height) || ((map[new_m][new_n] - 1) != current.height)
           || (parent && (new_m == parent->m) && (new_n == parent->n)))
            continue;
        v.height = current.height + 1U;
        adjacent_vertices.push_back(v);
    }
    for(const vertex v: adjacent_vertices)
        dfs(current, v, map, peaks);
}

uint64_t solve(const mat& map) {
    map_height = map.size();
    map_width = map[0].size();
    std::vector<vertex> trailheads = ::find_height_points(map, 0U);
    // std::cout << std::format("Exploring {} trailheads\n", trailheads.size());
    uint64_t score {};
    for(const vertex& trailhead: trailheads) {
        std::vector<vertex> peaks;
        dfs(std::nullopt, trailhead, map, peaks);
        std::sort(peaks.begin(), peaks.end());
        // peaks.erase(std::unique(peaks.begin(), peaks.end()), peaks.end());
        score += peaks.size();
        // std::cout << std::format("\n{} peaks counted for trailhead {}\n", peaks.size(),
        // trailhead);
    }
    return score;
}
}

int main(int argc, char** argv) {
    if(argc < 2U) {
        std::cout << "Invalid number of arguments\n";
        return 1;
    }
    mat map = get_input(argv[1]);
    // print_map(map);
    try {
        std::chrono::time_point<std::chrono::high_resolution_clock> start =
            std::chrono::high_resolution_clock::now();
        uint64_t sol_stage_1 = stage_1::solve(map);
        const std::chrono::microseconds t_stage_1 =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);

        start = std::chrono::high_resolution_clock::now();

        uint64_t sol_stage_2 = stage_2::solve(map);
        const std::chrono::microseconds t_stage_2 =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);

        std::cout << std::format(
            "Stage 1 solution: {} took {} us\nStage 2 solution: {} took {} us\n",
            sol_stage_1,
            t_stage_1,
            sol_stage_2,
            t_stage_2);
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}

mat get_input(const fs::path& path) {
    const auto ws_path = fs::canonical(path);
    std::ifstream ifs {ws_path};
    if(!ifs.good())
        throw std::runtime_error("Unable to open file " + ws_path.string());
    std::string line;
    mat map;
    map.reserve(line.size());
    while(std::getline(ifs, line)) {
        std::vector<char> line_v;
        std::for_each(line.cbegin(), line.cend(), [&](const char c) {
            line_v.push_back(static_cast<uint32_t>(c - '0'));
        });
        map.emplace_back(line_v);
    }
    return map;
}
void print_map(const mat& map) {
    std::cout << "\n";
    for(const auto row: map) {
        for(const auto& c: row) {
            std::cout << static_cast<uint32_t>(c) << " ";
        }
        std::cout << "\n";
    }
}
