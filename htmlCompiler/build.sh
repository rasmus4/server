set -e
gcc -fsanitize=undefined -fsanitize=address -g -Wall -Wpedantic -Wextra -o htmlCompiler htmlCompiler.c