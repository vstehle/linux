#!/bin/bash
set -eux

J=$(nproc)

[ -e .config ] || make -j$J defconfig
[ -e arch/arm64/boot/Image ] || make -j$J Image dtbs

rm -vf zero vmlinuz.uimg vmlinux.packed

mkimage \
	-D "-I dts -O dtb" \
	-f rk3399-gru-kevin.its \
	-p 2048 \
	vmlinuz.uimg

dd if=/dev/zero of=zero bs=512 count=1

vbutil_kernel \
	--arch aarch64 \
	--bootloader zero \
	--config cmdline \
	--keyblock /usr/share/vboot/devkeys/kernel.keyblock \
	--pack vmlinux.packed \
	--signprivate /usr/share/vboot/devkeys/kernel_data_key.vbprivk \
	--version 1 \
	--vmlinuz vmlinuz.uimg
