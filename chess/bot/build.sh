set -e

common_flags="-Wpedantic -Wall -Wextra -Wno-unused-parameter -Iinclude"
gcc -fsanitize=undefined -fsanitize=address -g -DUNREACHABLE='printf("unreachable (%s:%d)\n", __FILE__, __LINE__)' -o bot $common_flags bot.c
gcc -static -s -O2 -DNDEBUG -DUNREACHABLE='__builtin_unreachable()' -o release_bot $common_flags bot.c
echo "Bot build success"