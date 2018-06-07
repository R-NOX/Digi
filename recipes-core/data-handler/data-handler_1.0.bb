#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "data-handler application"
SECTION = "app"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "librnox curl sqlite3"

RDEPENDS_${PN} = "librnox curl sqlite3"

INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"

S = "${WORKDIR}"

SRC_URI = "\
	file://main.c \
	file://post.c \
	file://post.h \
	file://db.h \
	file://db.c \
	file://data-handler.sh \
"

inherit update-rc.d

INITSCRIPT_NAME = "data-handler.sh"
INITSCRIPT_PARAMS = "start 52 5 3 2 . stop 80 0 1 6 ."

FILES_${PN} = "${bindir}/* \
			   ${sysconfdir} \
"

do_compile() {
	${CC} -c main.c post.c db.c
	${CC} -o ${PN} main.o post.o db.o -lrnox -lcurl -lsqlite3
}

do_install() {
	 install -d ${D}${bindir}
	 install -m 0755 ${PN} ${D}${bindir}

 	 install -d ${D}${sysconfdir} \
	 			${D}${sysconfdir}/init.d

	 install -m 0755 ${WORKDIR}/data-handler.sh ${D}${sysconfdir}/init.d
}
