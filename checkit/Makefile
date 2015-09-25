# rule based checks if given TIFF is a specific baseline TIFF
# author: Andreas Romeyke, 2015
# licensed under conditions of libtiff 
# (see http://libtiff.maptools.org/misc.html)

### needs libtiff (>= v4)
###       libpcre (>=v3)

LIB+=-lpcre -ltiff -lm
SANITIZE?=-fsanitize=undefined -fsanitize=shift \
-fsanitize=integer-divide-by-zero -fsanitize=unreachable \
-fsanitize=vla-bound -fsanitize=null -fsanitize=return \
-fsanitize=signed-integer-overflow 
#-fsanitize=leak
CFLAGS?= -g -Wall $(SANITIZE)
GRAMMAR=config_dsl.grammar.c
SRCS=$(wildcard tagrules/*.c) check_helper.c check_ifd.c config_parser.c
OBJS=$(patsubst %.c,%.o, $(SRCS))

# default target
all: checkit_tiff test

# remove debugging symbols (smaller size)
strip: all
	strip --strip-all checkit_tiff

config_parser.c: config_dsl.grammar.c

config_dsl.grammar.c: config_dsl.grammar.peg

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

# produce parser via PEG
%.c: %.peg
	peg $< > $@

-include version.mk

checkit_tiff.o: checkit_tiff.c
	$(CC) -DVERSION="\"0.$(REPO_REVISION)\""  $(CFLAGS) $(INC) -c $< -o $@

# default executable
checkit_tiff: $(OBJS) checkit_tiff.o
	$(CC) -o $@ $(CFLAGS) $(INC) $^ $(LIB)

test: $(OBJS) test.o
	$(CC) -o $@ $(CFLAGS) $(INC) $^ $(LIB)

test.c: $(GRAMMAR)

# doc
doc: ../common/doxygen.conf
	@doxygen ../common/doxygen.conf

# clean workdir
clean:
	@rm -f *.o tagrules/*.o
	@rm -f checkit_tiff test

# mrproper clean
distclean: clean
	@rm -f *~ tagrules/*~
	@rm -f core
	@rm -Rf doc/
	@rm -f .depend
	@rm -f *.d
	@rm -f .depend


depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend;

-include .depend

.PHONY: all clean distclean strip depend


