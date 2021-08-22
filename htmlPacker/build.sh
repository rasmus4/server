set -e
gcc -fsanitize=undefined -fsanitize=address -g -Wall -Wpedantic -Wextra -Wconversion -o htmlPacker htmlPacker.c