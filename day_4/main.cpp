#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

std::vector<char> read_input(const std::filesystem::path& path) {
    const std::filesystem::path working_path = std::filesystem::canonical(path);
    std::ifstream ifs {working_path};

    if(!ifs.good())
        throw std::runtime_error("Could not open " + working_path.string());

    const uintmax_t file_sz = std::filesystem::file_size(working_path);

    // Create value with of length file_sz with default constructed elements
    // so we can use read (otherwise the vector wouldn't be aware of added elements)
    std::vector<char> buffer(file_sz);

    ifs.read(reinterpret_cast<char*>(buffer.data()), file_sz);

    return buffer;
}

std::size_t get_grid_width(const std::filesystem::path& path) {
    const std::filesystem::path working_path = std::filesystem::canonical(path);
    std::ifstream ifs {working_path};
    if(!ifs.good())
        throw std::runtime_error("Could not open " + working_path.string());

    std::string line;
    if(!std::getline(ifs, line))
        throw std::runtime_error("Couldn't get line from file " + working_path.string());
    return line.size();
}

class puzzle_solver {
  public:
    puzzle_solver(const std::vector<char>& c,
                  const std::string& word,
                  const std::size_t width,
                  const std::size_t height) :
        c_(c), width_(width), height_(height), word_(word) {
        if((width_ * height_) != c.size())
            throw std::runtime_error("Invalid combination");
    }

    std::size_t solve() {
        const auto cols = get_cols();
        const auto rows = get_rows();

        std::size_t row_count {};
        for(const auto& row: rows) {
            std::size_t words = scan_for_words(row);
            row_count += words;
            std::vector<char> reversed {row.rbegin(), row.rend()};
            words = scan_for_words(reversed);
            row_count += words;
        }
        std::cout << "Found " << row_count << " words in rows\n";
        std::size_t col_count {};
        for(const auto& col: cols) {
            std::size_t words = scan_for_words(col);
            col_count += words;
            std::vector<char> reversed {col.rbegin(), col.rend()};
            words = scan_for_words(reversed);
            col_count += words;
        }
        std::cout << "Found " << col_count << " words in rows\n";

        std::vector<std::vector<char>> diagonals = get_diagonals(rows);
        std::size_t diagonals_count {};
        for(const auto& diagonal: diagonals) {
            std::size_t words = scan_for_words(diagonal);
            diagonals_count += words;
            std::vector<char> reversed {diagonal.rbegin(), diagonal.rend()};
            words = scan_for_words(reversed);
            diagonals_count += words;
        }

        std::vector<std::vector<char>> mat_r;
        for(const auto& row: rows) {
            mat_r.push_back(std::vector<char> {row.rbegin(), row.rend()});
        }

        std::vector<std::vector<char>> diagonals_r = get_diagonals(mat_r);
        for(const auto& diagonal: diagonals_r) {
            std::size_t words = scan_for_words(diagonal);
            diagonals_count += words;
            std::vector<char> reversed {diagonal.rbegin(), diagonal.rend()};
            words = scan_for_words(reversed);
            diagonals_count += words;
        }

        std::cout << "Found " << diagonals_count << " words in diagonals\n";
        return col_count + row_count + diagonals_count;
    }

  private:
    [[nodiscard]] std::vector<std::vector<char>>
        get_diagonals(const std::vector<std::vector<char>>& mat) const {
        std::vector<std::vector<char>> diagonals;
        const std::size_t sz_diags = width_ + height_ - 1U;
        diagonals.reserve(sz_diags);

        for(std::size_t diagonal = 0U; diagonal < sz_diags; ++diagonal) {
            std::vector<char> diag;
            const std::size_t col_idex = std::min(diagonal, width_ - 1U);
            const std::size_t init_index = diagonal - col_idex;
            for(std::size_t m = init_index; m <= std::min(diagonal, height_ - 1U); ++m) {
                diag.push_back(mat.at(m).at(diagonal - m));
            }
            diagonals.emplace_back(diag);
        }
        return diagonals;
    }

