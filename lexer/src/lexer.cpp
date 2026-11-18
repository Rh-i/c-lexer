#include "lexer/lexer.hpp"

#include <cctype>
#include <string>

#include "lexer/keyword_table.hpp"

namespace lexer {
namespace {

constexpr int kMaxIdentifierLength = 63;

bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

bool isDigit(char c) { return c >= '0' && c <= '9'; }

bool isHexDigit(char c) {
    return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool isIdentChar(char c) { return isAlpha(c) || isDigit(c) || c == '_'; }

bool isBlank(char c) { return c == ' ' || c == '\t' || c == '\r'; }

bool isNewline(char c) { return c == '\n' || c == '\r'; }

bool isSingleOperator(char c) {
    switch (c) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '=':
    case '<':
    case '>':
    case ';':
    case '(':
    case ')':
        return true;
    default:
        return false;
    }
}

bool isSupportedEscape(char c) {
    return c == 'n' || c == 't' || c == 'r' || c == '0' || c == '\\' || c == '\'' ||
           c == '"';
}

std::string toLower(std::string text) {
    for (char& c : text) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return text;
}

bool isValidSuffix(const std::string& suffix, bool isFloat) {
    const std::string s = toLower(suffix);
    if (isFloat) {
        return s == "f" || s == "l";
    }
    return s.empty() || s == "u" || s == "l" || s == "ul" || s == "lu" || s == "ll" ||
           s == "ull" || s == "llu";
}

}  // namespace

Lexer::Lexer(const std::string& source, std::string fileName)
    : source_(source), fileName_(std::move(fileName)) {}

char Lexer::at(std::size_t index) const {
    return index < source_.size() ? source_[index] : '\0';
}

char Lexer::advance() {
    const char c = source_[pos_++];
    if (c == '\n') {
        ++line_;
        col_ = 1;
    } else if (c == '\r' && at(pos_) != '\n') {
        // 单独的 CR 也当作一次换行；CRLF 留给后面的 LF 记一次
        ++line_;
        col_ = 1;
    } else {
        ++col_;
    }
    return c;
}

Token Lexer::makeToken(Kind kind, std::size_t start, int line, int col) {
    Token token;
    token.kind = kind;
    token.text = source_.substr(start, pos_ - start);
    token.raw = token.text;
    token.line = line;
    token.col = col;
    return token;
}

// 字符与字符串常量的属性值不含外层引号，便于阅读二元式
void Lexer::stripQuotes(Token& token) {
    if (token.raw.size() >= 2) {
        token.text = token.raw.substr(1, token.raw.size() - 2);
    }
}

void Lexer::skipBraceComment() {
    const int startLine = line_;
    const int startCol = col_;
    advance();  // '{'
    while (!eof() && source_[pos_] != '}') {
        advance();
    }
    if (eof()) {
        diagnostics_.add("E301", "花括号注释未闭合", startLine, startCol);
        return;
    }
    advance();  // '}'
}

void Lexer::skipLineComment() {
    advance();
    advance();
    while (!eof() && !isNewline(source_[pos_])) {
        advance();
    }
}

void Lexer::skipBlockComment() {
    const int startLine = line_;
    const int startCol = col_;
    advance();
    advance();
    while (!eof()) {
        if (source_[pos_] == '*' && at(pos_ + 1) == '/') {
            advance();
            advance();
            return;
        }
        advance();
    }
    diagnostics_.add("E301", "块注释未闭合", startLine, startCol);
}

void Lexer::skipBlankAndComment() {
    for (;;) {
        if (eof()) {
            return;
        }
        const char c = source_[pos_];
        if (isBlank(c) || c == '\n') {
            advance();
            continue;
        }
        if (c == '{') {
            skipBraceComment();
            continue;
        }
        if (c == '/' && at(pos_ + 1) == '/') {
            skipLineComment();
            continue;
        }
        if (c == '/' && at(pos_ + 1) == '*') {
            skipBlockComment();
            continue;
        }
        return;
    }
}

Token Lexer::scanIdentifier() {
    const int line = line_;
    const int col = col_;
    const std::size_t start = pos_;
    while (!eof() && isIdentChar(source_[pos_])) {
        advance();
    }
    Token token = makeToken(Kind::Identifier, start, line, col);
    if (static_cast<int>(token.text.size()) > kMaxIdentifierLength) {
        diagnostics_.add("E401", "标识符长度超过 63", line, col);
    }
    if (isKeyword(token.text)) {
        token.kind = Kind::Keyword;
    }
    return token;
}

Token Lexer::scanNumber() {
    const int line = line_;
    const int col = col_;
    const std::size_t start = pos_;
    int base = 10;
    bool isFloat = false;

    if (source_[pos_] == '0' && (at(pos_ + 1) == 'x' || at(pos_ + 1) == 'X')) {
        base = 16;
        advance();
        advance();
        if (!isHexDigit(at(pos_))) {
            diagnostics_.add("E201", "十六进制常量缺少数字", line, col);
        } else {
            while (!eof() && isHexDigit(source_[pos_])) {
                advance();
            }
        }
    } else if (source_[pos_] == '0' && isDigit(at(pos_ + 1))) {
        base = 8;
        advance();
        int badCol = 0;
        while (!eof() && isDigit(source_[pos_])) {
            if ((source_[pos_] == '8' || source_[pos_] == '9') && badCol == 0) {
                badCol = col_;
            }
            advance();
        }
        if (badCol != 0) {
            diagnostics_.add("E202", "八进制常量中出现 8 或 9", line, badCol);
        }
    } else {
        while (!eof() && isDigit(source_[pos_])) {
            advance();
        }
    }

    if (at(pos_) == '.') {
        isFloat = true;
        advance();
        while (!eof() && isDigit(source_[pos_])) {
            advance();
        }
    }

    if (at(pos_) == 'e' || at(pos_) == 'E') {
        isFloat = true;
        advance();
        if (at(pos_) == '+' || at(pos_) == '-') {
            advance();
        }
        if (!isDigit(at(pos_))) {
            diagnostics_.add("E203", "浮点常量缺少指数部分", line, col);
        } else {
            while (!eof() && isDigit(source_[pos_])) {
                advance();
            }
        }
    }

    const int suffixCol = col_;
    const std::size_t suffixStart = pos_;
    while (!eof() && isAlpha(source_[pos_])) {
        advance();
    }
    const std::string suffix = source_.substr(suffixStart, pos_ - suffixStart);

    Token token = makeToken(isFloat ? Kind::FloatConst : Kind::IntConst, start, line, col);
    token.number.base = base;
    token.number.isFloat = isFloat;
    token.number.suffix = suffix;

    if (!suffix.empty() && !isValidSuffix(suffix, isFloat)) {
        diagnostics_.add("E204", "非法的数值后缀：" + suffix, line, suffixCol);
    }
    return token;
}

Token Lexer::scanChar() {
    const int line = line_;
    const int col = col_;
    const std::size_t start = pos_;
    advance();  // 开引号

    int contentCount = 0;
    bool closed = false;
    bool escapeReported = false;

    while (!eof()) {
        const char c = source_[pos_];
        if (c == '\'') {
            advance();
            closed = true;
            break;
        }
        if (isNewline(c)) {
            break;
        }
        if (c == '\\') {
            advance();
            const char escaped = at(pos_);
            if (escaped == '\0') {
                break;
            }
            if (isSupportedEscape(escaped)) {
                advance();
            } else {
                if (!escapeReported) {
                    diagnostics_.add("E104", std::string("非法转义序列：\\") + escaped,
                                     line, col_);
                    escapeReported = true;
                }
                advance();
            }
            ++contentCount;
            continue;
        }
        if (static_cast<unsigned char>(c) >= 0x80) {
            diagnostics_.add("E101", "非法字符：非 ASCII 字节", line, col_);
            advance();
            ++contentCount;
            continue;
        }
        advance();
        ++contentCount;
    }

    if (!closed) {
        diagnostics_.add("E102", "字符常量未闭合", line, col);
        return makeToken(Kind::Error, start, line, col);
    }
    if (contentCount == 0) {
        diagnostics_.add("E103", "字符常量内容为空", line, col);
    } else if (contentCount > 1) {
        diagnostics_.add("E103", "字符常量只能包含一个字符", line, col);
    }
    Token token = makeToken(Kind::CharConst, start, line, col);
    stripQuotes(token);
    return token;
}

Token Lexer::scanString() {
    const int line = line_;
    const int col = col_;
    const std::size_t start = pos_;
    advance();  // 开引号

    bool closed = false;
    while (!eof()) {
        const char c = source_[pos_];
        if (c == '"') {
            advance();
            closed = true;
            break;
        }
        if (isNewline(c)) {
            break;
        }
        if (c == '\\') {
            advance();
            const char escaped = at(pos_);
            if (escaped == '\0') {
                break;
            }
            if (!isSupportedEscape(escaped)) {
                diagnostics_.add("E104", std::string("非法转义序列：\\") + escaped, line,
                                 col_);
            }
            advance();
            continue;
        }
        if (static_cast<unsigned char>(c) >= 0x80) {
            diagnostics_.add("E101", "非法字符：非 ASCII 字节", line, col_);
            advance();
            continue;
        }
        advance();
    }

    if (!closed) {
        diagnostics_.add("E105", "字符串常量未闭合", line, col);
        return makeToken(Kind::Error, start, line, col);
    }
    Token token = makeToken(Kind::StrConst, start, line, col);
    stripQuotes(token);
    return token;
}

Token Lexer::scanOperator() {
    static const char* const kTwoCharOperators[] = {"==", "!=", "<=", ">=", "&&", "||"};

    const int line = line_;
    const int col = col_;
    const std::size_t start = pos_;
    const char c = source_[pos_];
    const char next = at(pos_ + 1);

    for (const char* op : kTwoCharOperators) {
        if (c == op[0] && next == op[1]) {
            advance();
            advance();
            return makeToken(Kind::Operator, start, line, col);
        }
    }

    if (isSingleOperator(c)) {
        advance();
        return makeToken(Kind::Operator, start, line, col);
    }

    std::string message = "非法字符：";
    if (c == '!') {
        message += "! 只能作为 != 出现";
    } else if (c == '&') {
        message += "& 只能作为 && 出现";
    } else if (c == '|') {
        message += "| 只能作为 || 出现";
    } else if (static_cast<unsigned char>(c) < 0x20) {
        static const char* const kHex = "0123456789ABCDEF";
        const int value = static_cast<unsigned char>(c);
        message += "控制字符 0x";
        message += kHex[(value >> 4) & 0xF];
        message += kHex[value & 0xF];
    } else {
        message += c;
    }

    advance();
    diagnostics_.add("E101", message, line, col);
    return makeToken(Kind::Error, start, line, col);
}

Token Lexer::GetToken() {
    skipBlankAndComment();

    const int line = line_;
    const int col = col_;
    if (eof()) {
        Token token;
        token.kind = Kind::End;
        token.line = line;
        token.col = col;
        return token;
    }

    const char c = source_[pos_];
    if (isAlpha(c) || c == '_') {
        return scanIdentifier();
    }
    if (isDigit(c) || (c == '.' && isDigit(at(pos_ + 1)))) {
        return scanNumber();
    }
    if (c == '\'') {
        return scanChar();
    }
    if (c == '"') {
        return scanString();
    }
    if (static_cast<unsigned char>(c) >= 0x80) {
        const std::size_t start = pos_;
        advance();
        diagnostics_.add("E101", "非法字符：非 ASCII 字节", line, col);
        return makeToken(Kind::Error, start, line, col);
    }
    return scanOperator();
}

int Lexer::GetToken(std::string& attr) {
    const Token token = GetToken();
    attr = token.text;
    return static_cast<int>(token.kind);
}

}  // namespace lexer
