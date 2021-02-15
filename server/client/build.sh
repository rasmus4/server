set -e
gcc -fsanitize=undefined -Wall -Wpedantic -o htmlCompiler htmlCompiler.c
./htmlCompiler html/main.html generatedHtml
echo "Client build success"