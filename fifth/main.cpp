#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <sys/types.h>

std::vector<std::pair<uint32_t, uint32_t>> read_orderings(const std::filesystem::path& path) {
    const auto& working_path = std::filesystem::canonical(path);

    std::ifstream ifs {working_path};
    if(!ifs.good())
        throw std::runtime_error("Unable to open file " + path.string());

    std::vector<std::pair<uint32_t, uint32_t>> orderings;

    std::string line;
    while(std::getline(ifs, line)) {
        const std::string::size_type pos = line.find("|");
        if(pos == std::string::npos)
            continue;

        const uint32_t x = std::stoul(line.substr(0U, pos));
        const uint32_t y = std::stoul(line.substr(pos + 1U, line.size() - 1U));
        orderings.emplace_back(std::pair<uint32_t, uint32_t> {x, y});
    }
    return orderings;
}

std::vector<std::vector<uint32_t>> read_updates(const std::filesystem::path& path) {
    const auto& working_path = std::filesystem::canonical(path);
    std::ifstream ifs {working_path};
    if(!ifs.good())
        throw std::runtime_error("Unable to open file " + path.string());

    std::vector<std::vector<uint32_t>> updates;

    std::string line;
    while(std::getline(ifs, line)) {
        if(line.find(",") == std::string::npos)
            continue;
        std::stringstream ss {line};
        std::vector<uint32_t> update;
        for(uint32_t i; ss >> i;) {
            update.emplace_back(i);
            if(ss.peek() == ',')
                ss.ignore();
        }
        updates.emplace_back(update);
    }
    return updates;
}

[[nodiscard]] bool
    is_update_ordered(const std::vector<uint32_t>& update,
                      const std::vector<std::pair<uint32_t, uint32_t>>& ordering_rules) {
    for(auto page = update.cbegin(); page != update.cend(); ++page) {
        std::vector<uint32_t> pages_before, pages_after;
        for(const std::pair<uint32_t, uint32_t>& rule: ordering_rules) {
            if(rule.first == *page)
                pages_after.emplace_back(rule.second);
            if(rule.second == *page)
                pages_before.emplace_back(rule.first);
        }
        for(const uint32_t after: pages_after) {
            if(std::find(update.cbegin(), page, after) != page)
                return false;
        }
        for(const uint32_t before: pages_before) {
            if(std::find(page, update.cend(), before) != update.cend())
                return false;
        }
    }
    return true;
}

void order_update(std::vector<uint32_t>& update,
                  const std::vector<std::pair<uint32_t, uint32_t>>& ordering_rules) {
    bool performed_swap = false;
    for(auto page = update.begin(); page != update.end(); ++page) {
        std::vector<uint32_t> pages_after;
        for(const std::pair<uint32_t, uint32_t>& rule: ordering_rules) {
            if(rule.first == *page)
                pages_after.emplace_back(rule.second);
        }
        for(const uint32_t after: pages_after) {
            if(auto it = std::find(update.begin(), page, after); it != page) {
                std::swap(*page, *it);
                performed_swap = true;
            }
        }
    }
    if(performed_swap)
        order_update(update, ordering_rules);
}

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "invalid number of arguments\n";
        return 1;
    }
    const std::filesystem::path path {argv[1]};
    const auto& orderings = read_orderings(path);
    const auto& updates = read_updates(path);

    std::vector<std::vector<uint32_t>> unordered_updates;
    std::size_t ordered_updates {};
    uint64_t sum_middle_page_numbers {};
    std::for_each(updates.cbegin(), updates.cend(), [&](const std::vector<uint32_t>& update) {
        if(is_update_ordered(update, orderings)) {
            ++ordered_updates;
            sum_middle_page_numbers += update.at(update.size() / 2);
        } else {
            unordered_updates.push_back(update);
        }
    });
    std::cout << ordered_updates << "/" << updates.size() << " updates are ordered\n";
    std::cout << "Sum of middle page numbers: " << sum_middle_page_numbers << "\n";

    uint64_t sum_middle_page_numbers_reordered {};
    for(auto& unordered_update: unordered_updates) {
        order_update(unordered_update, orderings);
        sum_middle_page_numbers_reordered += unordered_update.at(unordered_update.size() / 2);
    }
    std::cout << "Sum of middle page numbers: " << sum_middle_page_numbers_reordered << "\n";
}
