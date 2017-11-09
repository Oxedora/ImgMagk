CC=cc
CFLAGS=`pkg-config --cflags --libs MagickWand`

imgMagk : main.c
	$(CC) -o imgMagk main.c $(CFLAGS)

clean :
	rm -f *~ *.o imgMagk
