# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
INCS = -I. -I/usr/include -I${X11INC}

X11LIB = /usr/X11R6/lib
LIBS = -L/usr/lib 

CFLAGS = -Os ${INCS} 
LDFLAGS = ${LIBS}

# compiler and linker
CC = gcc
LD = ${CC}
