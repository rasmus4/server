set -e

name="perf"
common_flags="-Wpedantic -Wall -Wextra -Wconversion -Wno-unused-parameter -Wno-implicit-fallthrough"
CC=${CC:-gcc}
$CC -fsanitize=undefined -fsanitize=address -g -DUNREACHABLE='assert(0)' -o debug_$name.bin $common_flags $name.c
$CC -static -g -DUNREACHABLE='assert(0)' -o $name.bin $common_flags $name.c
$CC -static -s -O2 -DNDEBUG -DUNREACHABLE='__builtin_unreachable()' -o release_$name.bin $common_flags $name.c
echo "Build success"