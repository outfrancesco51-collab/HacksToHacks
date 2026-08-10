import re

with open('.github/workflows/release.yml', 'r', encoding='utf-8') as f:
    content = f.read()

# Fix windows compilation
content = re.sub(
    r'gcc -O2 -Wall -Isrc -I\$RL_SRC -c src/cutscene\.c -o out-windows/cutscene\.o',
    r'gcc -O2 -Wall -Isrc -I$RL_SRC -c src/cutscene.c -o out-windows/cutscene.o\n          gcc -O2 -Wall -Isrc -I$RL_SRC -c src/minigame_fingerprint.c -o out-windows/minigame_fingerprint.o\n          gcc -O2 -Wall -Isrc -I$RL_SRC -c src/minigame_wires.c -o out-windows/minigame_wires.o',
    content
)

content = re.sub(
    r'out-windows/city3d\.o out-windows/cutscene\.o \\',
    r'out-windows/city3d.o out-windows/cutscene.o \\\n              out-windows/minigame_fingerprint.o out-windows/minigame_wires.o \\',
    content
)

with open('.github/workflows/release.yml', 'w', encoding='utf-8') as f:
    f.write(content)
