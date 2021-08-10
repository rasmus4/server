set -e

common_flags="-Wpedantic -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -I../recursive/include -I../common/include -I../client/include -I../../common/include"
gcc -fsanitize=undefined -fsanitize=address -g -DUNREACHABLE='assert(0)' -o perf $common_flags perf.c
gcc -static -s -O2 -DNDEBUG -DUNREACHABLE='__builtin_unreachable()' -o release_perf $common_flags perf.c
echo "Build success"