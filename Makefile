LIB=-ltiff 
CFLAGS=-Os

# default target
all: fixit_tiff

# remove debugging symbols (smaller size)
strip: all
	strip --strip-all fixit_tiff

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# default executable
fixit_tiff: fixit_tiff.o
	$(CC) $(CFLAGS) -o $@ $(LIB) $<

# clean workdir
clean:
	@rm -f *.o
	@rm -f fixit_tiff

# mrproper clean
distclean: clean
	@rm -f *~

.PHONY: all clean distclean strip
