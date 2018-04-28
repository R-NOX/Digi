BINARY := data-handler

SOURCES = data-handler-1.0/main.c \
			data-handler-1.0/post.c \
			data-handler-1.0/db.c
OBJ = main.o post.o db.o

CFLAGS += -Wall -O0

$(BINARY): $(OBJ)
	$(CC) -L../librnox $(LDFLAGS) $^ $(LDLIBS) -o $@ -lrnox -lsqlite3 -lcurl

$(OBJ): $(SOURCES)
	$(CC) -I/home/herman/Projects/RNOX/dey-2.4/sources/meta-rnox/recipes-core/librnox -g -c $(SOURCES)

.PHONY: install
install: $(BINARY)
	install -d $(DESTDIR)/usr/bin
	install -m 0755 $^ $(DESTDIR)/usr/bin/

.PHONY: all
all: $(BINARY)

.PHONY: clean
clean:
	-rm -f *.o $(BINARY)