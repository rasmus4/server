set -e

(cd ../htmlCompiler && ./build.sh)
(cd client && ../../htmlCompiler/htmlCompiler html/main.html generatedHtml)
echo "Client build success"

common_flags="-Wpedantic -Wall -Wextra -Wno-unused-parameter -Iserver -I../server -Iclient"
gcc -fsanitize=undefined -fsanitize=address -g -DUNREACHABLE='assert(0)' -o chess $common_flags server/chess.c
gcc -static -s -O2 -DNDEBUG -DUNREACHABLE='__builtin_unreachable()' -o release_chess $common_flags server/chess.c
echo "Server build success"