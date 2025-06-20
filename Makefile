PREFIX?=/usr/local
LIBDIR=${PREFIX}/lib
CFLAGS= -O0 -Wall -Werror -Wno-unused-function -fPIC -std=c99


SRCS= xorg.c event.c text.c win.c image.c
INCS= -I /usr/X11R6/include -I /usr/X11R6/include/freetype2
LIBS= -L /usr/X11R6/lib -lxcb -lxcb-render -lxcb-render-util -lxcb-image
LIBS+= -lfreetype
MAN3=mui.3

OBJS=$(SRCS:.c=.o)



.ifdef defined(MUI_DEBUG) && ${MUI_DEBUG} > 0
    CFLAGS += -DMUI_DEBUG
.endif


all: mui

.c.o:
	${CC} -c ${CFLAGS} ${INCS} $< -o $@

mui: ${OBJS}
	${CC} -shared -Wl,-soname,libmui.so -o libmui.so ${OBJS} ${LIBS}

clean:
	make clean -C demo/hello_win
	make clean -C demo/multiline
	make clean -C demo/image
	rm -f libmui.so ${OBJS}

.PHONY: all mui
