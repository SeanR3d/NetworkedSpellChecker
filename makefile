# Networked Spell Checker makefile
# by Sean Reddington
# October 31, 2018

TARGETS = driver

CC_C = $(CROSS_TOOL)gcc

CFLAGS = -g
all: clean $(TARGETS)

$(TARGETS): driver.c server.h workQueue.h logQueue.h spellChecker.h
	$(CC_C) $(CFLAGS) $@.c -o spellcheck -lpthread

clean:
	rm -f $(TARGETS)