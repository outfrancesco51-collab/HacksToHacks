/**
 * platform_sdl3.c — SDL3 backend adapter for HacksToHacks
 *
 * This file re-implements a minimal raylib-compatible surface using SDL3
 * and OpenGL, so main.c compiles unchanged on Linux with SDL3 instead of
 * the native raylib window backend.
 *
 * Compile with: -DPLATFORM_SDL3 and link -lSDL3 -lGL
 *
 * The real raylib library is still linked for its drawing, text, audio, and
 * asset-loading utilities; only the window/input layer is shimmed here via
 * SDL3's window management.
 *
 * NOTE: On SDL3 builds raylib's own window creation is skipped by defining
 * SUPPORT_CUSTOM_FRAME_CONTROL and providing our own loop management.
 */

#if defined(PLATFORM_SDL3)

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
static SDL_Window   *sdl_window   = NULL;
static SDL_GLContext sdl_glctx    = NULL;
static int           sdl_running  = 1;

// ---------------------------------------------------------------------------
// Thin helpers called by main.c at startup / shutdown
// ---------------------------------------------------------------------------

/**
 * SDL3_PlatformInit
 * Called from main() before InitWindow() when PLATFORM_SDL3 is defined.
 * Creates the SDL3 window and an OpenGL 3.3 context that raylib will use.
 */
int SDL3_PlatformInit(const char *title, int w, int h)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        fprintf(stderr, "[SDL3] SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    sdl_window = SDL_CreateWindow(title, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!sdl_window) {
        fprintf(stderr, "[SDL3] SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    sdl_glctx = SDL_GL_CreateContext(sdl_window);
    if (!sdl_glctx) {
        fprintf(stderr, "[SDL3] SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return 0;
    }

    SDL_GL_MakeCurrent(sdl_window, sdl_glctx);
    SDL_GL_SetSwapInterval(1); // vsync

    printf("[SDL3] Window created (%dx%d) — SDL %s\n",
           w, h, SDL_GetVersion());
    return 1;
}

/**
 * SDL3_PlatformPollEvents
 * Pumps SDL3 events so the OS doesn't think the window is frozen.
 * Returns 0 when the user closes the window.
 */
int SDL3_PlatformPollEvents(void)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_EVENT_QUIT) {
            sdl_running = 0;
        }
        if (ev.type == SDL_EVENT_KEY_DOWN &&
            ev.key.scancode == SDL_SCANCODE_ESCAPE) {
            sdl_running = 0;
        }
    }
    return sdl_running;
}

/**
 * SDL3_PlatformSwapBuffers
 * Swaps the OpenGL front/back buffers (replaces EndDrawing's swap call).
 */
void SDL3_PlatformSwapBuffers(void)
{
    SDL_GL_SwapWindow(sdl_window);
}

/**
 * SDL3_PlatformShutdown
 * Cleans up SDL3 resources. Called from main() after the game loop exits.
 */
void SDL3_PlatformShutdown(void)
{
    if (sdl_glctx)  SDL_GL_DestroyContext(sdl_glctx);
    if (sdl_window) SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

#endif /* PLATFORM_SDL3 */
