FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}:"

SRC_URI += " \
    file://openvpn.conf \
"

inherit update-rc.d

INITSCRIPT_NAME = "${PN}"
INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_PARAMS = "start 90 5 2 . stop 30 0 1 6 ."

do_install_append() {
  install -d ${D}${sysconfdir}/openvpn
  install -m 644 ${WORKDIR}/openvpn.conf ${D}${sysconfdir}/openvpn/
}
