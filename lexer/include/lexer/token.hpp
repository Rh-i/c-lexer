#pragma once

#include <string>

namespace lexer {

// 种别码，与报告表 1-3 一致
enum class Kind : int {
    Identifier = 1,  // 标识符
    IntConst   = 2,  // 整型常量
    FloatConst = 3,  // 浮点常量
    CharConst  = 4,  // 字符常量
    StrConst   = 5,  // 字符串常量
    Keyword    = 6,  // 保留字
    Operator   = 7,  // 运算符与界符
    End        = 8,  // 文件结束
    Error      = 9   // 词法错误
};

// 数字的三类属性：类型、进制、后缀
struct NumberAttr {
    int base = 10;      // 8 / 10 / 16
    bool isFloat = false;
    std::string suffix; // 原文中的后缀字母
};

struct Token {
    Kind kind = Kind::End;
    std::string text;   // 属性值：二元式中引号内的内容
    std::string raw;    // 记号原文
    int line = 1;
    int col = 1;
    NumberAttr number{};
};

const char* kindName(Kind kind);

// 二元式：(种别码,"属性值")
std::string formatToken(const Token& token);

}  // namespace lexer
