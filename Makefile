ifeq ($(origin IDF_PATH),undefined)
$(error You must "source esp-idf/export.sh" before building)
endif

SHELL=/bin/bash
.PHONY: default
default: fw/s3-boy-doom/build/build.ninja assets/doom.squashfs
	echo "make: Entering directory \`$(abspath fw/s3-boy-doom/build/)'"
	cd fw/s3-boy-doom && idf.py build && cd build && esptool.py --chip esp32s3 merge_bin -t 0 --output s3-boy-doom-combined.bin `tail -n +2 flash_args` 0x110000 ../../../assets/doom.squashfs
	cd fw/s3-boy-doom/build && esptool.py --chip esp32s3 merge_bin -t 0 --output s3-boy-doom-code.bin `tail -n +2 flash_args`
	cp fw/s3-boy-doom/build/s3-boy-doom-combined.bin assets/
	cp fw/s3-boy-doom/build/s3-boy-doom-code.bin assets/

fw/s3-boy-doom/build/build.ninja:
	cd fw/s3-boy-doom && idf.py set-target esp32s3

.PHONY: squash
squash: assets/doom.squashfs
assets/doom.squashfs: assets/doom-root/doom/doom1.wad Makefile
	mksquashfs assets/doom-root $@ -noappend \
		-all-root -no-exports -no-sparse -no-xattrs -all-time 1 -noI \
		-b 32k -comp lz4 -Xhc

.PHONY: flash-squash flash-code flash-all
ifneq ($(origin PORT),undefined)
P_PORT=-p $(PORT)
else
P_PORT=
endif
flash-squash: assets/doom.squashfs
	esptool.py $(P_PORT) write_flash 0x110000 $<
flash-code: default
	esptool.py $(P_PORT) write_flash 0 assets/s3-boy-doom-code.bin
flash-all: default
	esptool.py $(P_PORT) write_flash 0 assets/s3-boy-doom-combined.bin
