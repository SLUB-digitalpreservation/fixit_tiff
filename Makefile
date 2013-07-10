LIB+=-ltiff
CFLAGS=-Os -Wall

# default target
all: fixit_tiff

# remove debugging symbols (smaller size)
strip: all
	strip --strip-all fixit_tiff

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

# default executable
fixit_tiff: fixit_tiff.o
	$(CC) $(CFLAGS) $(INC) $(LIB) -o $@ $<

# doc
doc: doxygen.conf
	@doxygen doxygen.conf

# clean workdir
clean:
	@rm -f *.o
	@rm -f fixit_tiff

# mrproper clean
distclean: clean
	@rm -f *~
	@rm -Rf doc/

.PHONY: all clean distclean strip
