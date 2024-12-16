#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <sys/types.h>

[[nodiscard]] bool read_file_to_buf(std::filesystem::path path, std::string& s) {
    std::vector<char> buf;
    const auto canonical_path = std::filesystem::canonical(path);
    std::ifstream ifs {canonical_path};
    if(!ifs.good()) {
        std::cout << "Failed to open file " << path << std::endl;
        return false;
    }

    const uintmax_t file_sz = std::filesystem::file_size(canonical_path);
    buf.resize(file_sz);
    ifs.read(buf.data(), file_sz);
    s = std::string {buf.data(), buf.size()};
    return true;
}

std::vector<std::pair<uint32_t, uint32_t>> parse_pairs(std::string s) {
    const std::regex phrase {R"(mul\([0-9]+,[0-9]+\))"};
    const std::regex number {"[0-9]+"};
    std::vector<std::pair<uint32_t, uint32_t>> output;
    for(std::smatch smo; std::regex_search(s, smo, phrase);) {
        std::string tmp = smo.str();
        std::vector<uint32_t> nums;
        for(std::smatch smi; std::regex_search(tmp, smi, number);) {
            nums.emplace_back(std::stoul(smi.str()));
            tmp = smi.suffix();
        }
        if(nums.size() == 2U)
            output.emplace_back(std::pair<uint32_t, uint32_t> {nums[0], nums[1]});
        s = smo.suffix();
    }
    return output;
}

std::vector<std::string> find_valid_sections(std::string s) {
    std::vector<std::string> output;
    for(std::string::size_type pos_end = s.find("don't()"); pos_end != std::string::npos;
        pos_end = s.find("don't()")) {
        constexpr std::size_t sz_do = 4U;
        constexpr std::size_t sz_dont = 7U;
        std::string prefix {s.begin(), s.begin() + pos_end + sz_dont};
        output.emplace_back(prefix);
        std::string suffix = std::string {s.begin() + pos_end + sz_dont, s.end()};
        std::string::size_type pos = suffix.find("do()");

        if(pos == std::string::npos)
            break;
        s = std::string {suffix.begin() + pos, suffix.end()};
    }
    output.emplace_back(s);
    return output;
}

int main(int argc, char** argv) {
    if(argc < 2U) {
        std::cout << "invalid number of arguments\n";
        return 1;
    }
    const std::filesystem::path path {argv[1]};
    std::string data;
    if(!read_file_to_buf("input.txt", data))
        return 1;
    std::vector<std::pair<uint32_t, uint32_t>> pairs = parse_pairs(data);

    std::size_t stage_1_sum {};
    std::for_each(pairs.cbegin(), pairs.cend(), [&](const std::pair<uint32_t, uint32_t>& v) {
        stage_1_sum += v.first * v.second;
    });
    std::cout << "Stage 1 program output: " << stage_1_sum << std::endl;
    const std::vector<std::string> sections = find_valid_sections(data);

    std::vector<std::pair<uint32_t, uint32_t>> stage_2_pairs;
    std::for_each(sections.cbegin(), sections.cend(), [&](std::string s) {
        std::vector<std::pair<std::uint32_t, uint32_t>> parsed_pairs = parse_pairs(s);
        stage_2_pairs.insert(stage_2_pairs.end(), parsed_pairs.begin(), parsed_pairs.end());
    });

    std::size_t stage_2_sum {};
    std::for_each(
        stage_2_pairs.cbegin(), stage_2_pairs.cend(), [&](const std::pair<uint32_t, uint32_t>& v) {
            stage_2_sum += v.first * v.second;
        });
    std::cout << "Stage 2 program output: " << stage_2_sum << std::endl;

    return 0;
}
