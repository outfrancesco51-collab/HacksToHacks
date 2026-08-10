import re

with open('.github/workflows/build.yml', 'r') as f:
    content = f.read()

# Fix emcc command
content = re.sub(
    r'emcc -o build_web/index\.html.*?-DPLATFORM_WEB',
    'emcc src/main.c src/cJSON.c src/tween.c src/city3d.c src/cutscene.c src/minigame_fingerprint.c src/minigame_wires.c -o build_web/index.html -Os -Wall raylib-src/src/libraylib.a -Iraylib-src/src -Lraylib-src/src -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 --preload-file assets --preload-file locales -DPLATFORM_WEB',
    content
)

# Fix gcc windows command
content = re.sub(
    r'x86_64-w64-mingw32-gcc src/main\.c src/cJSON\.c src/tween\.c src/city3d\.c src/cutscene\.c(.*?) -o build_windows',
    r'x86_64-w64-mingw32-gcc src/main.c src/cJSON.c src/tween.c src/city3d.c src/cutscene.c src/minigame_fingerprint.c src/minigame_wires.c\1 -o build_windows',
    content
)

with open('.github/workflows/build.yml', 'w') as f:
    f.write(content)
