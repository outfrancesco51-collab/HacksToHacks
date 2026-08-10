cmake_minimum_required(VERSION 3.19)
project(HacksToHacks_WiiU C)

# ─── devkitPro / WUT toolchain ────────────────────────────────────────────────
if(NOT DEFINED DEVKITPRO)
    message(FATAL_ERROR "DEVKITPRO not set. Run with powerpc-eabi-cmake.")
endif()

# Find packages
find_package(WUT REQUIRED)
# We can just link directly using -l for portlibs, or rely on wiiu-pkg-config.
# For simplicity, let's use the WUT targets and link SDL2 manually if needed.
set(WIIU_LIB "${DEVKITPRO}/portlibs/wiiu/lib")
set(WIIU_INC "${DEVKITPRO}/portlibs/wiiu/include")

# ─── Source files ─────────────────────────────────────────────────────────────
add_executable(HacksToHacks_WiiU
    main_wiiu.c
)

# ─── Include paths ────────────────────────────────────────────────────────────
target_include_directories(HacksToHacks_WiiU PRIVATE
    ${WIIU_INC}
    ${WIIU_INC}/SDL2
)

# ─── Link libraries ───────────────────────────────────────────────────────────
target_link_directories(HacksToHacks_WiiU PRIVATE ${WIIU_LIB})

target_link_libraries(HacksToHacks_WiiU PRIVATE
    WUT::WUT
    -lSDL2_ttf
    -lSDL2
    -lfreetype
    -lbz2
    -lpng
    -lz
    -lm
)

# ─── WUT helpers → generates .rpx from ELF ───────────────────────────────────
wut_create_rpx(HacksToHacks_WiiU)

# Install target: copy .rpx to output/
install(
    FILES "${CMAKE_BINARY_DIR}/HacksToHacks_WiiU.rpx"
    DESTINATION "${CMAKE_BINARY_DIR}/wiiu-dist"
)
