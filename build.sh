set -e
gcc -fsanitize=undefined -g -Wpedantic -Wall -o chess -Iinclude -Iclient chess.c
gcc -static -O2 -DNDEBUG -s -Wpedantic -Wall -o release_chess -Iinclude -Iclient chess.c
echo "Server build success"