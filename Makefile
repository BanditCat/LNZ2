################################################################################
## Copyright (c) Jonathan(Jon) DuBois 2013. This file is part of LNZ.         ##
##                                                                            ##
## LNZ is free software: you can redistribute it and/or modify it under the   ##
## terms of the GNU General Public License as published by the Free Software  ##
## Foundation, either version 3 of the License, or (at your option) any later ##
## version.                                                                   ##
##                                                                            ##
## LNZ is distributed in the hope that it will be useful, but WITHOUT ANY     ##
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  ##
## FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more     ##
## details.                                                                   ##
##                                                                            ##
## You should have received a copy of the GNU General Public License along    ##
## with LNZ, located in the file LICENSE.  If not, see                        ##
## <http://www.gnu.org/licenses/>.                                            ##
##                                                                            ##
## Alternative licensing terms may be available for a fee, contact Jon DuBois ##
## (bcj1980@gmail.com) for details.                                           ##
################################################################################



.PHONY: all
all: depend debug



# Toolchain.
CC=gcc
CCFLAGS=-std=c11 -Wall -fno-exceptions -pedantic -Wextra -Werror -c
CCINCFLAG=-I/include/SDL2 -Iglew
LD=gcc
LDFLAGS=-Lglew -lglew32 -lSDL2_test -lmingw32 -lSDL2main -lSDL2 -lopengl32 -mwindows


TARGET=lnz.exe
TARGETDEFINE=-DWINDOWS
OBJS:=$(OBJS) windowsResource.o
windowsResource.o: windowsResource.rc bcj.ico main.glsl frag.frag vert.vert
	windres $< -o $@

# Actual build rules.
# These are supposed everything that might be edited.
TXTS:=$(TXTS) $(wildcard ./*.txt) ./Makefile ./README ./LICENSE ./windowsResource.rc
SRCS:=$(SRCS) $(wildcard ./*.h) $(wildcard ./*.c)
CS:=$(CS) $(wildcard ./*.c)
OBJS:=$(OBJS) $(CS:.c=.o)
$(OBJS): Makefile
include deps

# Override defaults
%.o: %.c
	$(CC) $(CCINCFLAG) $(CCFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(LD) $^ -o $@ $(LDFLAGS)
	$(STRIP)
	$(PACK)

TAGS: $(SRCS)
	etags --declarations --ignore-indentation $^


.PHONY: release 
release: $(TARGET)
release: CCFLAGS:=-O4 $(TARGETDEFINE) -flto $(CCFLAGS)
release: LDFLAGS:=-O4 -flto $(LDFLAGS)
release: STRIP:=strip -p $(TARGET)
release: PACK:=upx --best $(TARGET)

.PHONY: debug 
debug: $(TARGET)
debug: CPPFLAGS:=$(TARGETDEFINE) -DDEBUG $(CPPFLAGS)


.PHONY: clean
clean:
	rm -f ./*.o ./$(TARGET)

.PHONY: backup
backup: release
	git commit -a -m "$(shell cat ~/lnz/workingon.txt)" || true

.PHONY: depend
depend:
	gcc $(CCINCFLAG) $(TARGETDEFINE) -MM $(CS) > ./deps

.PHONY: run
run: all
	./$(TARGET)

.PHONY: unixify
unixify:
	dos2unix -U $(TXTS) $(SRCS)
