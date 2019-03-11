object_files = main.o grid.o utilities.o directives.o minimax.o
header_files = hex.h grid.h directives.h

CC = gcc
CFLAGS = -Wall

hex: $(object_files)
	$(CC) $(CFLAGS) $(object_files) -o hex

main.o: $(header_files)

grid.o: $(header_files)

utilities.o: $(header_files)

directives.o: $(header_files)

minimax.o: $(header_files)

clean:
	rm hex $(object_files)