# List of all the board related files.
BOARDSRC = $(ROOT)/boards/ST_NUCLEO144_F429ZI/board.c

# Required include directories
BOARDINC = $(ROOT)/boards/ST_NUCLEO144_F429ZI

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
