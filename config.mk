# dzen version
VERSION = 0.9.5-svn

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib
INCS = -I. -I/usr/include -I${X11INC}

# Configure the features you want to be supported
# Only one of the following options has to be uncommented,
# all others must be commented!
#
# Uncomment: Remove # from the beginning of respective lines
# Comment  : Add # to the beginning of the respective lines

## Option 1: No Xinerama no XPM no XFT
#LIBS = -L/usr/lib -lc -L${X11LIB} -lX11
#CFLAGS = -Wall -Os ${INCS} -DVERSION=\"${VERSION}\"


## Option 2: No Xinerama with XPM
#LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 -lXpm
#CFLAGS = -Wall -Os ${INCS} -DVERSION=\"${VERSION}\" -DDZEN_XPM


# Option 3: With Xinerama no XPM
#LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 -lXinerama
#CFLAGS = -Wall -Os ${INCS} -DVERSION=\"${VERSION}\" -DDZEN_XINERAMA


## Option 4: With Xinerama and XPM
#LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 -lXinerama -lXpm
#CFLAGS = -Wall -Os ${INCS} -DVERSION=\"${VERSION}\" -DDZEN_XINERAMA -DDZEN_XPM


## Option 5: With XFT
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 `pkg-config --libs xft`
CFLAGS = -Wall -Os ${INCS} -DVERSION=\"${VERSION}\" -DDZEN_XFT `pkg-config --cflags xft`


## Option 6: With XPM and XFT
#LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 -lXpm `pkg-config --libs xft`
#CFLAGS = -Wall -Os ${INCS} -DVERSION=\"${VERSION}\" -DDZEN_XPM -DDZEN_XFT `pkg-config --cflags xft`


## Option 7: With Xinerama and XFT
#LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 -lXinerama `pkg-config --libs xft`
#CFLAGS = -Wall -Os ${INCS} -DVERSION=\"${VERSION}\" -DDZEN_XINERAMA -DDZEN_XFT `pkg-config --cflags xft`


## Option 8: With Xinerama and XPM and XFT
#LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 -lXinerama -lXpm `pkg-config --libs xft`
#CFLAGS = -Wall -Os ${INCS} -DVERSION=\"${VERSION}\" -DDZEN_XINERAMA -DDZEN_XPM -DDZEN_XFT `pkg-config --cflags xft`



# END of feature configuration


LDFLAGS = ${LIBS}

# Solaris, uncomment for Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}
#CFLAGS += -xtarget=ultra

# Debugging
#CFLAGS = ${INCS} -DVERSION=\"${VERSION}\" -std=gnu89 -pedantic -Wall -W -Wundef -Wendif-labels -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Winline -Wdisabled-optimization -O2 -pipe -DDZEN_XFT `pkg-config --cflags xft`
#LDFLAGS = ${LIBS}

# compiler and linker
CC = gcc
LD = ${CC}
