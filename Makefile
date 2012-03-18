# dzen2
#   (C)opyright MMVII Robert Manea

include config.mk

SRC = draw.c main.c util.c action.c
OBJ = ${SRC:.c=.o}

all: options dzen2

options:
	@echo dzen2 build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "LD       = ${LD}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: dzen.h action.h config.mk

dzen2: ${OBJ}
	@echo LD $@
	@${LD} -o $@ ${OBJ} ${LDFLAGS}
	@strip $@
	@echo "Run ./help for documentation"

clean:
	@echo cleaning
	@rm -f dzen2 ${OBJ} dzen2-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p dzen2-${VERSION}
	@mkdir -p dzen2-${VERSION}/gadgets
	@mkdir -p dzen2-${VERSION}/bitmaps
	@cp -R CREDITS LICENSE Makefile INSTALL README.dzen README help config.mk action.h dzen.h ${SRC} dzen2-${VERSION}
	@cp -R gadgets/Makefile  gadgets/config.mk gadgets/README.dbar gadgets/textwidth.c gadgets/README.textwidth gadgets/dbar.c gadgets/gdbar.c gadgets/README.gdbar gadgets/gcpubar.c gadgets/README.gcpubar gadgets/kittscanner.sh gadgets/README.kittscanner gadgets/noisyalert.sh dzen2-${VERSION}/gadgets
	@cp -R bitmaps/alert.xbm bitmaps/ball.xbm bitmaps/battery.xbm bitmaps/envelope.xbm bitmaps/volume.xbm bitmaps/pause.xbm bitmaps/play.xbm bitmaps/music.xbm  dzen2-${VERSION}/bitmaps
	@tar -cf dzen2-${VERSION}.tar dzen2-${VERSION}
	@gzip dzen2-${VERSION}.tar
	@rm -rf dzen2-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f dzen2 ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/dzen2

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/dzen2

.PHONY: all options clean dist install uninstall
