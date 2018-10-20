all: exec

exec: test.c lib
	gcc -o test test.c -I$(HOME)/local/include -L$(HOME)/local/lib -lcunit -lz 
	#gcc -Wall -I$HOME/local/include test.c -L$HOME/local/lib -lcunit -lz -o test 
lib: 
	cd ../src && $(MAKE) clean && $(MAKE)
clean: test
	cd ../src && $(MAKE) clean
	cd ../tests
	rm test
