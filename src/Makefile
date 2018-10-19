# See gcc/clang manual to understand all flags
CFLAGS += -std=c99 # Define which version of the C standard to use
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing
CFLAGS += -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE # feature_test_macros for getpot and getaddrinfo
CFLAGS += -g

# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS= -rdynamic

# Default target
all: clean sender receiver
 
# If we run `make debug` instead, keep the debug symbols for gdb
# and define the DEBUG macro.
debug: CFLAGS += -g -DDEBUG -Wno-unused-parameter -fno-omit-frame-pointer
debug: clean sender receiver

# We use an implicit rule to build an executable named 'sender'
sender: sender.o read_write_loop.o create_socket.o real_address.o wait_for_client.o create_packet.o packet_implem.o -lz

receiver: receiver.o read_write_loop.o create_socket.o real_address.o wait_for_client.o create_packet.o packet_implem.o -lz

tests:
	cd tests && $(MAKE)

.PHONY: clean

clean:
	@rm -f sender sender.o receiver receiver.o read_write_loop.o create_socket.o real_address.o wait_for_client.o create_packet.o packet_implem.o
