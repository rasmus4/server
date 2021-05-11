set -e
gcc -fsanitize=undefined -fsanitize=address -g -Wpedantic -Wall -Wextra -Wno-unused-parameter -o chess -Iinclude -Iclient chess.c
gcc -static -O2 -DNDEBUG -s -Wpedantic -Wall -Wextra -Wno-unused-parameter -o release_chess -Iinclude -Iclient chess.c
echo "Server build success"