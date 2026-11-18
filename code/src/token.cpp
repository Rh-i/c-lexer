#include "token.hpp"

namespace lexer {

const char* kindName(Kind kind) {
    switch (kind) {
    case Kind::Identifier: return "标识符";
    case Kind::IntConst: return "整型常量";
    case Kind::FloatConst: return "浮点常量";
    case Kind::CharConst: return "字符常量";
    case Kind::StrConst: return "字符串常量";
    case Kind::Keyword: return "保留字";
    case Kind::Operator: return "运算符与界符";
    case Kind::End: return "文件结束";
    case Kind::Error: return "词法错误";
    }
    return "未知";
}

std::string formatToken(const Token& token) {
    std::string out;
    out.reserve(token.text.size() + 8);
    out += '(';
    out += std::to_string(static_cast<int>(token.kind));
    out += ",\"";
    for (char c : token.text) {
        if (c == '"') {
            out += '\\';
        }
        out += c;
    }
    out += "\")";
    return out;
}

}  // namespace lexer
