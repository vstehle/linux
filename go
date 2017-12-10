#!/bin/bash
set -eux

export PATH="/usr/lib/ccache:$PATH"

case $(uname -m) in
aarch64) ;;
*)
	export CCACHE_DIR=/exported/armelle/.ccache
	export ARCH=arm64
	export CROSS_COMPILE='ccache aarch64-linux-gnu-'
	;;
esac

J=$(nproc)
[ -e .config ] || make -j"$J" defconfig
make -j"$J" Image dtbs modules

rm -fr install
make -j"$J" modules_install INSTALL_MOD_PATH="$(pwd)/install"

# No need for firmware for now
rm -fr install/lib/firmware

rel="$(make kernelrelease)"
#headers="/usr/src/linux-headers-$rel"
#make -j"$J" headers_install INSTALL_HDR_PATH="$(pwd)/install$headers"

# Fix modules build & source links
(
	cd "install/lib/modules/$rel"
	rm -f build source
#	ln -s "$headers" build
#	ln -s "$headers" source
)

#find . -name Makefile\* -o -name Kconfig\* -o -name \*.pl > hdrsrcfiles
#find arch/arm64/include include scripts -type f >> hdrsrcfiles
#find arch/arm64 -name module.lds -o -name Kbuild.platforms -o -name Platform >> hdrsrcfiles
#find $(find arch/arm64 -name include -o -name scripts -type d) -type f >> hdrsrcfiles
#find arch/arm64/include Module.symvers include scripts -type f > hdrobjfiles
#tar -c -f - -T - < hdrsrcfiles | (cd "install$headers"; tar -xf -)
#tar -c -f - -T - < hdrobjfiles | (cd "install$headers"; tar -xf -)
#cp .config "install$headers"/.config
#rm -f hdrsrcfiles hdrobjfiles

mkimage \
	-D "-I dts -O dtb -p 2048" \
	-f rk3399-gru-kevin.its \
	vmlinux.uimg

dd if=/dev/zero of=zero bs=512 count=1

vbutil_kernel \
	--arch aarch64 \
	--bootloader zero \
	--config cmdline \
	--keyblock /usr/share/vboot/devkeys/kernel.keyblock \
	--pack vmlinux.kpart \
	--signprivate /usr/share/vboot/devkeys/kernel_data_key.vbprivk \
	--version 1 \
	--vmlinuz vmlinux.uimg

mkdir -p install/boot
cp -v vmlinux.kpart install/boot/vmlinux-"$rel".kpart
cp -v vmlinux install/boot/vmlinux-"$rel"

fakeroot sh -c 'set -eux; chown -R root:root install; chmod -R a-s,go-w,u+w,a+rX install; tar -C install -cf install.tar .'

rm -f install.tar.gz
pigz install.tar
