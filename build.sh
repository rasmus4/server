set -e
gcc -fsanitize=undefined -g -Wpedantic -Wall -o server -Iinclude -Iclient server.c
echo "Server build success"