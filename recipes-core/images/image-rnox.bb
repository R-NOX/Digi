# R-NOX image
# Settings are taken from "poky/meta/recipes-core/images/core-image-base.bb" 
# and "meta-digi/meta-digi-dey/recipes-core/images/core-image-base.bbappend"

LICENSE = "MIT"

inherit core-image

IMAGE_FEATURES += " \
    dey-network \
    eclipse-debug \
    package-management \
    ssh-server-dropbear \
    ${@bb.utils.contains('MACHINE_FEATURES', 'accel-video', 'dey-gstreamer', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'alsa', 'dey-audio', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'bluetooth', 'dey-bluetooth', '', d)} \
    ${@bb.utils.contains('MACHINE_FEATURES', 'wifi', 'dey-wireless', '', d)} \
"        
         
# SDK features (for toolchains generated from an image with populate_sdk)
SDKIMAGE_FEATURES ?= "dev-pkgs dbg-pkgs staticdev-pkgs"

# Add dey-image tweaks to the final image (like /etc/build info)
inherit dey-image

# Do not install udev-cache
BAD_RECOMMENDATIONS += "udev-cache"

IMAGE_INSTALL += "helloworld install-rnox-scr install-rnox-scr"

#require recipes-core/install-rnox-scr/install-rnox-scr_1.0.bb