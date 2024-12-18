#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

[[nodiscard]] bool read_input(const std::filesystem::path& path,
                              std::vector<int32_t>& left,
                              std::vector<int32_t>& right) {
    try {
        std::ifstream ifs {path};
        if(!ifs.good())
            return false;
        std::string input;
        while(std::getline(ifs, input)) {
            std::string::size_type pos = input.find_first_of(' ');
            if(pos == std::string::npos)
                break;
            const std::string col_l = input.substr(0, pos);
            const std::string col_r = input.substr(pos, input.size());
            left.emplace_back(std::stoul(col_l));
            right.emplace_back(std::stoul(col_r));
        }
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
        return false;
    }
    return true;
}

std::size_t compute_similarity(const std::vector<int32_t>& left,
                               const std::vector<int32_t>& right) {
    std::unordered_map<int32_t, std::size_t> map;
    for(const auto v: left) {
        const std::size_t count = std::count(right.cbegin(), right.cend(), v);
        map.insert({{v, count}});
    }

    std::size_t similarity_score {};
    for(const auto v: left) {
        const std::unordered_map<int32_t, std::size_t>::iterator it = map.find(v);
        if(it == map.end())
            throw std::runtime_error("Key not found in map!");

        similarity_score += v * it->second;
    }
    return similarity_score;
}

std::size_t compute_total_distance(std::vector<int32_t>& left, std::vector<int32_t>& right) {
    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());

    std::size_t sz = left.size();
    std::size_t distance {};
    for(std::size_t i = 0U; i < sz; ++i)
        distance += std::abs(left[i] - right[i]);
    return distance;
}

int main(int argc, char** argv) {
    if(argc < 2U) {
        std::cout << "invalid number of arguments\n";
        return 1;
    }
    std::vector<int32_t> left, right;

    if(!read_input(argv[1], left, right))
        std::cout << "Failed input\n";

    if(left.size() != right.size())
        std::cout << "Unequal list size, aborting" << std::endl;

    const std::size_t total_distance = compute_total_distance(left, right);
    std::cout << "Total distance between lists is " << total_distance << std::endl;

    const std::size_t similarity = compute_similarity(left, right);
    std::cout << "Similarity score is " << similarity << std::endl;
}
