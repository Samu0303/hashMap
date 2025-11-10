CC = gcc
CCFLAGS = -std=c11 -Wall -pedantic 

.PHONY: clean run check

main: ./main.c ./hashMap.o 
	$(CC) $(CCFLAGS) ./main.c hashMap.o -o main

hashMap.o: ./hashMap.c ./hashMap.h
	$(CC) $(CCFLAGS) -DDEBUG ./hashMap.c -c -o hashMap.o

run: ./main
	./main > test.table

check:	./main
	valgrind -s --leak-check=full ./main > /dev/null

clean:
	rm -rf ./main ./hashMap.o ./test.table
