SUMMARY = "bme-sensor application"
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
	file://bme280.c \
	file://bme280.h \
	file://bme280_defs.h \
	file://bme-sensor.sh \
"

inherit update-rc.d

INITSCRIPT_NAME = "bme-sensor.sh"
INITSCRIPT_PARAMS = "start 62 5 3 2 . stop 70 0 1 6 ."

FILES_${PN} = "${bindir}/* \
			   ${sysconfdir} \
"

do_compile() {
	${CC} -c main.c bme280.c -DBME280_FLOAT_ENABLE
	${CC} -o ${PN} main.o bme280.o -ldigiapix -lrnox
}

do_install() {
	 install -d ${D}${bindir}
	 install -m 0755 ${PN} ${D}${bindir}

  	 install -d ${D}${sysconfdir} \
 				${D}${sysconfdir}/init.d

	 install -m 0755 ${WORKDIR}/bme-sensor.sh ${D}${sysconfdir}/init.d
}
