all:
	@echo "Please specify a target, try: make [ab | ba]"
     
ab: main.o liba.a libb.a
	gcc main.o -L. -la -lb
 
ba: main.o liba.a libb.a
	gcc main.o -L. -lb -la
 
main.o: main.c
	gcc main.c -c
 
liba.a: a.c
	gcc a.c -c
	ar -cr liba.a a.o
 
libb.a: b.c
	gcc b.c -c
	ar -cr libb.a b.o
 
clean:
	rm -fr *.o *.a *.so a.out
