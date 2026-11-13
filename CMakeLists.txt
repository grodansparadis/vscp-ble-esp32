# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# vscp-firmware sources, fetched at configure time (they used to be the
# third-party/vscp-firmware git submodule; same commit). Use
# -DFETCHCONTENT_SOURCE_DIR_VSCP_FIRMWARE=<path> for a local checkout.
include(FetchContent)
FetchContent_Declare(
    vscp_firmware
    URL https://codeload.github.com/grodansparadis/vscp-firmware/tar.gz/ab829cdeee687fedbecd03dd49305c669742e137
    URL_HASH SHA256=850340363f2c0ace73f2f139d891ad415e94c08307f3da069c2d85150df03085
    SOURCE_SUBDIR __sources_only__
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(vscp_firmware)
set(VSCP_FIRMWARE_DIR "${vscp_firmware_SOURCE_DIR}" CACHE INTERNAL "vscp-firmware sources")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ble-example)
