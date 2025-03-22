
all: untar

TARSTEX_SRC_DIR = ../../src

SRCS = \
	untar.c \
	$(TARSTEX_SRC_DIR)/tarStreamExtractor.c

CFLAGS = \
	-Wall \
	-I. \
	-I$(TARSTEX_SRC_DIR) \
	-O0 \
	-g3

untar: $(SRCS)
	gcc $(CFLAGS) $^ -o $@

clean:
	rm untar