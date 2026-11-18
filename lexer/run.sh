#!/usr/bin/env bash
# 一键运行：优先使用 bin/ 中的预编译产物，缺失时退回 build/，再缺失则现场构建
set -u

SELF_DIR="$(cd "$(dirname "$0")" && pwd)"
BIN="$SELF_DIR/bin/lexer-linux-x86_64"

if [ ! -x "$BIN" ]; then
    BIN="$SELF_DIR/build/lexer"
fi

if [ ! -x "$BIN" ]; then
    echo "未找到可执行文件，正在构建..." >&2
    cmake -S "$SELF_DIR" -B "$SELF_DIR/build" -DCMAKE_BUILD_TYPE=Release >&2 || exit 2
    cmake --build "$SELF_DIR/build" -j >&2 || exit 2
    BIN="$SELF_DIR/build/lexer"
fi

exec "$BIN" "$@"
