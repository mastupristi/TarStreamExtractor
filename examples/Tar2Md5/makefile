
all: tar2md5

TARSTEX_SRC_DIR = ../../src

SRCS = \
	tar2md5.c \
	digest2string.c \
	$(TARSTEX_SRC_DIR)/tarStreamExtractor.c

CFLAGS = \
	-Wall \
	-I. \
	-I$(TARSTEX_SRC_DIR) \
	-O0 \
	-g3


LIBS = \
	-lssl \
	-lcrypto

tar2md5: $(SRCS)
	gcc $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm tar2md5