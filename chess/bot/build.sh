set -e

common_flags="-Wpedantic -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -Iinclude"
gcc -fsanitize=undefined -fsanitize=address -g -DUNREACHABLE='assert(0)' -o bot $common_flags bot.c
gcc -static -s -O2 -DNDEBUG -DUNREACHABLE='__builtin_unreachable()' -o release_bot $common_flags bot.c
echo "Bot build success"