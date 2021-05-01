set -e
gcc -fsanitize=undefined -g -Wpedantic -Wall -o chess -Iinclude -Iclient chess.c
echo "Server build success"