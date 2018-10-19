all: exec

exec: test.c lib
	gcc -o test test.c -lz
lib: 
	cd ../src && $(MAKE) clean && $(MAKE)
clean: test
	cd ../src && $(MAKE) clean
	cd ../tests
	rm test
