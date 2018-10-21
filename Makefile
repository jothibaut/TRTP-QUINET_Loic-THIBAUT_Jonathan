all : sender receiver

sender: buffer.o create_packet.o create_socket.o packet_implem.o read_write_loop.o real_address.o wait_for_client.o sender.o
	gcc -o sender buffer.o create_packet.o create_socket.o packet_implem.o read_write_loop.o real_address.o wait_for_client.o sender.o -lz

receiver: buffer.o create_packet.o create_socket.o packet_implem.o read_write_loop.o real_address.o wait_for_client.o receiver.o
	gcc -o receiver buffer.o create_packet.o create_socket.o packet_implem.o read_write_loop.o real_address.o wait_for_client.o receiver.o -lz

buffer.o: src/buffer.c src/buffer.h
	gcc -c src/buffer.c -lz

create_packet.o: src/create_packet.c src/create_packet.h
	gcc -c src/create_packet.c -lz

create_socket.o: src/create_socket.c src/create_socket.h
	gcc -c src/create_socket.c -lz

packet_implem.o: src/packet_implem.c src/packet_interface.h
	gcc -c src/packet_implem.c -lz

read_write_loop.o: src/read_write_loop.c src/read_write_loop.h
	gcc -c src/read_write_loop.c -lz

real_address.o: src/real_address.c src/real_address.h
	gcc -c src/real_address.c -lz

wait_for_client.o: src/wait_for_client.c src/wait_for_client.h
	gcc -c src/wait_for_client.c -lz

receiver.o: src/receiver.c
	gcc -c src/receiver.c -lz

sender.o: src/sender.c
	gcc -c src/sender.c -lz
clean :
	rm sender receiver buffer.o create_socket.o create_packet.o packet_implem.o read_write_loop.o real_address.o receiver.o sender.o wait_for_client.o

tests:
	cd tests && $(MAKE)
