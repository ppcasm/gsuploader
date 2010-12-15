PROG = gsuploader
OBJS = gsuploader.o

CFLAGS = -DLINUX

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS)

clean:
	rm -f $(PROG) $(OBJS)
