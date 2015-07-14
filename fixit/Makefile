LIB+=-ltiff
CFLAGS=-O2 -g -Wall
OBJS=tiff_helper.o check_baseline.o check_datetime.o cleanup_datetime.o cleanup_baseline.o cleanup_order.o fixit_tiff.o

# default target
all: fixit_tiff

# remove debugging symbols (smaller size)
strip: all
	strip --strip-all fixit_tiff

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

# default executable
fixit_tiff: $(OBJS)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIB) 

# doc
doc: ../common/doxygen.conf
	@doxygen ../common/doxygen.conf

# clean workdir
clean:
	@rm -f *.o
	@rm -f fixit_tiff

# mrproper clean
distclean: clean
	@rm -f *~
	@rm -Rf doc/

.PHONY: all clean distclean strip
