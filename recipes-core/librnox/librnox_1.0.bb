#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "Library for logging and messages queueing"
SECTION = "library"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "jansson"

#RDEPENDS_${PN} = "libdigiapix"

INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"

# This tells bitbake where to find the files we're providing on the local filesystem
#FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

S = "${WORKDIR}"

SRC_URI = "\
	file://log.c \
	file://log.h \
	file://queue.c \
	file://queue.h \
"

FILES_${PN} = "${libdir}/*.so"

PACKAGES = "${PN} ${PN}-dev ${PN}-dbg ${PN}-staticdev"

RDEPENDS_${PN}-staticdev = ""
RDEPENDS_${PN}-dev = ""
RDEPENDS_${PN}-dbg = ""

do_compile() {
	${CC} -c -fpic queue.c log.c
	${CC} -shared -o ${PN}-${PV}.so log.o queue.o -ljansson 
}

do_install() {
	 install -d ${D}${libdir}
	 install -m 0644 ${PN}-${PV}.so ${D}${libdir}
}
