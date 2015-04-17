SHELL=/usr/bin/env bash

OSTEST=$(shell echo $$MACHTYPE | grep 64)

ifeq ("${OSTEST}", "")
	POSFIX=.32
endif

COMP_EXE=compare${POSFIX}
STR_EXE=drawstroke${POSFIX}

all:
	@if [ "" == "${POSFIX}" ]; then\
		echo "64-bits MACHINE";\
	else\
		echo "32-bits MACHINE";\
	fi
	@echo "type make [compare|stroke] to get executable"

compare: compare.c
	gcc --std=c99 compare.c -o ${COMP_EXE}

stroke: drawstroke.c
	gcc drawstroke.c -o ${STR_EXE} `pkg-config --libs --cflags gtk+-2.0 gdk-2.0 cairo`
