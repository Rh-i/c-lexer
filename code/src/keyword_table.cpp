#include "keyword_table.hpp"

#include <algorithm>
#include <array>

namespace lexer {
namespace {

// 指导书表内 unsigned 重复列了一次，去重后 21 个，按字典序排列便于二分查找
constexpr std::array<std::string_view, 21> kKeywords = {
    "auto",  "break",    "case",   "char",    "const",   "continue", "do",
    "double", "else",    "enum",   "float",   "if",      "int",      "long",
    "main",  "return",   "static", "switch",  "unsigned", "void",    "while",
};

}  // namespace

bool isKeyword(std::string_view word) {
    return std::binary_search(kKeywords.begin(), kKeywords.end(), word);
}

const std::string_view* keywordArray() { return kKeywords.data(); }

int keywordCount() { return static_cast<int>(kKeywords.size()); }

}  // namespace lexer
