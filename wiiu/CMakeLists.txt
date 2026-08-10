cmake_minimum_required(VERSION 3.19)
project(HacksToHacks_WiiU C)

# ─── devkitPro / WUT toolchain ────────────────────────────────────────────────
# This file is configured by powerpc-eabi-cmake (WUT cmake wrapper).
# The toolchain sets:
#   CMAKE_SYSTEM_NAME  = "Generic"
#   DEVKITPRO          = /opt/devkitpro
#   CMAKE_C_COMPILER   = powerpc-eabi-gcc

if(NOT DEFINED DEVKITPRO)
    message(FATAL_ERROR "DEVKITPRO not set. Run with powerpc-eabi-cmake.")
endif()

set(WIIU_LIB "${DEVKITPRO}/portlibs/wiiu/lib")
set(WIIU_INC "${DEVKITPRO}/portlibs/wiiu/include")
set(WUT_LIB  "${DEVKITPRO}/wut/lib")
set(WUT_INC  "${DEVKITPRO}/wut/include")

# ─── Source files ─────────────────────────────────────────────────────────────
add_executable(HacksToHacks_WiiU
    main_wiiu.c
)

# ─── Include paths ────────────────────────────────────────────────────────────
target_include_directories(HacksToHacks_WiiU PRIVATE
    ${WUT_INC}
    ${WIIU_INC}
    ${WIIU_INC}/SDL2
)

# ─── Compiler flags ───────────────────────────────────────────────────────────
target_compile_options(HacksToHacks_WiiU PRIVATE
    -O2
    -mcpu=750
    -meabi
    -mhard-float
    -D__WIIU__
    -D__WUT__
)

# ─── Link libraries ───────────────────────────────────────────────────────────
# Order matters for static linking:
# SDL2 → SDL2_ttf → wut (provides WUT runtime + system calls)
target_link_libraries(HacksToHacks_WiiU PRIVATE
    ${WIIU_LIB}/libSDL2.a
    ${WIIU_LIB}/libSDL2_ttf.a
    ${WIIU_LIB}/libfreetype.a
    ${WIIU_LIB}/libbz2.a
    ${WIIU_LIB}/libpng.a
    ${WIIU_LIB}/libz.a
    ${WUT_LIB}/libwut.a
    ${WIIU_LIB}/libvpad.a
    ${WIIU_LIB}/libwhb.a
)

# ─── WUT helpers → generates .rpx from ELF ───────────────────────────────────
# The wut_create_rpx helper is automatically defined by WiiU.cmake toolchain
wut_create_rpx(HacksToHacks_WiiU)

# Install target: copy .rpx to output/
install(
    FILES "${CMAKE_BINARY_DIR}/HacksToHacks_WiiU.rpx"
    DESTINATION "${CMAKE_BINARY_DIR}/wiiu-dist"
)
