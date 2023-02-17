#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>

namespace tools {

[[nodiscard]]
std::string replace_all(std::string haystack, std::string_view needle, std::string_view replacement);

}

#endif // TOOLS_HPP