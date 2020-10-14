CC= cc
DEFS=
PROGNAME= grabc
LIBS= -L/usr/X11R6/lib -lX11

INCLUDES=  -I/usr/X11R6/include

DEFINES= $(INCLUDES) $(DEFS) -D__USE_FIXED_PROTOTYPES__ -DSYS_UNIX=1
CFLAGS= -O $(DEFINES)

SRCS = grabc.c
OBJS = grabc.o

.c.o:
	rm -f $@
	$(CC) $(CFLAGS) -c $*.c

all: $(PROGNAME)

$(PROGNAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGNAME) $(OBJS) $(LIBS)

install:
	install ${PROGNAME} /usr/local/bin/${PROGNAME}

clean:
	rm -f $(OBJS) $(PROGNAME) core
