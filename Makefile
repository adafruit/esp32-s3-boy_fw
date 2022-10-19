SHELL=/bin/bash
.PHONY: default
default:
	. esp-idf/export.sh && cd fw/s3-boy-doom && idf.py set-target esp32s3 && idf.py build && cd build && esptool.py --chip esp32s3 merge_bin -t 0 --output s3-boy-doom-combined.bin `tail -n +2 flash_args` 0x110000 ../../../assets/doom.squashfs
