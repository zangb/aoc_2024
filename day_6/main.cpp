#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <sys/types.h>

enum class field { empty = 0, obstacle = 1, guard = 2, invalid = 3 };
using mat = std::vector<std::vector<field>>;

std::vector<field> process_line(const std::string& s) {
    std::vector<field> line;
    std::for_each(s.cbegin(), s.cend(), [&](const char c) {
        switch(c) {
        case '.':
            line.emplace_back(field::empty);
            break;
        case '#':
            line.emplace_back(field::obstacle);
            break;
        case '^':
            line.emplace_back(field::guard);
            break;
        default:
            break;
        }
    });
    return line;
}

mat get_map(const std::filesystem::path& path) {
    const std::filesystem::path ws_path = std::filesystem::canonical(path);
    std::ifstream ifs {ws_path};

    if(!ifs.good())
        throw std::runtime_error("Could not open file " + ws_path.string());

    mat map;
    std::string line;
    while(std::getline(ifs, line)) {
        map.emplace_back(process_line(line));
    }
    return map;
}

struct location {
    enum class direction : uint32_t { up = 0U, right = 1U, down = 2U, left = 3U };
    static constexpr std::array<std::array<int32_t, 2U>, 4U> direction_offset_ {
        {{-1, 0}, {0, +1}, {1, 0}, {0, -1}}};
    int32_t m;        //!< row
    int32_t n;        //!< column
    direction dir;    //!< direction faced
};
template<>
struct std::formatter<location::direction> : std::formatter<std::string> {
    auto format(const location::direction d, format_context& ctx) const {
        std::string s;
        switch(d) {
        case location::direction::up:
            s = "UP";
            break;
        case location::direction::right:
            s = "RIGHT";
            break;
        case location::direction::down:
            s = "DOWN";
            break;
        case location::direction::left:
            s = "LEFT";
            break;
        }
        return formatter<string>::format(std::format("{}", s), ctx);
    }
};
template<>
struct std::formatter<location> : std::formatter<std::string> {
    auto format(const location l, format_context& ctx) const {
        return formatter<string>::format(std::format("[{} {} {}]", l.m, l.n, l.dir), ctx);
    }
};

enum class states { standby, change_direction, move_forward };
template<>
struct std::formatter<states> : std::formatter<std::string> {
    auto format(const states d, format_context& ctx) const {
        std::string s;
        switch(d) {
        case ::states::move_forward:
            s = "MV_FWD";
            break;
        case ::states::change_direction:
            s = "CHANGE_DIR";
            break;
        case ::states::standby:
            s = "STANDBY";
            break;
        }
        return formatter<string>::format(std::format("{}", s), ctx);
    }
};

class guard {
  public:
    guard(const mat& map, const location& start_position) :
        map_(map), width_(map_[0].size()), height_(map.size()), pos_(start_position) {
        visited_locations_.push_back({start_position, 1U});
    }

    bool move() {
        switch(state_) {
        case states::standby: {
            const field next_field = get_next_field();
            if(next_field == field::invalid) {
                // std::cout << std::format("Invalid field for next position, current pos {}\n",
                // pos_);
                return false;
            }
            if(next_field == field::obstacle) {
                // std::cout << std::format("STATE CHANGE {}\n", state_, states::change_direction);
                state_ = states::change_direction;
            } else {
                // std::cout << std::format("STATE CHANGE {}\n", state_, states::move_forward);
                state_ = states::move_forward;
            }
            break;
        }
        case states::move_forward: {
            // std::cout << std::format("STATE CHANGE {}\n", state_, states::standby);
            state_ = states::standby;
            const location new_location {
                .m = pos_.m + location::direction_offset_[static_cast<uint32_t>(pos_.dir)][0],
                .n = pos_.n + location::direction_offset_[static_cast<uint32_t>(pos_.dir)][1],
                .dir = pos_.dir};
            if(was_already_visited(new_location)) {
                state_ = states::standby;
                return false;
            }
            if(is_in_bounds(new_location)) {
                // std::cout << std::format("{}\n", new_location);
                pos_ = new_location;
                update_locations_visisted();
                return true;
            }
            return false;
        }
        case states::change_direction: {
            // std::cout << std::format("STATE CHANGE {}\n", state_, states::standby);
            pos_.dir =
                static_cast<location::direction>((static_cast<uint32_t>(pos_.dir) + 1U) % 4U);
            state_ = states::standby;
            break;
        }
        }
        return true;
    }

    uint32_t num_locations_visited() const {
        return get_unique_locations().size();
    }

    [[nodiscard]] std::vector<location> visited_locations() const {
        return get_unique_locations();
    }

