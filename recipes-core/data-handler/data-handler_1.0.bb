#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "data-handler application"
SECTION = "app"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "librnox"

RDEPENDS_${PN} = "librnox"

INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"

S = "${WORKDIR}"

SRC_URI = "\
	file://main.c \
"

FILES_${PN} = "${bindir}/*"

do_compile() {
	${CC} -c main.c
	${CC} -o ${PN} main.o -lrnox
}

do_install() {
	 install -d ${D}${bindir}
	 install -m 0755 ${PN} ${D}${bindir}
}