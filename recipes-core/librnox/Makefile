LIB = librnox

SOURCES = log.c queue.c
DEPS = log.h queue.h
OBJ = log.o queue.o

CFLAGS += -Wall -O0

$(LIB): $(OBJ)
	$(CC) -shared -o $(LIB).so $(OBJ) -ljansson
	
$(OBJ): $(SOURCES) $(DEPS)
	$(CC) -c -fpic $(SOURCES)
	
.PHONY: all
all: $(LIB)

.PHONY: clean
clean:
	-rm -f *.o $(LIB).so
