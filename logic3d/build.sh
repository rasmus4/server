set -e

(cd ../htmlPacker && ./build.sh)
(cd client && ../../htmlPacker/htmlPacker.bin html/main.html generatedHtml)
echo "Client build success"