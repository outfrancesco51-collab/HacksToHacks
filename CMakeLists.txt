cmake_minimum_required(VERSION 3.11)
project(HacksToHacks VERSION 2.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(DOWNLOAD_EXTRACT_TIMESTAMP ON)

include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(HacksToHacks
    src/main.c
    src/cJSON.c
    src/tween.c
    src/city3d.c
    src/cutscene.c
    src/minigame_fingerprint.c
    src/minigame_wires.c
)

target_include_directories(HacksToHacks PRIVATE src)
target_link_libraries(HacksToHacks PRIVATE raylib)

if(ANDROID)
    target_link_libraries(HacksToHacks PRIVATE log android EGL GLESv2 OpenSLES m)
endif()

# Copy assets to build output directory
file(COPY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY "${CMAKE_SOURCE_DIR}/locales" DESTINATION "${CMAKE_BINARY_DIR}")

# For iOS/macOS app bundle: copy assets into the bundle at install time
if(APPLE)
    set_target_properties(HacksToHacks PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "HacksToHacks"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.hacksToHacks.game"
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    )

    # Copy assets into the .app bundle
    file(GLOB ASSET_FILES "${CMAKE_SOURCE_DIR}/assets/*")
    target_sources(HacksToHacks PRIVATE ${ASSET_FILES})
    set_source_files_properties(${ASSET_FILES} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/assets"
    )

    file(GLOB LOCALE_FILES "${CMAKE_SOURCE_DIR}/locales/*")
    target_sources(HacksToHacks PRIVATE ${LOCALE_FILES})
    set_source_files_properties(${LOCALE_FILES} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/locales"
    )
endif()
