set -e

(cd ../htmlPacker && ./build.sh)
(cd client && ../../htmlPacker/htmlPacker html/main.html generatedHtml)
echo "Client build success"