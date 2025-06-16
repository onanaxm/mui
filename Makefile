PREFIX?=/usr/local
LIBDIR=${PREFIX}/lib
CFLAGS= -O0 -Wall -Werror -Wno-unused-function -fPIC -std=c99


SRCS= mui.c xorg.c event.c text.c win.c
INCS= -I /usr/X11R6/include -I /usr/X11R6/include/freetype2
LIBS= -L /usr/X11R6/lib -lxcb -lxcb-render -lxcb-render-util
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
	rm -f libmui.so ${OBJS}

.PHONY: all mui
