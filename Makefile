SHELL=/bin/bash
.PHONY: default
default: assets/doom.squashfs
	. esp-idf/export.sh && cd fw/s3-boy-doom && idf.py set-target esp32s3 && idf.py build && cd build && esptool.py --chip esp32s3 merge_bin -t 0 --output s3-boy-doom-combined.bin `tail -n +2 flash_args` 0x110000 ../../../assets/doom.squashfs

.PHONY: squash
squash: assets/doom.squashfs
assets/doom.squashfs: assets/doom-root/doom/doom1.wad Makefile
	mksquashfs assets/doom-root $@ -noappend \
		-all-root -no-exports -no-sparse -no-xattrs -all-time 1 -noI \
		-b 32k -comp gzip -Xwindow-size 14 \

.PHONY: flash-squash
ifneq ($(origin PORT),undefined)
P_PORT=-p $(PORT)
else
P_PORT=
endif
flash-squash: assets/doom.squashfs
	esptool.py $(P_PORT) write_flash 0x110000 $<
