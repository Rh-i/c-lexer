#!/usr/bin/env bash
# 逐用例比对期望输出，并把结果写入 docs/测试结果.md
set -u

cd "$(dirname "$0")/.." || exit 2
BIN="${BIN:-build/lexer}"

if [ ! -x "$BIN" ]; then
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release > /dev/null || exit 2
    cmake --build build -j > /dev/null || exit 2
    BIN="build/lexer"
fi

REPORT="../docs/测试结果.md"
mkdir -p "$(dirname "$REPORT")"
{
    echo "# 测试结果"
    echo
    echo "运行时间：$(date '+%Y-%m-%d %H:%M:%S')"
    echo
    echo "| 用例 | 结果 |"
    echo "| --- | --- |"
} > "$REPORT"

pass=0
fail=0
for f in tests/cases/*.c; do
    name="$(basename "$f" .c)"
    out="$(mktemp)"
    "$BIN" "$f" > "$out" 2>&1
    if diff -u "tests/expected/$name.out" "$out" > /dev/null; then
        pass=$((pass + 1))
        echo "PASS  $name"
        echo "| $name | PASS |" >> "$REPORT"
    else
        fail=$((fail + 1))
        echo "FAIL  $name"
        echo "| $name | FAIL |" >> "$REPORT"
        diff -u "tests/expected/$name.out" "$out" | head -40
    fi
    rm -f "$out"
done

total=$((pass + fail))
{
    echo
    echo "合计：$pass/$total 通过，$fail 失败。"
} >> "$REPORT"

echo "-----"
echo "合计：$pass/$total 通过，$fail 失败"
[ "$fail" -eq 0 ]
