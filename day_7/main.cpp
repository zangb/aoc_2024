#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <sys/types.h>

std::vector<std::vector<uint64_t>> get_input(const std::filesystem::path& path) {
    const std::filesystem::path ws_path = std::filesystem::canonical(path);
    std::ifstream ifs {ws_path};
    if(!ifs.good())
        throw std::runtime_error("Unable to open file " + ws_path.string());
    std::vector<std::vector<uint64_t>> input;
    std::string line;
    while(std::getline(ifs, line)) {
        line.erase(std::remove_if(line.begin(),
                                  line.end(),
                                  [&](const char c) {
                                      return c == ':';
                                  }),
                   line.end());
        std::stringstream ss {line};
        std::vector<uint64_t> values;
        uint64_t value;
        while(ss >> value) {
            values.push_back(value);
        }
        input.push_back(values);
    }
    return input;
}

class stage_2_solver {
  public:
    stage_2_solver() = default;

    uint64_t solve(const std::vector<std::vector<uint64_t>>& input) {
        const uint32_t max_permutation_sz = find_max_permutation_sz(input);
        std::vector<std::vector<std::vector<opcode>>> opcode_permutations;
        for(uint32_t perm_i = 1U; perm_i <= max_permutation_sz; ++perm_i)
            opcode_permutations.emplace_back(calculate_opcode_permutations(perm_i));

        uint64_t total_calibration_sum = {};

        for(const auto& line: input) {
            const uint64_t expected_result = line[0U];
            // -2 U because the result is stored as the first element of the vector and we only need
            // n - 1 inputs when we get n numbers as input
            const std::size_t permutation_length = line.size() - 2U;
            // -1 because we're zero based when indexing
            const std::vector<std::vector<opcode>>& valid_opcode_permutations =
                opcode_permutations[permutation_length - 1U];

            for(const auto& opcode_permutation: valid_opcode_permutations) {
                uint64_t computation_result = line[1U];
                for(std::size_t opcode_i = 0U; opcode_i < opcode_permutation.size(); ++opcode_i) {
                    computation_result = compute(
                        computation_result, line[opcode_i + 2U], opcode_permutation[opcode_i]);
                }
                if(computation_result == expected_result) {
                    total_calibration_sum += computation_result;
                    break;
                }
            }
        }
        return total_calibration_sum;
    }

  private:
    enum class opcode { add, mul, cat };
    static constexpr std::array<opcode, 3U> opcode_set {{opcode::add, opcode::mul, opcode::cat}};

    std::string opcode_to_str(const opcode op) {
        if(opcode::cat == op)
            return "cat";
        if(opcode::add == op)
            return "add";
        return "mul";
    }

    // https://www.geeksforgeeks.org/print-all-the-permutation-of-length-l-using-the-elements-of-an-array-iterative/
    std::vector<std::vector<opcode>> calculate_opcode_permutations(const std::size_t sz) {
        const uint64_t possible_permutations = std::pow<uint64_t>(opcode_set.size(), sz);
        std::vector<std::vector<opcode>> permutations;

        for(uint64_t perm_i = 0U; perm_i < possible_permutations; ++perm_i) {
            std::vector<opcode> permutation;
            uint64_t n = perm_i;
            for(std::size_t opcode_i = 0U; opcode_i < sz; ++opcode_i) {
                const opcode op = opcode_set[n % opcode_set.size()];
                n /= opcode_set.size();
                permutation.emplace_back(op);
            }
            permutations.emplace_back(permutation);
        }
        return permutations;
    }

    uint64_t find_max_permutation_sz(const std::vector<std::vector<uint64_t>>& input) {
        std::vector<std::size_t> v;
        v.reserve(input.size());
        // substract 1 from the length of a line because the result of the operations
        // is stored as the first element in the vector
        std::for_each(input.cbegin(), input.cend(), [&](const std::vector<uint64_t>& i) {
            v.push_back(i.size() - 1U);
        });
        if(const auto it = std::max_element(v.cbegin(), v.cend()); it != v.cend())
            return *it - 1;    // -1 because for n input values we only need n - 1 opcodes
        return 0U;
    }

    uint64_t digit_count(uint64_t x) {
        uint64_t digits {1U};
        while(x /= 10)
            ++digits;
        return digits;
    }

    uint64_t compute(const uint64_t x, const uint64_t y, const opcode op) {
        uint64_t result = {};
        switch(op) {
        case opcode::add:
            result = x + y;
            break;
        case opcode::mul:
            result = x * y;
            break;
        case opcode::cat:
            result = x * std::pow<uint64_t>(10U, digit_count(y)) + y;
        }
        return result;
    };
};

