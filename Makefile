all: edit

edit : sender.o receiver.o packet_implem.o
	cc -o edit sender.o receiver.o  packet_implem.o

sender.o : sender.c sender
	cc -c sender.c -lz

receiver.o : receiver.c receiver
	cc -c receiver.c -lz

packet_implem.o : packet_implem.c packet_implem.h
	cc -c packet_implem.c -lz

tests: tests

clean :
	@rm -f edit sender.o receiver.o packet_implem.o
