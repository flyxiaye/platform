dir_y += vi
dir_y += venc
#dir_y += vi
dir_y += adec
dir_y += aenc
dir_y += vdec
dir_y += vo
dir_y += tde
dir_y += ao
dir_y += ai
dir_y += drv
dir_y += test
dir_y += APP

all:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i; \
		if [ $$? -ne 0 ];then exit 1; fi \
	done

install:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i install; \
	done

image:
	@echo ""

clean:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i clean; \
	done

.PHONY: all install clean
