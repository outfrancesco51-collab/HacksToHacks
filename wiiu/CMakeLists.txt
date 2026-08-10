cmake_minimum_required(VERSION 3.19)
project(HacksToHacks_WiiU C)

# ─── devkitPro / WUT toolchain ────────────────────────────────────────────────
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

target_compile_options(HacksToHacks_WiiU PRIVATE
    -O2
    -mcpu=750
    -meabi
    -mhard-float
    -D__WIIU__
    -D__WUT__
)

target_link_directories(HacksToHacks_WiiU PRIVATE ${WIIU_LIB} ${WUT_LIB})

target_link_libraries(HacksToHacks_WiiU PRIVATE
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
