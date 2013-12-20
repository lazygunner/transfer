CFLAGS = -g

transfer: transfer.o main.o log.o crc.o frame.o

main.o: main.c
transfer.o: transfer.c transfer.o
log.o: log.c log.o
crc.o: crc.c crc.o
frame.o: frame.c frame.o
