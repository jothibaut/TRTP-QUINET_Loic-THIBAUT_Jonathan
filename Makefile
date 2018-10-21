# Default target
all: clean sender receiver
 
# If we run `make debug` instead, keep the debug symbols for gdb
# and define the DEBUG macro.
debug: CFLAGS += -g -DDEBUG -Wno-unused-parameter -fno-omit-frame-pointer
debug: clean sender receiver

# We use an implicit rule to build an executable named 'sender'
sender: 
	cd src && $(MAKE)

receiver: 
	cd src && $(MAKE)

tests:
	cd tests && $(MAKE)

.PHONY: clean

clean:
	cd src && $(MAKE) clean