    [[nodiscard]] std::vector<std::vector<char>> get_rows() const {
        std::vector<std::vector<char>> rows;
        for(std::size_t i = 0U; i < height_; ++i) {
            rows.emplace_back(
                std::vector<char> {c_.begin() + i * width_, c_.begin() + i * width_ + width_});
        }
        return rows;
    }

    [[nodiscard]] std::vector<std::vector<char>> get_cols() const {
        std::vector<std::vector<char>> cols;
        for(std::size_t i = 0U; i < width_; ++i) {
            std::vector<char> col;
            col.reserve(height_);
            for(std::size_t o = 0U; o < height_; ++o) {
                col.emplace_back(c_[o * width_ + i]);
            }
            cols.push_back(col);
        }
        return cols;
    }

    [[nodiscard]] std::size_t scan_for_words(const std::vector<char>& row) {
        std::size_t word_count {};
        for(std::size_t index = 0U; index < row.size(); ++index) {
            if(index > (row.size() - word_.size()))
                break;
            bool found_word = true;
            for(std::size_t i = 0U; i < word_.size(); ++i) {
                if(row[index + i] != word_[i]) {
                    found_word = false;
                    break;
                }
            }
            if(found_word)
                ++word_count;
        }
        return word_count;
    }

    [[nodiscard]] bool word_vertical_ahead(const std::size_t index) {
        return false;
    }

    const std::vector<char>& c_;
    const std::string word_;
    const std::size_t width_;
    const std::size_t height_;
};

class puzzle_solver_2 {
  public:
    puzzle_solver_2(const std::vector<char>& c, const std::size_t width, const std::size_t height) :
        c_(c), width_(width), height_(height) { }

    std::size_t solve() {
        std::size_t xmas_count {};
        const std::vector<std::vector<char>> mat = get_rows();

        for(std::size_t m = 1; m < (height_ - 1U); ++m) {
            for(std::size_t n = 1; n < (width_ - 1U); ++n) {
                if(mat.at(m).at(n) != 'A')
                    continue;
                const char top_left = mat.at(m - 1).at(n - 1);
                const char top_right = mat.at(m - 1).at(n + 1);
                const char bottom_left = mat.at(m + 1).at(n - 1);
                const char bottom_right = mat.at(m + 1).at(n + 1);

                if(((top_left == 'S') && (bottom_right == 'M')
                    || (top_left == 'M') && (bottom_right == 'S'))
                   && ((top_right == 'S' && (bottom_left == 'M')
                        || (top_right == 'M') && (bottom_left == 'S')))) {
                    ++xmas_count;
                }
            }
        }

        return xmas_count;
    }

  private:
    [[nodiscard]] std::vector<std::vector<char>> get_rows() const {
        std::vector<std::vector<char>> rows;
        for(std::size_t i = 0U; i < height_; ++i) {
            rows.emplace_back(
                std::vector<char> {c_.begin() + i * width_, c_.begin() + i * width_ + width_});
        }
        return rows;
    }
    const std::size_t width_;
    const std::size_t height_;
    const std::vector<char>& c_;
};

int main(int argc, char** argv) {
    if(argc < 2U) {
        std::cout << "invalid number of arguments\n";
        return 1;
    }
    try {
        const std::filesystem::path path {argv[1]};
        std::vector<char> data;
        const std::size_t grid_width = get_grid_width(path);
        data = read_input(path);
        const std::size_t grid_height = std::count(data.cbegin(), data.cend(), '\n');
        std::cout << "Grid width " << grid_width << " Grid Height " << grid_height << std::endl;
        for(std::vector<char>::iterator it = data.begin(); it != data.end();) {
            if(*it == '\n')
                it = data.erase(it);
            else
                ++it;
        }
        puzzle_solver solver {data, "XMAS", grid_width, grid_height};
        const auto stage_1_solution = solver.solve();
        std::cout << "Stage 1: Found " << stage_1_solution << " occurences \n";
        puzzle_solver_2 solver_2 {data, grid_width, grid_height};
        const auto stage_2_solution = solver_2.solve();
        std::cout << "Stage 2: Found " << stage_2_solution << " occurences \n";

    } catch(const std::exception& e) {
        std::cout << e.what() << "\n";
        return 1;
    }
}
