DESCRIPTION = "Bootloader for R-NOX"
SECTION = "boot"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS += "dtc-native u-boot-mkimage-native u-boot-mkimage"
#DEPENDS += "${@base_conditional('TRUSTFENCE_SIGN', '1', 'trustfence-sign-tools-native', '', d)}"

#PROVIDES += "u-boot"

#SRCBRANCH = "v2015.04/maint"
#SRCREV = "5ea79bed5143fc47e5b11af2d44a04bab9af4092"

inherit deploy
addtask deploy after compile

#inherit autotools

# This tells bitbake where to find the files we're providing on the local filesystem
FILESPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

SRC_URI = "file://install-rnox-scr_1.0.txt"

S = "${WORKDIR}"

FILES_${PN} = "install-rnox-scr_1.0.txt"
ALLOW_EMPTY_${PN} = "1"
FILES_SOLIBSDEV = ""

do_deploy() {
	# R-NOX firmware install script
	mkimage -T script -n "R-NOX firmware install script" -C none -d ${WORKDIR}/install-rnox-scr_1.0.txt ${DEPLOYDIR}/install_rnox_fw_sd.scr
}

#do_install() {
#<------>install -d ${D}${bindir}/
#<------>install -m 0755 install-rnox-scr_1.0.txt ${D}${bindir}/
#}


#COMPATIBLE_MACHINE = "(ccimx6$|ccimx6ul)"
