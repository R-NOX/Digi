BINARY := opcn2-sensor

SOURCES = opcn2-sensor-1.0/main.c \
			opcn2-sensor-1.0/opcn2.c \
			
OBJ = main.o opcn2.o

CFLAGS += -Wall -O0

CFLAGS += $(shell pkg-config --cflags libdigiapix)
LDLIBS += $(shell pkg-config --libs libdigiapix) -lrnox

$(BINARY): $(OBJ)
	$(CC) -L../librnox $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ): $(SOURCES)
	$(CC) -g -c $(SOURCES)
	
.PHONY: install
install: $(BINARY)
	install -d $(DESTDIR)/usr/bin
	install -m 0755 $^ $(DESTDIR)/usr/bin/

.PHONY: all
all: $(BINARY)

.PHONY: clean
clean:
	-rm -f *.o $(BINARY)
