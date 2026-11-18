#include "diagnostic.hpp"

#include <algorithm>
#include <sstream>

namespace lexer {

void DiagnosticBag::add(std::string code, std::string message, int line, int col) {
    Diagnostic diag;
    diag.code = std::move(code);
    diag.message = std::move(message);
    diag.line = line;
    diag.col = col;
    items_.push_back(std::move(diag));
}

std::string renderDiagnostic(const Diagnostic& diag,
                             const std::string& sourceLine,
                             const std::string& fileName) {
    std::ostringstream out;
    out << fileName << ':' << diag.line << ':' << diag.col << ": " << diag.code << ' '
        << diag.message << '\n';

    const std::string digits = std::to_string(diag.line);
    const std::size_t width = std::max<std::size_t>(3, digits.size());
    std::string number = digits;
    number.insert(0, width - digits.size(), ' ');
    out << number << " | " << sourceLine << '\n';

    out << std::string(width + 1, ' ') << "| ";
    for (int i = 1; i < diag.col; ++i) {
        const std::size_t index = static_cast<std::size_t>(i - 1);
        const char ch = index < sourceLine.size() ? sourceLine[index] : ' ';
        out << (ch == '\t' ? '\t' : ' ');
    }
    out << "^\n";
    return out.str();
}

}  // namespace lexer
