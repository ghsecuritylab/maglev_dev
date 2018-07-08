# List of all the board related files.
BOARDSRC = $(ROOT)/boards/dual_controller_rev1/board.c

# Required include directories
BOARDINC = $(ROOT)/boards/dual_controller_rev1

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
