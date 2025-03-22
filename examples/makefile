SUBDIRS := Tar2Md5 unTar

all: $(SUBDIRS)

.PHONY: $(SUBDIRS) clean

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
