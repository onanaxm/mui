SRCS=image.c
LIBDIR=../..
LIBS=-L ${LIBDIR} -lmui
INCS=-I ${LIBDIR} -Wl,-rpath=${LIBDIR}


all:
	make -C ${LIBDIR}
	${CC} ${SRCS} -o bin ${LIBS} ${INCS}

clean:
	rm -rf bin bin.core
