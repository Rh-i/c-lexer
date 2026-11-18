#pragma once

#include <string_view>

namespace lexer {

// 保留字表：指导书表内 unsigned 列了两次，去重后 21 个
bool isKeyword(std::string_view word);

const std::string_view* keywordArray();
int keywordCount();

}  // namespace lexer
