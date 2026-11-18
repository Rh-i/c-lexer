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
cmake -S "$ROOT/code" -B "$ROOT/code/build-static" -DCMAKE_BUILD_TYPE=Release -DLEXER_STATIC=ON > /dev/null
cmake --build "$ROOT/code/build-static" -j > /dev/null

echo "运行测试..."
(cd "$ROOT/code" && BIN=build-static/lexer ./tests/run_tests.sh > /dev/null)

echo "整理 $NAME ..."
rm -rf "$STAGE"
mkdir -p "$STAGE/bin" "$STAGE/inc" "$STAGE/src" "$STAGE/tests/cases" "$STAGE/tests/expected" "$STAGE/examples" "$STAGE/docs"

strip -s "$ROOT/code/build-static/lexer" -o "$STAGE/bin/lexer-${OS_NAME}-${ARCH}"
chmod +x "$STAGE/bin/lexer-${OS_NAME}-${ARCH}"

cp "$ROOT/code/CMakeLists.txt" "$STAGE/"
cp "$ROOT/code/run.sh" "$ROOT/code/run.bat" "$STAGE/"
cp -r "$ROOT/code/inc/." "$STAGE/inc/"
cp -r "$ROOT/code/src/." "$STAGE/src/"
cp "$ROOT/code/tests/cases/"*.c "$STAGE/tests/cases/"
cp "$ROOT/code/tests/expected/"*.out "$STAGE/tests/expected/"
cp "$ROOT/code/tests/run_tests.sh" "$ROOT/code/tests/run_tests.bat" "$STAGE/tests/"
cp "$ROOT/code/examples/"*.c "$STAGE/examples/"
cp "$ROOT/docs/"*.md "$STAGE/docs/"
cp "$ROOT/README.md" "$STAGE/"

(cd "$ROOT/dist" && rm -f "${NAME}.zip" && zip -qr "${NAME}.zip" "$NAME" && sha256sum "${NAME}.zip" > CHECKSUMS.txt)

echo "完成：dist/${NAME}.zip"
