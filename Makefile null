all: edit

edit : sender.o receiver.o packet_implem.o
	gcc -g -Wall -Werror -o edit sender.o receiver.o  packet_implem.o

sender.o : sender.c read_write_loop.o create_socket.o real_address.o wait_for_client.o create_packet.o packet_implem.o
	gcc -g -Wall -Werror -o sender.c -lz

receiver.o : receiver.c	read_write_loop.o create_socket.o real_address.o wait_for_client.o create_packet.o packet_implem.o
	gcc -g -Wall -Werror -o receiver.c -lz

packet_implem.o : packet_implem.c packet_interface.h
	gcc -g -Wall -Werror -o packet_implem.c -lz

tests: tests

clean :
	@rm -f edit sender sender.o receiver receiver.o packet_implem.o
