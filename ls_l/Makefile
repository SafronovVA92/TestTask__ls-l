all: ls_l

ls_l: main.o
	gcc main.o -o ls_l

main.o: main.c
	gcc -c main.c

clean:
	rm -rf *.o ls_l