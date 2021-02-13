#gcc -fsanitize=undefined -Wpedantic -Wall -o server -Iinclude server.c

gcc -g -O -Iinclude -Wall -static -fno-pie -no-pie -mno-red-zone -nostdlib -nostdinc \
  -o hello.com.dbg server.c -Wl,--gc-sections -Wl,-z,max-page-size=0x1000 -fuse-ld=bfd \
  -Wl,-T,ape.lds -include cosmopolitan.h crt.o ape.o cosmopolitan.a
objcopy -SO binary hello.com.dbg hello.com