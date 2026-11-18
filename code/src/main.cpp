#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

#include "lexer.hpp"
#include "token.hpp"

namespace {

struct Stats {
    int identifier = 0;
    int intConst = 0;
    int floatConst = 0;
    int charConst = 0;
    int strConst = 0;
    int keyword = 0;
    int op = 0;
    int error = 0;
    int base8 = 0;
    int base10 = 0;
    int base16 = 0;
};

bool isRegularFile(const std::string& path) {
    std::error_code ec;
    const std::filesystem::path target = std::filesystem::u8path(path);
    return std::filesystem::is_regular_file(target, ec) && !ec;
}

std::string readAll(const std::string& path, bool& ok) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        ok = false;
        return {};
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    ok = !in.bad();
    return buffer.str();
}

std::vector<std::string> splitLines(const std::string& source) {
    std::vector<std::string> lines;
    std::string current;
    for (std::size_t i = 0; i < source.size(); ++i) {
        const char c = source[i];
        if (c == '\r') {
            if (i + 1 < source.size() && source[i + 1] == '\n') {
                ++i;
            }
            lines.push_back(current);
            current.clear();
        } else if (c == '\n') {
            lines.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        lines.push_back(current);
    }
    return lines;
}

void printUsage(const std::string& program) {
    std::printf("用法: %s <源文件.c>\n", program.c_str());
    std::printf("      %s -h\n\n", program.c_str());
    std::printf("输出: 带行号的源程序、逐行二元式、错误定位、统计\n");
}

int run(const std::vector<std::string>& args) {
    const std::string program = args.empty() ? "lexer" : args[0];

    if (args.size() == 2 && (args[1] == "-h" || args[1] == "--help")) {
        printUsage(program);
        return 0;
    }
    if (args.size() != 2) {
        printUsage(program);
        return 2;
    }

    const std::string& path = args[1];
    if (!isRegularFile(path)) {
        std::fprintf(stderr, "不是可读取的文件: %s\n", path.c_str());
        return 2;
    }

    bool ok = false;
    const std::string source = readAll(path, ok);
    if (!ok) {
        std::fprintf(stderr, "无法读取文件: %s\n", path.c_str());
        return 2;
    }

    lexer::Lexer lexer(source, path);
    std::vector<lexer::Token> tokens;
    for (;;) {
        lexer::Token token = lexer.GetToken();
        if (token.kind == lexer::Kind::End) {
            break;
        }
        tokens.push_back(token);
    }

    const std::vector<std::string> lines = splitLines(source);
    const int width = static_cast<int>(
        std::max<std::size_t>(3, std::to_string(lines.size()).size()));
    const std::string tokenPrefix(static_cast<std::size_t>(width) + 1, ' ');
    std::printf("=== %s ===\n", path.c_str());

    std::size_t index = 0;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const int lineNumber = static_cast<int>(i + 1);
        std::printf("%*d | %s\n", width, lineNumber, lines[i].c_str());

        std::string row;
        while (index < tokens.size() && tokens[index].line == lineNumber) {
            row += lexer::formatToken(tokens[index]);
            row += ' ';
            ++index;
        }
        if (!row.empty()) {
            row.pop_back();
            std::printf("%s| %s\n", tokenPrefix.c_str(), row.c_str());
        }
    }
    while (index < tokens.size()) {
        std::printf("%s| %s\n", tokenPrefix.c_str(),
                    lexer::formatToken(tokens[index]).c_str());
        ++index;
    }

    if (!lexer.diagnostics().empty()) {
        std::printf("\n--- 诊断 (%zu) ---\n", lexer.diagnostics().size());
        for (const lexer::Diagnostic& diag : lexer.diagnostics().items()) {
            std::string text;
            if (diag.line >= 1 && static_cast<std::size_t>(diag.line) <= lines.size()) {
                text = lines[static_cast<std::size_t>(diag.line - 1)];
            }
            std::printf("%s", lexer::renderDiagnostic(diag, text, path).c_str());
        }
    }

    Stats stats;
    for (const lexer::Token& token : tokens) {
        switch (token.kind) {
        case lexer::Kind::Identifier: ++stats.identifier; break;
        case lexer::Kind::IntConst:
            ++stats.intConst;
            if (token.number.base == 8) {
                ++stats.base8;
            } else if (token.number.base == 16) {
                ++stats.base16;
            } else {
                ++stats.base10;
            }
            break;
        case lexer::Kind::FloatConst: ++stats.floatConst; break;
        case lexer::Kind::CharConst: ++stats.charConst; break;
        case lexer::Kind::StrConst: ++stats.strConst; break;
        case lexer::Kind::Keyword: ++stats.keyword; break;
        case lexer::Kind::Operator: ++stats.op; break;
        case lexer::Kind::Error: ++stats.error; break;
        case lexer::Kind::End: break;
        }
    }

    std::printf("\n--- 统计 ---\n");
    std::printf("行数: %zu  记号: %zu  错误: %zu\n", lines.size(), tokens.size(),
                lexer.diagnostics().size());
    std::printf("标识符 %d  整型常量 %d  浮点常量 %d  字符常量 %d  字符串常量 %d  保留字 %d  运算符与界符 %d\n",
                stats.identifier, stats.intConst, stats.floatConst, stats.charConst,
                stats.strConst, stats.keyword, stats.op);
    std::printf("整型常量进制: 十进制 %d  八进制 %d  十六进制 %d\n", stats.base10, stats.base8,
                stats.base16);

    return lexer.diagnostics().empty() ? 0 : 1;
}

}  // namespace

#ifdef _WIN32
namespace {

std::string toUtf8(const wchar_t* text) {
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 1) {
        return {};
    }
    std::string out(static_cast<std::size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), size, nullptr, nullptr);
    out.pop_back();
    return out;
}

}  // namespace

int wmain(int argc, wchar_t** argv) {
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_BINARY);

    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        args.push_back(toUtf8(argv[i]));
    }
    return run(args);
}
#else
int main(int argc, char** argv) {
    std::vector<std::string> args(argv, argv + argc);
    return run(args);
}
#endif
