#CFLAGS	      = -std=c99 -pedantic -Wall  -g -Ddebug
CFLAGS	      = -std=c99 -pedantic -Wall -O1000

DEST	      = .

EXTHDRS	      =

HDRS	      = sudoku_def.h

INSTALL	      = cp

LD	      = $(CC)

LDFLAGS	      =

LIBS	      =

MAKEFILE      = Makefile

OBJS	      = sudoku.o

PRINT	      = pr

PROGRAM       = sudoku.exe

SHELL	      = /bin/sh

SRCS	      = sudoku.c

SYSHDRS	      =

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS)
		@echo "Linking $(PROGRAM) ..."
		@$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS) $(PROGRAM) core

clobber:;	@rm -f $(OBJS) $(PROGRAM) core tags

depend:;	@mkmf -f $(MAKEFILE)

echo:;		@echo $(HDRS) $(SRCS)

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		@-strip $(PROGRAM)
		@if [ $(DEST) != . ]; then \
		(rm -f $(DEST)/$(PROGRAM); $(INSTALL) $(PROGRAM) $(DEST)); fi

print:;		@$(PRINT) $(HDRS) $(SRCS)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

$(OBJS):$(SRCS) $(HDRS) Makefile

