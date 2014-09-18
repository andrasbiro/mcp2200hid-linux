all: mcp2200io.o
	gcc mcp2200config.c mcp2200io.o -O2 -o mcp2200config
	gcc mcp2200gpio.c mcp2200io.o -O2 -o mcp2200gpio

clean:
	rm -f *.o
	rm -f mcp2200config
	rm -f mcp2200gpio

mcp2200io.o:
	gcc -c -O2 mcp2200io.c