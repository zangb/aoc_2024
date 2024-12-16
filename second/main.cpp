#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <sys/types.h>

[[nodiscard]] std::vector<std::string> split(const std::string_view str,
                                             const std::string_view delimiter) {
    std::vector<std::string> tokens {};
    std::string::size_type pos_start {};
    for(std::string::size_type pos_end = 0U; pos_end != std::string::npos;) {
        pos_end = str.find(delimiter, pos_start);
        tokens.emplace_back(str.substr(pos_start, pos_end - pos_start));
        pos_start = pos_end + delimiter.size();
    }
    return tokens;
}

void read_input(const std::filesystem::path& path, std::vector<std::vector<uint32_t>>& reports) {
    std::ifstream ifs {std::filesystem::canonical(path)};
    if(!ifs.good())
        throw std::runtime_error("Unable to open file!");

    std::string line;
    while(std::getline(ifs, line)) {
        std::vector<uint32_t> report;
        std::vector<std::string> tokens = split(line, " ");
        std::for_each(tokens.begin(), tokens.end(), [&](const std::string& s) {
            try {
                report.emplace_back(std::stoul(s));
            } catch(const std::exception& e) {
                std::cout << e.what() << " substring " << s << std::endl;
            }
        });
        if(!report.empty())
            reports.emplace_back(report);
    }
}

[[nodiscard]] bool is_safe_report(const std::vector<uint32_t>& report) {
    if(report.size() < 2)
        return true;

    if(!std::is_sorted(report.cbegin(), report.cend())
       && !std::is_sorted(report.cbegin(), report.cend(), std::greater<> {})) {
        return false;
    }

    for(auto it = report.cbegin(); std::next(it) != report.cend(); ++it) {
        const auto distance =
            std::abs(static_cast<int32_t>(*it) - static_cast<int32_t>(*std::next(it)));
        if((distance < 1) || (distance > 3)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool can_be_safe_report(const std::vector<uint32_t>& report) {
    const std::size_t sz = report.size();
    for(std::size_t i = 0; i < sz; ++i) {
        std::vector<uint32_t> cp {report};
        cp.erase(cp.begin() + i);
        if(is_safe_report(cp))
            return true;
    }
    return false;
}

int main(int argc, char** argv) {
    if(argc < 2U) {
        std::cout << "invalid number of arguments\n";
        return 1;
    }
    std::vector<std::vector<uint32_t>> reports;
    read_input(argv[1], reports);
    std::vector<std::vector<uint32_t>> unsafe_reports;
    std::size_t safe_reports =
        std::count_if(reports.begin(), reports.end(), [&](const std::vector<uint32_t>& report) {
            if(!is_safe_report(report)) {
                unsafe_reports.emplace_back(report);
                return false;
            }
            return true;
        });

    std::size_t can_be_safe_reports = std::count_if(
        unsafe_reports.begin(), unsafe_reports.end(), [](const std::vector<uint32_t>& report) {
            return can_be_safe_report(report);
        });
    std::cout << safe_reports << " of " << reports.size() << " reports are safe" << std::endl;
    std::cout << can_be_safe_reports << " of " << reports.size() - safe_reports
              << " reports can be safe" << std::endl;
    std::cout << "Total number of safe reports: " << can_be_safe_reports + safe_reports
              << std::endl;
}
