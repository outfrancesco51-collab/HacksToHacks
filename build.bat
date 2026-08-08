@echo off
echo Creazione directory bin...
if not exist bin mkdir bin

echo Compilazione in corso...
gcc -Wall -Iraylib/include src/main.c -o bin/HacksToHacks.exe -Lraylib/lib -lraylib -lgdi32 -lwinmm

if %ERRORLEVEL% == 0 (
    echo Compilazione completata con successo!
    echo Eseguibile: bin\HacksToHacks.exe
) else (
    echo Errore durante la compilazione.
)
pause
