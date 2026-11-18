#pragma once

#include <string>

#include "diagnostic.hpp"
#include "token.hpp"

namespace lexer {

// 指导书要求：GetToken() 每次分析出一个记号，返回值给出种类，
// 属性通过变量带回。两个重载都保留。
class Lexer {
public:
    Lexer(const std::string& source, std::string fileName);

    Token GetToken();
    int GetToken(std::string& attr);

    const DiagnosticBag& diagnostics() const { return diagnostics_; }
    int lineCount() const { return line_; }

private:
    char at(std::size_t index) const;
    bool eof() const { return pos_ >= source_.size(); }
    char advance();

    void skipBlankAndComment();
    void skipBraceComment();
    void skipLineComment();
    void skipBlockComment();

    Token scanIdentifier();
    Token scanNumber();
    Token scanChar();
    Token scanString();
    Token scanOperator();

    Token makeToken(Kind kind, std::size_t start, int line, int col);
    void stripQuotes(Token& token);

    const std::string& source_;
    std::string fileName_;
    std::size_t pos_ = 0;
    int line_ = 1;
    int col_ = 1;
    DiagnosticBag diagnostics_;
};

}  // namespace lexer
