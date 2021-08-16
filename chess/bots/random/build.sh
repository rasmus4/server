set -e

common_flags="-Wpedantic -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough"
gcc -fsanitize=undefined -fsanitize=address -g -DUNREACHABLE='assert(0)' -o random $common_flags random.c
gcc -static -s -O2 -DNDEBUG -DUNREACHABLE='__builtin_unreachable()' -o release_random $common_flags random.c
echo "Build success"