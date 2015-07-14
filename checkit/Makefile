LIB+=-ltiff
CFLAGS=-O2 -g -Wall
GRAMMAR=config_dsl.grammar.c
OBJS=../fixit/tiff_helper.o checkit_tiff.o

# default target
all: checkit_tiff test

# remove debugging symbols (smaller size)
#strip: all
#	strip --strip-all checkit_tiff

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

# produce parser via PEG
%.c: %.peg
	peg $< > $@

# default executable
checkit_tiff: $(OBJS)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIB) 

test.c: $(GRAMMAR)

# doc
doc: ../common/doxygen.conf
	@doxygen ../common/doxygen.conf

# clean workdir
clean:
	@rm -f *.o
	@rm -f checkit_tiff test

# mrproper clean
distclean: clean
	@rm -f *~
	@rm -Rf doc/

.PHONY: all clean distclean strip
