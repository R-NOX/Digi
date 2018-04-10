BINARY := opcn2-sensor

CFLAGS += -Wall -O0

CFLAGS += $(shell pkg-config --cflags libdigiapix)
LDLIBS += $(shell pkg-config --libs libdigiapix)

$(BINARY): main.o opcn2.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: install
install: $(BINARY)
	install -d $(DESTDIR)/usr/bin
	install -m 0755 $^ $(DESTDIR)/usr/bin/

.PHONY: all
all: $(BINARY)

.PHONY: clean
clean:
	-rm -f *.o $(BINARY)
