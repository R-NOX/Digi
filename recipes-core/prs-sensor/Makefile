BINARY := prs-sensor

SOURCES = prs-sensor-1.0/main.c
OBJ = main.o

CFLAGS += -Wall -O0

CFLAGS += $(shell pkg-config --cflags libdigiapix)
LDLIBS += $(shell pkg-config --libs libdigiapix)

$(BINARY): $(OBJ)
	$(CC) -L../librnox $(LDFLAGS) $^ $(LDLIBS) -o $@ -lrnox-1.0

$(OBJ): $(SOURCES)
	$(CC) -I../librnox -g -c $(SOURCES)

.PHONY: install
install: $(BINARY)
	install -d $(DESTDIR)/usr/bin
	install -m 0755 $^ $(DESTDIR)/usr/bin/

.PHONY: all
all: $(BINARY)

.PHONY: clean
clean:
	-rm -f *.o $(BINARY)
