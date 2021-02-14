set -e
gcc -fsanitize=undefined -Wpedantic -Wall -o server -Iinclude -Iclient server.c
echo "Server build success"