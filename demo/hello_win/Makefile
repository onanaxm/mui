SRCS=hello_win.c
LIBDIR=../..
INCS= -I ${LIBDIR}
LIBS=-Wl,-rpath=${LIBDIR} -L ${LIBDIR} -lmui

all:
	make MUI_DEBUG=1 -C ${LIBDIR}
	${CC} ${SRCS} -o bin ${INCS} ${LIBS}

clean:
	rm -rf bin bin.core
