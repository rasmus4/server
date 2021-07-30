set -e

common_flags="-Wpedantic -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -Iinclude -I../../client/include -I../../common/include"
gcc -fsanitize=undefined -fsanitize=address -g -DUNREACHABLE='assert(0)' -o recursive $common_flags recursive.c
gcc -static -s -O2 -DNDEBUG -DUNREACHABLE='__builtin_unreachable()' -o release_recursive $common_flags recursive.c
echo "Build success"