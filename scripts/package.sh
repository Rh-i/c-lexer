#!/usr/bin/env bash
# 生成分发包：静态 Release 构建、跑测试、铺 dist 目录、打包 zip、出校验和
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION="${VERSION:-1.0}"
OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
ARCH="$(uname -m)"
case "$OS" in
    linux) OS_NAME="linux" ;;
    darwin) OS_NAME="macos" ;;
    *) OS_NAME="$OS" ;;
esac

NAME="lexer-${VERSION}-${OS_NAME}-${ARCH}"
STAGE="$ROOT/dist/$NAME"

echo "构建 Release 静态产物..."
cmake -S "$ROOT/lexer" -B "$ROOT/lexer/build-static" -DCMAKE_BUILD_TYPE=Release -DLEXER_STATIC=ON > /dev/null
cmake --build "$ROOT/lexer/build-static" -j > /dev/null

echo "运行测试..."
(cd "$ROOT/lexer" && BIN=build-static/lexer ./tests/run_tests.sh > /dev/null)

echo "整理 $NAME ..."
rm -rf "$STAGE"
mkdir -p "$STAGE/bin" "$STAGE/include" "$STAGE/src" "$STAGE/tests/cases" "$STAGE/tests/expected" "$STAGE/examples" "$STAGE/docs"

strip -s "$ROOT/lexer/build-static/lexer" -o "$STAGE/bin/lexer-${OS_NAME}-${ARCH}"
chmod +x "$STAGE/bin/lexer-${OS_NAME}-${ARCH}"

cp "$ROOT/lexer/CMakeLists.txt" "$STAGE/"
cp "$ROOT/lexer/run.sh" "$ROOT/lexer/run.bat" "$STAGE/"
cp -r "$ROOT/lexer/include/." "$STAGE/include/"
cp -r "$ROOT/lexer/src/." "$STAGE/src/"
cp "$ROOT/lexer/tests/cases/"*.c "$STAGE/tests/cases/"
cp "$ROOT/lexer/tests/expected/"*.out "$STAGE/tests/expected/"
cp "$ROOT/lexer/tests/run_tests.sh" "$ROOT/lexer/tests/run_tests.bat" "$STAGE/tests/"
cp "$ROOT/lexer/examples/"*.c "$STAGE/examples/"
cp "$ROOT/docs/"*.md "$STAGE/docs/"
cp "$ROOT/README.md" "$STAGE/"

(cd "$ROOT/dist" && zip -qr "${NAME}.zip" "$NAME" && sha256sum "${NAME}.zip" > CHECKSUMS.txt)

echo "完成：dist/${NAME}.zip"
