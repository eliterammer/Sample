all: 
	 gcc -w -g -Wall house_keeper.c ramdisk.c `pkg-config fuse --cflags --libs` -o ramdisk
clean:
	rm -rf *.o ./a.out
