#include "spdlog/spdlog.h"
#include "tools.hpp"

namespace tools {

std::string replace_all(std::string haystack, std::string_view needle, std::string_view replacement) {
    std::size_t pos = 0;
    auto needle_size = needle.size();
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        haystack.replace(pos, needle_size, replacement);
    }
    return haystack;
}

}
