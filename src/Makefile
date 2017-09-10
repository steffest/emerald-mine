# build emerald mine

CFLAGS = -O
LDFLAGS = -s

default:
	@echo
	@echo 'usage: make <target>'
	@echo
	@echo '<target> = x11 [ http://www.x.org/ ]'
	@echo '<target> = sdl [ http://www.libsdl.org/ ]'
	@echo '<target> = allegro [ http://alleg.sourceforge.net/ ]'
	@echo
	@echo '<target> = linux | mingw | darwin | amiga | djgpp'
	@echo

#

OBJ = logic.o convert.o binary.o game.o file.o unzip.o datamap.o datagfx.o datasnd.o dataulaw.o

logic.o: logic.c emerald.h
	$(COMPILE.c) logic.c
convert.o: convert.c emerald.h cave.h
	$(COMPILE.c) convert.c
binary.o: binary.c cave.h
	$(COMPILE.c) binary.c

game.o: game.c emerald.h cave.h proto.h
	$(COMPILE.c) game.c
file.o: file.c proto.h
	$(COMPILE.c) file.c
unzip.o: unzip.c
	$(COMPILE.c) unzip.c
datamap.o: datamap.c proto.h
	$(COMPILE.c) datamap.c
datagfx.o: datagfx.c proto.h
	$(COMPILE.c) datagfx.c
datasnd.o: datasnd.c proto.h
	$(COMPILE.c) datasnd.c
dataulaw.o: dataulaw.c proto.h
	$(COMPILE.c) dataulaw.c

#

x11.o: x11.c proto.h
	$(COMPILE.c) x11.c
sdl.o: sdl.c proto.h
	$(COMPILE.c) `sdl-config --cflags` sdl.c
allegro.o: allegro.c proto.h
	$(COMPILE.c) `allegro-config --cflags` allegro.c

x11: $(OBJ) x11.o
	$(LINK.c) $(OBJ) x11.o -lX11 -o emerald
sdl: $(OBJ) sdl.o
	$(LINK.c) $(OBJ) sdl.o `sdl-config --libs` -o emerald
allegro: $(OBJ) allegro.o
	$(LINK.c) $(OBJ) allegro.o `allegro-config --libs` -o emerald

#

linux.o: x11.c proto.h
	$(COMPILE.c) x11.c
mingw.o: mingw.c proto.h
	$(COMPILE.c) mingw.c
darwin.o: darwin.c proto.h
	$(COMPILE.c) darwin.c
amiga.o: amiga.c proto.h
	$(COMPILE.c) -noixemul amiga.c
djgpp.o: allegro.c proto.h
	$(COMPILE.c) allegro.c

Icon.o: Icon.rc Icon.ico
	$(CC:gcc=windres) Icon.rc -o Icon.o

linux: $(OBJ) linux.o
	$(LINK.c) $(OBJ) x11.o -lX11 -lasound -o emerald
mingw: $(OBJ) mingw.o Icon.o
	$(LINK.c) $(OBJ) mingw.o Icon.o -lgdi32 -lwinmm -mwindows -o emerald
darwin: $(OBJ) darwin.o
	$(LINK.c) $(OBJ) darwin.o -framework Carbon -o emerald
amiga: $(OBJ) amiga.o
	$(LINK.c) $(OBJ) amiga.o -noixemul -o emerald
djgpp: $(OBJ) djgpp.o
	$(LINK.c) $(OBJ) allegro.o -lalleg -o emerald

#

INSTALL = install
DESTDIR = /usr/local/

install:
	$(INSTALL) -d $(DESTDIR)/bin/
	$(INSTALL) -c emerald $(DESTDIR)/bin/

clean:
	$(RM) emerald
	$(RM) logic.o convert.o binary.o
	$(RM) game.o file.o unzip.o datamap.o datagfx.o datasnd.o dataulaw.o
	$(RM) x11.o sdl.o allegro.o
	$(RM) mingw.o darwin.o amiga.o experimental.o Icon.o