    [[nodiscard]] bool was_loop_detected() const {
        return loop_detected_;
    }

  private:
    std::vector<location> get_unique_locations() const {
        std::vector<location> unique_locations;
        unique_locations.reserve(visited_locations_.size());
        for(const auto& visited: visited_locations_) {
            if(std::find_if(unique_locations.begin(),
                            unique_locations.end(),
                            [&](const location& pos) {
                                return (pos.m == visited.first.m) && (pos.n == visited.first.n);
                            })
               == unique_locations.end())
                unique_locations.push_back(visited.first);
        }
        return unique_locations;
    }

    [[nodiscard]] bool was_already_visited(const location& l) {
        for(const auto& visited: visited_locations_) {
            if((visited.first.m == l.m) && (visited.first.n == l.n)
               && (visited.first.dir == l.dir)) {
                loop_detected_ = true;
                return true;
            }
        }
        return false;
    }

    void update_locations_visisted() {
        if(auto it = std::find_if(visited_locations_.begin(),
                                  visited_locations_.end(),
                                  [&](const std::pair<location, uint32_t>& visited) {
                                      return (visited.first.m == pos_.m)
                                             && (visited.first.n == pos_.n)
                                             && (visited.first.dir == pos_.dir);
                                  });
           it != visited_locations_.end()) {
            // if we've been at that location with a fitting direction, update the counter
            ++(it->second);
        } else {
            // if not add the location to the visited places
            visited_locations_.emplace_back(std::pair<location, uint32_t> {
                location {.m = pos_.m, .n = pos_.n, .dir = pos_.dir}, 1U});
        }
    }

    [[nodiscard]] bool is_in_bounds(const location& l) {
        return (l.m > -1) && (l.m < height_) && (l.n > -1) && (l.n < width_);
    }

    [[nodiscard]] field get_next_field() {
        const location l {
            .m = pos_.m + location::direction_offset_[static_cast<uint32_t>(pos_.dir)][0],
            .n = pos_.n + location::direction_offset_[static_cast<uint32_t>(pos_.dir)][1],
            .dir = pos_.dir};
        if(is_in_bounds(l))
            return map_[l.m][l.n];
        return field::invalid;
    }

    states state_ {states::standby};
    bool loop_detected_ = false;
    const mat& map_;
    const int32_t width_;
    const int32_t height_;
    location pos_;
    std::vector<std::pair<location, uint32_t>> visited_locations_;
};

std::optional<location> get_guard_position(const mat& map) {
    const std::size_t rows = map.size();
    const std::size_t cols = map[0].size();
    for(int32_t m = 0U; m < rows; ++m) {
        for(int32_t n = 0U; n < cols; ++n) {
            if(map[m][n] == field::guard)
                return location {.m = m, .n = n, .dir = location::direction::up};
        }
    }
    return std::nullopt;
}

int64_t stage_1(const mat& map) {
    std::optional<location> guard_position = get_guard_position(map);
    if(!guard_position) {
        std::cout << "No valid guard position found";
        return -1;
    }
    guard g {map, *guard_position};

    while(g.move()) {
    }
    return g.num_locations_visited();
}

int64_t stage_2(const mat& map) {
    std::optional<location> guard_position = get_guard_position(map);
    if(!guard_position) {
        std::cout << "No valid guard position found";
        return -1;
    }
    guard g {map, *guard_position};

    while(g.move()) {
    }

    const std::vector<location> visited_locations = g.visited_locations();

    mat mutable_map = map;
    uint64_t detected_loops {};
    const auto num_locations = g.num_locations_visited();
    uint64_t counter {};
    std::cout << "\n";
    for(const auto& visited_location: visited_locations) {
        std::cout << std::format("{}/{}, checking location [{:4} {:4}]\r",
                                 counter,
                                 num_locations,
                                 visited_location.m,
                                 visited_location.n);
        mutable_map[visited_location.m][visited_location.n] = field::obstacle;
        guard gd {mutable_map, *guard_position};
        while(gd.move()) {
        }
        if(gd.was_loop_detected())
            ++detected_loops;
        mutable_map[visited_location.m][visited_location.n] = field::empty;
        ++counter;
    }
    std::cout << "\n";
    return detected_loops;
}

int main(int argc, char** argv) {
    if(argc < 2U) {
        std::cout << "Invalid number of arguments\n";
        return 1;
    }

    const mat map = get_map(argv[1]);
    std::cout << "STAGE 1: " << stage_1(map) << " locations visited\n";
    std::cout << "STAGE 2: " << stage_2(map) << " loops detected\n";
}
