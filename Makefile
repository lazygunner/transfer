CFLAGS = -g 
LDFLAGS = -pthread

transfer: transfer.o main.o log.o crc.o frame.o file.o thread.o memory.o

memory.o: memory.c memory.o
main.o: main.c
transfer.o: transfer.c transfer.o
log.o: log.c log.o
crc.o: crc.c crc.o
frame.o: frame.c frame.o
file.o: file.c file.o
thread.o: thread.c thread.o

.PHONY : clean
clean:
	-rm *.o
