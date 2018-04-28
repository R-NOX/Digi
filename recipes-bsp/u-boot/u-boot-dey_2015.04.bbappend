# This tells bitbake where to find the patches on the local filesystem
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

SRC_URI += "\
	file://0001-added-install-rnox-image.patch \
"
