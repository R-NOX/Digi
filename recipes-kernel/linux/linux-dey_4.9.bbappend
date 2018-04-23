# This tells bitbake where to find the patches on the local filesystem
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

SRC_URI += "\
	file://0001-enabling-PWM2-PWM3-PWM4.patch \
	file://0002-deleted-setting-class-for-exported-channels.patch \
"
