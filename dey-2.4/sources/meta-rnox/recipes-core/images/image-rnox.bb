SUMMARY = "A console-only image that fully supports the target device \
hardware."

LICENSE = "MIT"

IMAGE_FEATURES += "splash"

#IMAGE_INSTALL += "helloworld"

IMAGE_INSTALL_append = " helloworld"

inherit core-image
