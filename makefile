all: ted.o fileops.o linked-list.o str-utils.o
	gcc -g ted.o fileops.o linked-list.o str-utils.o -lncurses

ted.o: ted.c ted.h
	gcc -c ted.c -lncurses

fileops.o: fileops/fileops.c fileops/fileops.h
	gcc -c fileops/fileops.c

linked-list.o: linked-list.c linked-list.h
	gcc -c linked-list.c

str-utils.o: str-utils/str-utils.h str-utils/str-utils.c
	gcc -c str-utils/str-utils.c

clean:
	rm *.o