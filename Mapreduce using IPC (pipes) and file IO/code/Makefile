CC=gcc
CFLAGS=-g

SRCDIR=src
INCLDIR=include
LIBDIR=lib

mapreduce: $(SRCDIR)/mapreduce.c $(LIBDIR)/utils.o mapper reducer stream
	$(CC) $(CFLAGS) -I$(INCLDIR) $(LIBDIR)/utils.o $(SRCDIR)/mapreduce.c -o mapreduce

mapper: $(SRCDIR)/mapper.c $(LIBDIR)/utils.o
	$(CC) $(CFLAGS) -I$(INCLDIR) $(LIBDIR)/utils.o $(SRCDIR)/mapper.c -o mapper

reducer: $(SRCDIR)/reducer.c $(LIBDIR)/utils.o
	$(CC) $(CFLAGS) -I$(INCLDIR)  $(LIBDIR)/utils.o $(SRCDIR)/reducer.c -o reducer

stream: $(SRCDIR)/stream.c $(LIBDIR)/utils.o
	$(CC) $(CFLAGS) -I$(INCLDIR)  $(LIBDIR)/utils.o $(SRCDIR)/stream.c -o stream

$(LIBDIR)/utils.o: $(LIBDIR)/utils.c
	$(CC) $(CFLAGS) -I$(INCLDIR) -c $(LIBDIR)/utils.c -o $(LIBDIR)/utils.o


.PHONY: clean

clean:
	rm lib/utils.o mapreduce mapper reducer stream
	rm -rf output
	rm -rf *.dSYM
	rm -rf MapperInput