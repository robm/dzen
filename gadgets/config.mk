# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
INCS = -I. -I/usr/include -I${X11INC}

X11LIB = /usr/X11R6/lib

# Configure the features you want to be supported
# Only one of the following options has to be uncommented,
# all others must be commented!
#
# Uncomment: Remove # from the beginning of respective lines
# Comment  : Add # to the beginning of the respective lines

## Option 1: No XFT
#LIBS = -L/usr/lib
#CFLAGS = -Os ${INCS}

## Option 2: With XFT
LIBS = -L/usr/lib `pkg-config --libs xft`
CFLAGS = -Os ${INCS} -DDZEN_XFT `pkg-config --cflags xft`

LDFLAGS = ${LIBS}

# compiler and linker
CC = gcc
LD = ${CC}