class stage_1_solver {
  public:
    stage_1_solver() = default;

    uint64_t solve(const std::vector<std::vector<uint64_t>>& input) {
        const uint32_t max_permutation_sz = find_max_permutation_sz(input);
        std::vector<std::vector<std::vector<opcode>>> opcode_permutations;
        // for each input length, pre calculate the possible opcode permutations
        for(uint32_t perm_i = 1U; perm_i <= max_permutation_sz; ++perm_i)
            opcode_permutations.emplace_back(calculate_opcode_permutations(perm_i));

        uint64_t total_calibration_sum = {};

        for(const auto& line: input) {
            const uint64_t expected_result = line[0U];
            // -2 U because the result is stored as the first element of the vector and we only
            // need n - 1 inputs when we get n numbers as input
            const std::size_t permutation_length = line.size() - 2U;
            // -1 because we're zero based when indexing
            const std::vector<std::vector<opcode>>& valid_opcode_permutations =
                opcode_permutations[permutation_length - 1U];

            for(const auto& opcode_permutation: valid_opcode_permutations) {
                uint64_t computation_result = line[1U];
                for(std::size_t opcode_i = 0U; opcode_i < opcode_permutation.size(); ++opcode_i) {
                    computation_result = compute(
                        computation_result, line[opcode_i + 2U], opcode_permutation[opcode_i]);
                }
                if(computation_result == expected_result) {
                    total_calibration_sum += computation_result;
                    break;
                }
            }
        }
        return total_calibration_sum;
    }

  private:
    enum class opcode { add, mul };
    static constexpr std::array<opcode, 2U> opcode_set {{opcode::add, opcode::mul}};

    std::string opcode_to_str(opcode op) {
        if(op == opcode::add)
            return "add";
        else
            return "mul";
    }
    /**
     * @brief
     *
     * @param sz length of the array holding the opcodes, e.g. {add, mul, mul} -> 3U
     * @return
     */
    std::vector<std::vector<opcode>> calculate_opcode_permutations(const std::size_t sz) {
        const auto possible_permutations = std::pow<uint64_t>(2U, sz);
        std::vector<std::vector<opcode>> permutations;

        for(std::size_t perm_i = 0U; perm_i < possible_permutations; ++perm_i) {
            std::vector<opcode> permutation;
            for(std::size_t bit = 0U; bit < sz; ++bit) {
                // opcodes are binary, so each permutation number perm_i can be interpreted
                // as a permutation of opcodes, e.g. 3 => 011 => add, mul, mul
                permutation.push_back(opcode_set[(perm_i >> bit) & 0x1]);
            }
            permutations.emplace_back(permutation);
        }

        return permutations;
    }

    uint64_t find_max_permutation_sz(const std::vector<std::vector<uint64_t>>& input) {
        std::vector<std::size_t> v;
        v.reserve(input.size());
        // substract 1 from the length of a line because the result of the operations
        // is stored as the first element in the vector
        std::for_each(input.cbegin(), input.cend(), [&](const std::vector<uint64_t>& i) {
            v.push_back(i.size() - 1U);
        });
        if(const auto it = std::max_element(v.cbegin(), v.cend()); it != v.cend())
            return *it - 1;    // -1 because for n input values we only need n - 1 opcodes
        return 0U;
    }

    uint64_t compute(const uint64_t x, const uint64_t y, const opcode op) {
        uint64_t result = {};
        switch(op) {
        case opcode::add:
            result = x + y;
            break;
        case opcode::mul:
            result = x * y;
        }
        return result;
    }
};

int main(int argc, char** argv) {
    if(argc < 2U) {
        std::cout << "Invalid number of arguments\n";
        return 1;
    }
    std::vector<std::vector<uint64_t>> input = get_input(argv[1]);
    std::chrono::time_point<std::chrono::high_resolution_clock> start =
        std::chrono::high_resolution_clock::now();
    stage_1_solver solver_1 {};
    const auto stage_1_result = solver_1.solve(input);
    const auto t_stage_1 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start);

    start = std::chrono::high_resolution_clock::now();
    stage_2_solver solver_2 {};
    const auto stage_2_result = solver_2.solve(input);
    const auto t_stage_2 = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start);
    std::cout << std::format(
        "Stage 1: Total calibration result: {} took {} us\n", stage_1_result, t_stage_1);
    std::cout << std::format(
        "Stage 2: Total calibration result: {} took {} us\n", stage_2_result, t_stage_2);
}
