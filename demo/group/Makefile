SRCS=groups.c

LIBMUI=../..

LIBS=-L ${LIBMUI} -Wl,-rpath=${LIBMUI} -lmui

INCS=-I ${LIBMUI}


all:
	make MUI_DEBUG=1 -C ${LIBMUI}
	${CC} ${SRCS} -o bin ${LIBS} ${INCS}

clean:
	rm -rf bin bin.core
