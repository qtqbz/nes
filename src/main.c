#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "arena.h"
#include "str8.h"
#include "nes.h"

#define FPS 60

typedef struct SdlResources SdlResources;
struct SdlResources
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* buffer;
};

internal void
sdl_abort_on_error(int32_t result)
{
    if (result < 0) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
}

internal void *
sdl_abort_on_null(void *p)
{
    if (p == NULL) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
    return p;
}

internal SdlResources
sdl_create()
{
    sdl_abort_on_error(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window = (SDL_Window *)sdl_abort_on_null(SDL_CreateWindow("nes emulator by qtqbz",
                                                                          NES_DISPLAY_WIDTH_PX,
                                                                          NES_DISPLAY_HEIGHT_PX,
                                                                          SDL_WINDOW_RESIZABLE));

    SDL_Renderer *renderer = (SDL_Renderer *)sdl_abort_on_null(SDL_CreateRenderer(window, NULL));

    sdl_abort_on_error(SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND));

    SDL_Texture *buffer = (SDL_Texture *)sdl_abort_on_null(SDL_CreateTexture(renderer,
                                                                             SDL_PIXELFORMAT_RGBA8888,
                                                                             SDL_TEXTUREACCESS_STREAMING,
                                                                             NES_DISPLAY_WIDTH_PX,
                                                                             NES_DISPLAY_HEIGHT_PX));
    SdlResources result = {};
    result.window = window;
    result.renderer = renderer;
    result.buffer = buffer;
    return result;
}

internal void
sdl_free(SdlResources *sdl)
{
    SDL_DestroyTexture(sdl->buffer);
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}

int32_t
main(int32_t argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s ROM\n", argv[0]);
        exit(1);
    }
    Str8 romPath = str8_from_cstr(argv[1]);

    int32_t arenaBufCap = MB(64);
    uint8_t *arenaBuf = (uint8_t *)malloc(arenaBufCap);
    Arena permArena = arena_make(arenaBuf, arenaBufCap);

    Nes nes = {};

    Rom *rom = &nes.rom;
    if (!rom_load(&permArena, rom, romPath)) {
        fprintf(stderr, "Failed to load ROM: %.*s\n", STR8_VARG(romPath));
        exit(1);
    }

    Mmu *mmu = &nes.mmu;
    mmu->rom = rom;

    Cpu *cpu = &nes.cpu;
    cpu->mmu = mmu;
    cpu_init(cpu);

    Ppu *ppu = &nes.ppu;
    ppu->mmu = mmu;

    uint64_t targetFrameDurationMs = 1000 / FPS;

    SdlResources sdl = sdl_create();

    bool quit = false;
    while (!quit) {
        uint64_t startMs = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    quit = true;
                } break;
                default: {
                    // TODO
                }
            }
        }

        uint32_t *pixels;
        int32_t pitch;
        sdl_abort_on_error(SDL_LockTexture(sdl.buffer, NULL, (void **)&pixels, &pitch));

        ArenaBackup arenaBck = arena_backup(&permArena);
        nes_display_update(arenaBck.arena, &nes, pixels);
        arena_restore(&arenaBck);

        SDL_UnlockTexture(sdl.buffer);

        SDL_RenderClear(sdl.renderer);
        SDL_RenderTexture(sdl.renderer, sdl.buffer, NULL, NULL);
        SDL_RenderPresent(sdl.renderer);

        uint64_t endMs = SDL_GetTicks();

        uint64_t durationMs = endMs - startMs;
        if (durationMs < targetFrameDurationMs) {
            SDL_DelayNS(SDL_MS_TO_NS(targetFrameDurationMs - durationMs));
        } else {
            fprintf(stderr, "Missed a frame by %lu ms\n", durationMs);
        }
    }

    sdl_free(&sdl);
    free(arenaBuf);

    return 0;
}
