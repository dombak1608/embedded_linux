#!/bin/sh

set -u
set -e

# Add a console on tty1
if [ -e ${TARGET_DIR}/etc/inittab ]; then
    grep -qE '^tty1::' ${TARGET_DIR}/etc/inittab || \
	sed -i '/GENERIC_SERIAL/a\
tty1::respawn:/sbin/getty -L  tty1 0 vt100 # HDMI console' ${TARGET_DIR}/etc/inittab
fi

# Change config.txt to boot u-boot.bin instead of zImage
sed -i -e '/.*kernel=.*/c\
kernel=u-boot.bin' ${BINARIES_DIR}/rpi-firmware/config.txt

if ! grep -Fxq "enable_uart=1" ${BINARIES_DIR}/rpi-firmware/config.txt
then
    echo "enable_uart=1" >> ${BINARIES_DIR}/rpi-firmware/config.txt
fi
# Replace previous line with the following one in order to change uart0 clock and baud
# (used as workarround with previous linux kernel versions when DTS was using 3MHz clock
# and firmware overriden it to 48MHz as the new firmware realy sets uart0 clock to 48MHz,
# but overriding was not working through u-boot. now it is set in DTS to 48MHz)
#kernel=u-boot.bin\ninit_uart_clock=3000000\ninit_uart_baud=115200' ${BINARIES_DIR}/rpi-firmware/config.txt

# Change profile to print path
sed -i '/export PS1='"'"'\# '"'"'.*/c\
		export PS1="\\\`if \[\[ \\\$? = "0" ]]; then echo '"'"'\\e\[32m\\h\\e\[0m'"'"'; else echo '"'"'\\e\[31m\\h\\e\[0m'"'"' ; fi\\\`:\\\w\\\# "' ${TARGET_DIR}/etc/profile

sed -i '/export PS1='"'"'\$ '"'"'.*/c\
		export PS1="\\\`if \[\[ \\\$? = "0" ]]; then echo '"'"'\\e\[32m\\h\\e\[0m'"'"'; else echo '"'"'\\e\[31m\\h\\e\[0m'"'"' ; fi\\\`:\\\w\\\$ "' ${TARGET_DIR}/etc/profile

# Change sshd_config for SSH server
sed -i '/.*PermitRootLogin.*/c\
PermitRootLogin yes' ${TARGET_DIR}/etc/ssh/sshd_config

sed -i '/.*PermitEmptyPasswords.*/c\
PermitEmptyPasswords yes' ${TARGET_DIR}/etc/ssh/sshd_config
