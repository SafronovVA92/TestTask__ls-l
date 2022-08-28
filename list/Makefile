all: app

app: main.o
	gcc main.o -o app

main.o: main.c
	gcc -c main.c

clean:
	rm -rf *.o app