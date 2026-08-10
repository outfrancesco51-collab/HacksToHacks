/**
 * main_wiiu.c — HacksToHacks Wii U Port
 *
 * Wii U stub using WUT (Wii U Toolchain) + SDL2 (libSDL-wiiu).
 * Displays a styled main menu on the Wii U GamePad and TV screens.
 *
 * Built with devkitPro powerpc-eabi toolchain and wut library.
 * Target: Wii U .rpx homebrew application
 *
 * Controls:
 *   A Button     → Start Game (shows "Coming Soon" message)
 *   HOME Button  → Exit to Wii U Menu
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_udp.h>
#include <vpad/input.h>

#include <stdio.h>
#include <string.h>

// ─── Color palette ───────────────────────────────────────────────────────────
#define COL_BG       {  5,   5,   5, 255 }
#define COL_GREEN    {  0, 255,  65, 255 }
#define COL_DKGREEN  {  0, 143,  17, 255 }
#define COL_WHITE    {255, 255, 255, 255 }
#define COL_GREY     {100, 100, 100, 255 }

// ─── Screen config ────────────────────────────────────────────────────────────
// Wii U TV: 1920×1080 (or 1280×720 scaled)
// GamePad:  854×480
#define TV_W    1280
#define TV_H     720
#define PAD_W    854
#define PAD_H    480

// ─── Helpers ──────────────────────────────────────────────────────────────────
static void draw_text(SDL_Renderer *r, TTF_Font *font, const char *text,
                      int x, int y, SDL_Color col)
{
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FreeSurface(surf);
    if (!tex) return;
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    SDL_Rect dst = { x - w / 2, y - h / 2, w, h };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

static void draw_scanlines(SDL_Renderer *r, int sw, int sh)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 40);
    for (int y = 0; y < sh; y += 4) {
        SDL_RenderDrawLine(r, 0, y, sw, y);
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

static void render_menu(SDL_Renderer *r, TTF_Font *font_big,
                        TTF_Font *font_med, TTF_Font *font_sm,
                        int sw, int sh, int frame, int msg_timer)
{
    // Background
    SDL_SetRenderDrawColor(r, 5, 5, 5, 255);
    SDL_RenderClear(r);

    // Pulsing green border
    int pulse = (int)(20 + 15 * SDL_sinf((float)frame * 0.05f));
    SDL_SetRenderDrawColor(r, 0, 255, 65, (Uint8)pulse * 5 > 255 ? 255 : pulse * 5);
    SDL_Rect border = { 10, 10, sw - 20, sh - 20 };
    SDL_RenderDrawRect(r, &border);

    SDL_Color green   = { 0, 255,  65, 255 };
    SDL_Color dkgreen = { 0, 143,  17, 255 };
    SDL_Color white   = {255, 255, 255, 255 };
    SDL_Color grey    = {100, 100, 100, 255 };

    // Title
    draw_text(r, font_big, "HACKS TO HACKS", sw / 2, sh / 4,     green);
    draw_text(r, font_sm,  "[ WII U EDITION ]",  sw / 2, sh / 4 + 60, dkgreen);

    // Divider line
    SDL_SetRenderDrawColor(r, 0, 143, 17, 200);
    SDL_RenderDrawLine(r, sw / 4, sh / 2 - 40, 3 * sw / 4, sh / 2 - 40);

    // Menu items
    draw_text(r, font_med, "[ AVVIA GIOCO ]", sw / 2, sh / 2 + 10,  green);
    draw_text(r, font_sm,  "Premi A per iniziare", sw / 2, sh / 2 + 55, white);

    // Coming soon / status message
    if (msg_timer > 0) {
        SDL_Color yellow = { 255, 230, 0, 255 };
        draw_text(r, font_med, ">> GIOCO IN ARRIVO <<", sw / 2, sh * 3 / 4, yellow);
        draw_text(r, font_sm,  "La versione completa e' in sviluppo!", sw / 2, sh * 3 / 4 + 45, grey);
    }

    // Footer
    draw_text(r, font_sm, "HOME = Esci al Menu Wii U", sw / 2, sh - 40, grey);

    // Scanlines effect
    draw_scanlines(r, sw, sh);

    SDL_RenderPresent(r);
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main(int argc, char *argv[])
{
    // WUT process init
    WHBProcInit();
    WHBLogUdpInit();
    WHBLogPrint("[HacksToHacks] Wii U boot");

    // SDL init
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        WHBLogPrintf("[HacksToHacks] SDL_Init error: %s", SDL_GetError());
        WHBProcShutdown();
        return 1;
    }
    if (TTF_Init() < 0) {
        WHBLogPrintf("[HacksToHacks] TTF_Init error: %s", TTF_GetError());
        SDL_Quit();
        WHBProcShutdown();
        return 1;
    }

    // On Wii U, SDL2 creates two windows automatically:
    //   index 0 → TV screen
    //   index 1 → GamePad screen
    // We create TV window; GamePad appears automatically via WUT patch.
    SDL_Window *tv_win = SDL_CreateWindow("HacksToHacks",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        TV_W, TV_H,
        SDL_WINDOW_SHOWN);

    SDL_Renderer *tv_rend = SDL_CreateRenderer(tv_win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Load font from romfs / bundle (fallback to system)
    TTF_Font *font_big = TTF_OpenFont("romfs:/font/SourceCodePro-Bold.ttf", 52);
    TTF_Font *font_med = TTF_OpenFont("romfs:/font/SourceCodePro-Bold.ttf", 32);
    TTF_Font *font_sm  = TTF_OpenFont("romfs:/font/SourceCodePro-Regular.ttf", 20);

    // Fallback fonts if bundled font is missing
    if (!font_big) font_big = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 52);
    if (!font_med) font_med = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 32);
    if (!font_sm)  font_sm  = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",     20);

    int frame      = 0;
    int msg_timer  = 0;  // frames to show "coming soon" message
    VPADStatus vpad;
    VPADReadError vpad_err;

    WHBLogPrint("[HacksToHacks] Entering main loop");

    while (WHBProcIsRunning()) {
        // Read GamePad input
        VPADRead(VPAD_CHAN_0, &vpad, 1, &vpad_err);
        if (vpad_err == VPAD_READ_SUCCESS) {
            if (vpad.trigger & VPAD_BUTTON_A) {
                msg_timer = 180; // show message for 3 seconds @ 60fps
                WHBLogPrint("[HacksToHacks] A pressed → showing Coming Soon");
            }
        }

        // SDL events (handles HOME button via SDL_QUIT)
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) goto cleanup;
        }

        if (msg_timer > 0) msg_timer--;

        // Render TV screen
        render_menu(tv_rend, font_big, font_med, font_sm,
                    TV_W, TV_H, frame, msg_timer);

        frame++;
    }

cleanup:
    WHBLogPrint("[HacksToHacks] Shutting down");
    if (font_sm)  TTF_CloseFont(font_sm);
    if (font_med) TTF_CloseFont(font_med);
    if (font_big) TTF_CloseFont(font_big);
    SDL_DestroyRenderer(tv_rend);
    SDL_DestroyWindow(tv_win);
    TTF_Quit();
    SDL_Quit();
    WHBLogUdpDeinit();
    WHBProcShutdown();
    return 0;
}
