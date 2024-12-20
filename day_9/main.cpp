#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;
std::vector<char> get_input(const fs::path& path);

void print_vec(const std::vector<uint64_t>& vec, std::optional<fs::path> path) {
    if(path) {
        std::ofstream ofs {fs::canonical(*path)};
        std::for_each(vec.begin(), vec.end(), [&](const uint64_t i) {
            ofs << i << "\n";
        });
    }
}
std::vector<uint64_t> decompress_disk_map(const std::vector<char>& disk_map) {
    uint64_t id {};
    std::vector<uint64_t> decompressed {};
    const std::size_t disk_map_sz = disk_map.size();
    for(std::size_t i = 0U; i < disk_map_sz; ++i) {
        std::uint64_t push_back_v {};
        if(i & 0x1) {
            push_back_v = std::numeric_limits<uint64_t>::max();
        } else {
            push_back_v = id;
            ++id;
        }
        for(std::size_t j = 0U; j < disk_map.at(i); ++j) {
            decompressed.push_back(push_back_v);
        }
    }
    return decompressed;
}

namespace stage_1 {

uint64_t solve(const std::vector<char>& disk_map) {
    if(disk_map.empty())
        throw std::runtime_error("Zero length isn't accepted!");
    const std::size_t disk_map_sz = disk_map.size();
    const uint64_t drive_sz = std::accumulate(disk_map.cbegin(), disk_map.cend(), 0ULL);

    const uint64_t max_file_id =
        disk_map.size() & 0x1F ? ((disk_map.size()) / 2U) : (disk_map.size() / 2U) - 1;
    std::cout << std::format("Disk map size: {}, Drive Size: {}, Maximum file id: {}\n",
                             disk_map.size(),
                             drive_sz,
                             max_file_id);

    std::vector<uint64_t> decompressed = ::decompress_disk_map(disk_map);
    const std::size_t decompressed_sz = decompressed.size();

    // print_vec(decompressed, "pre-sorted.txt");

    std::size_t start_index {};    // prevents search from the very beginning at every loop
    for(std::size_t i = decompressed_sz - 1U; i >= 0U; --i) {
        if(i < start_index) {
            break;
        }
        if(decompressed.at(i) != std::numeric_limits<uint64_t>::max()) {
            for(std::size_t j = start_index; j < i; ++j) {
                if(decompressed.at(j) == std::numeric_limits<uint64_t>::max()) {
                    std::swap(decompressed[i], decompressed[j]);
                    start_index = j;
                    break;
                }
            }
        }
    }

    uint64_t checksum {};
    // print_vec(decompressed, "post-sorted.txt");
    for(std::size_t i = 0U; i < decompressed_sz; ++i) {
        if(decompressed[i] == std::numeric_limits<uint64_t>::max())
            break;
        checksum += decompressed[i] * i;
    }
    return checksum;
}
}
namespace stage_2 {
enum class block_t : uint32_t { file, free_mem };
struct file_entry {
    int64_t id;
    std::size_t pos;
    std::size_t size;
};
using file_table = std::vector<file_entry>;
}

template<>
struct ::std::formatter<stage_2::file_entry> : ::std::formatter<std::string> {
    auto format(stage_2::file_entry f, format_context& ctx) const {
        return formatter<string>::format(
            std::format("ID: {} Pos: {} Size: {}", f.id, f.pos, f.size), ctx);
    }
};

namespace stage_2 {
file_table generate_file_table(const std::vector<char>& disk_map) {
    file_table table;
    table.reserve(disk_map.size());
    const std::size_t disk_map_sz = disk_map.size();
    std::size_t position {};
    int64_t id {};
    for(std::size_t i = 0U; i < disk_map_sz; ++i) {
        file_entry entry {.id = (i & 0x1U) ? -1 : id++,
                          .pos = position,
                          .size = static_cast<std::size_t>(disk_map[i])};
        position += entry.size;
        table.push_back(entry);
    }
    return table;
}

uint64_t solve(const std::vector<char>& disk_map) {
    if(disk_map.empty())
        throw std::runtime_error("Zero length isn't accepted!");
    std::vector<uint64_t> decompressed = ::decompress_disk_map(disk_map);
    file_table f_table = generate_file_table(disk_map);
    if(f_table.size() != disk_map.size())
        throw std::runtime_error("Invalid file table generated!");

    const std::size_t file_table_sz = f_table.size();
    // print_vec(decompressed, "pre-sorted.txt");
    for(int64_t i = file_table_sz - 1; i >= 0U; --i) {
        if(f_table.at(i).id < 0U)
            continue;
        auto it = std::find_if(f_table.begin(), f_table.begin() + i, [&](const file_entry& fe) {
            return (fe.id < 0U) && (fe.size >= f_table.at(i).size);
        });
        if(it != (f_table.begin() + i)) {
            /*std::cout << "Copying range [";
            std::for_each(decompressed.begin() + f_table.at(i).pos,
                          decompressed.begin() + f_table.at(i).pos + f_table.at(i).size,
                          [&](const auto& e) {
                              std::cout << e << ", ";
                          });
            std::cout << "] from position " << f_table.at(i).pos << " to " << it->pos << "\n";
            */
            std::copy(decompressed.begin() + f_table.at(i).pos,
                      decompressed.begin() + f_table.at(i).pos + f_table.at(i).size,
                      decompressed.begin() + it->pos);
            // update source file entry from where we copied
            // move block data at end
            std::fill_n(decompressed.begin() + f_table.at(i).pos,
                        f_table.at(i).size,
                        std::numeric_limits<uint64_t>::max());

            // new slot of empty memory that has to be added to the file table
            file_entry fe {.id = -1,
                           .pos = it->pos + f_table.at(i).size,
                           .size = it->size - f_table.at(i).size};
            // update destiantion file entry
            const std::size_t block_sz = it->size;
            it->id = f_table.at(i).id;
            it->size = f_table.at(i).size;

            if(f_table.at(i).size < block_sz) {
                // std::cout << std::format("Inserting new slot {}\n", fe);
                f_table.insert(it + 1, fe);
            }

            f_table.at(i).id = -1;
        }
    }

    uint64_t checksum {};
    const std::size_t decompressed_sz = decompressed.size();
    print_vec(decompressed, "post-sorted.txt");
    for(std::size_t i = 0U; i < decompressed_sz; ++i) {
        if(decompressed[i] != std::numeric_limits<uint64_t>::max())
            checksum += decompressed[i] * i;
    }

    return checksum;
}
}

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "Invalid number of arguments\n";
        return 1;
    }
    std::vector<char> disk_map = get_input(argv[1]);
    try {
        std::chrono::time_point<std::chrono::high_resolution_clock> start =
            std::chrono::high_resolution_clock::now();
        uint64_t sol_stage_1 = stage_1::solve(disk_map);
        const std::chrono::microseconds t_stage_1 =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - start);

        start = std::chrono::high_resolution_clock::now();

        uint64_t sol_stage_2 = stage_2::solve(disk_map);
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
    return 0;
}

std::vector<char> get_input(const fs::path& path) {
    const auto ws_path = fs::canonical(path);
    std::ifstream ifs {ws_path};
    if(!ifs.good())
        throw std::runtime_error("Unable to open file " + ws_path.string());
    std::string line;
    std::getline(ifs, line);
    std::vector<char> disk_map;
    disk_map.reserve(line.size());
    std::for_each(line.cbegin(), line.cend(), [&](const char c) {
        disk_map.push_back(static_cast<uint32_t>(c - '0'));
    });
    return disk_map;
}
