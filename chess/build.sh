set -e

(cd ../htmlCompiler && ./build.sh)
(cd client && ../../htmlCompiler/htmlCompiler html/main.html generatedHtml)
echo "Client build success"

common_flags="-Wpedantic -Wall -Wextra -Wno-unused-parameter -Iserver -I../server -Iclient"
gcc -fsanitize=undefined -fsanitize=address -g -o chess $common_flags server/chess.c
gcc -static -O2 -DNDEBUG -s -o release_chess $common_flags server/chess.c
echo "Server build success"