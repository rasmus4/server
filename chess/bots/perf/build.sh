set -e

common_flags="-Wpedantic -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -I../recursive/include -I../common/include -I../client/include -I../../common/include"
gcc -static -s -O2 -DNDEBUG -DUNREACHABLE='__builtin_unreachable()' -o release_perf $common_flags perf.c
echo "Build success"