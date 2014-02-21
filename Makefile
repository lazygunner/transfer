CFLAGS = -g -I include/
LDFLAGS = -pthread

transfer: transfer.o main.o log.o crc.o frame.o file.o thread.o lib/memory.o config.o lib/lib_minini.o

memory.o: lib/memory.c lib/memory.o
main.o: main.c
transfer.o: transfer.c transfer.o
log.o: log.c log.o
crc.o: crc.c crc.o
frame.o: frame.c frame.o
file.o: file.c file.o
thread.o: thread.c thread.o
lib_minini.o: lib/lib_minini.c lib/lib_minini.o
config.o: config.c config.o

.PHONY : clean
clean:
	-rm *.o
