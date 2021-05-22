set -e

(cd ../htmlCompiler && ./build.sh)
(cd client && ../../htmlCompiler/htmlCompiler html/main.html generatedHtml)
echo "Client build success"