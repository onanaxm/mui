PREFIX?=/usr/local
CFLAGS= -O0 -Wall -Werror -Wno-unused-function -fPIC -std=c99


SRCS= xorg.c event.c text.c win.c image.c group.c
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


install: mui
	install -m 644 libmui.so ${PREFIX}/lib
	install -m 644 mui.h ${PREFIX}/include


uninstall:
	rm -rf ${PREFIX}/lib/libmui.so
	rm -rf ${PREFIX}/include/mui.h


clean:
	make clean -C demo/hello_win
	make clean -C demo/multiline
	make clean -C demo/image
	make clean -C demo/group
	rm -f libmui.so ${OBJS}

.PHONY: all mui install uninstall
