#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "opcn2-sensor application"
SECTION = "app"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "libdigiapix librnox"

RDEPENDS_${PN} = "libdigiapix librnox"

INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"

S = "${WORKDIR}"

SRC_URI = "\
	file://main.c \
	file://opcn2.c \
	file://opcn2.h \
"

FILES_${PN} = "${bindir}/*"

do_compile() {
	${CC} -c main.c opcn2.c
	${CC} -o ${PN} main.o opcn2.o -ldigiapix -lrnox
}

do_install() {
	 install -d ${D}${bindir}
	 install -m 0755 ${PN} ${D}${bindir}
}
