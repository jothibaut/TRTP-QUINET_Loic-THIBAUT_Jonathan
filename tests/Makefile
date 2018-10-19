all: exec

exec: test.c lib
	gcc -o test test.c -I$(HOME)/local/include -L$(HOME)/local/lib -lz -lcunit
lib: 
	cd ../src && $(MAKE) clean && $(MAKE)
clean: test
	cd ../src && $(MAKE) clean
	cd ../tests
	rm test
