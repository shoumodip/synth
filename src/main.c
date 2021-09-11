#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#define TAU 6.28318530718

float time = 0;
float freq = 440;

// Stolen from https://gist.github.com/amirrajan/fa6ce9fdc8918e06ca9759c3358e4cd2
void callback(void* userdata, Uint8* stream, int len)
{
    (void) userdata;

    short *snd = (short *) stream;
    len /= sizeof(*snd);
    for(int i = 0; i < len; i++) {
        snd[i] = 32000 * sin(time);
        time += freq * TAU / 48000.0;
        if(time >= TAU) time -= TAU;
    }
}

// See https://en.wikipedia.org/wiki/Piano_key_frequencies
static inline float get_note(int n)
{
    return powf(2, (n - 49) / 12.0) * 440;
}

static inline int scc(int c)
{
    if (c < 0) {
        fprintf(stderr, "error: %s\n", SDL_GetError());
        exit(1);
    }
    return c;
}

static inline void *scp(void *p)
{
    if (p == NULL) {
        fprintf(stderr, "error: %s\n", SDL_GetError());
        exit(1);
    }
    return p;
}

int main(void)
{
    scc(SDL_Init(SDL_INIT_VIDEO));
    SDL_Window *window = scp(SDL_CreateWindow("Synth",
                                  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  640, 480, SDL_WINDOW_SHOWN));

    SDL_Surface *surface = SDL_GetWindowSurface(window);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0x2B, 0x2B, 0x2B));
    SDL_UpdateWindowSurface(window);

    scc(SDL_Init(SDL_INIT_AUDIO));
    SDL_AudioSpec spec, aspec;
    SDL_zero(spec);
    spec.freq = 48000;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 4096;
    spec.callback = callback;
    spec.userdata = NULL;

    // Open audio
    int id = scc(SDL_OpenAudioDevice(NULL, 0, &spec, &aspec, SDL_AUDIO_ALLOW_ANY_CHANGE));

    SDL_Event e;
    bool running = true;

    size_t offset = 40;
    while (running) {

        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT: running = false; break;
            case SDL_KEYDOWN: SDL_PauseAudioDevice(id, 0);
                switch (e.key.keysym.sym) {
                case SDLK_a: freq = get_note(offset + 0); break;
                case SDLK_s: freq = get_note(offset + 2); break;
                case SDLK_d: freq = get_note(offset + 4); break;
                case SDLK_f: freq = get_note(offset + 5); break;
                case SDLK_j: freq = get_note(offset + 7); break;
                case SDLK_k: freq = get_note(offset + 9); break;
                case SDLK_l: freq = get_note(offset + 11); break;
                case SDLK_q: running = false; break;
                default: SDL_PauseAudioDevice(id, 1);
                }
                break;
            case SDL_KEYUP: SDL_PauseAudioDevice(id, 1); break;
            }
        }
    }
    

    SDL_DestroyWindow(window);
    SDL_Quit();
}
