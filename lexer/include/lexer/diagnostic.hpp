#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace lexer {

struct Diagnostic {
    std::string code;    // 如 E101
    std::string message; // 中文说明
    int line = 1;
    int col = 1;
};

class DiagnosticBag {
public:
    void add(std::string code, std::string message, int line, int col);
    bool empty() const { return items_.empty(); }
    std::size_t size() const { return items_.size(); }
    const std::vector<Diagnostic>& items() const { return items_; }

private:
    std::vector<Diagnostic> items_;
};

// 渲染成：文件名:行:列: 码 说明 + 源码行 + ^ 指示
std::string renderDiagnostic(const Diagnostic& diag,
                             const std::string& sourceLine,
                             const std::string& fileName);

}  // namespace lexer
