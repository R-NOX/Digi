#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "prs-sensor application"
SECTION = "app"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "libdigiapix librnox"

RDEPENDS_${PN} = "libdigiapix librnox"

INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"

# This tells bitbake where to find the files we're providing on the local filesystem
#FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

S = "${WORKDIR}"

SRC_URI = "\
	file://main.c \
	file://prs-sensor.sh \
"

inherit update-rc.d

INITSCRIPT_NAME = "prs-sensor.sh"
INITSCRIPT_PARAMS = "start 62 5 3 2 . stop 70 0 1 6 ."

FILES_${PN} = "${bindir}/* \
			   ${sysconfdir} \
"

#PACKAGES = "${PN} ${PN}-dev ${PN}-dbg ${PN}-staticdev"

#RDEPENDS_${PN}-staticdev = ""
#RDEPENDS_${PN}-dev = ""
#RDEPENDS_${PN}-dbg = ""

do_compile() {
	${CC} -c main.c
	${CC} -o ${PN} main.o -ldigiapix -lrnox
}

do_install() {
	 install -d ${D}${bindir}
	 install -m 0755 ${PN} ${D}${bindir}

 	 install -d ${D}${sysconfdir} \
	 			${D}${sysconfdir}/init.d

	 install -m 0755 ${WORKDIR}/prs-sensor.sh ${D}${sysconfdir}/init.d
}
