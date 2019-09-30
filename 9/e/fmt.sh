# !/bin/sh
find . -name "*.c" | xargs clang-format -i -style=webkit
