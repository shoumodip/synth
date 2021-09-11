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

    size_t octave = 40;
    while (running) {

        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT: running = false; break;
            case SDL_KEYDOWN: SDL_PauseAudioDevice(id, 0);
                switch (e.key.keysym.sym) {
                case SDLK_a: freq = get_note(octave + 0); break;
                case SDLK_w: freq = get_note(octave + 1); break;
                case SDLK_s: freq = get_note(octave + 2); break;
                case SDLK_e: freq = get_note(octave + 3); break;
                case SDLK_d: freq = get_note(octave + 4); break;
                case SDLK_f: freq = get_note(octave + 5); break;
                case SDLK_u: freq = get_note(octave + 6); break;
                case SDLK_j: freq = get_note(octave + 7); break;
                case SDLK_i: freq = get_note(octave + 8); break;
                case SDLK_k: freq = get_note(octave + 9); break;
                case SDLK_o: freq = get_note(octave + 10); break;
                case SDLK_l: freq = get_note(octave + 11); break;

                case SDLK_0: octave = 0; break;
                case SDLK_1: octave = 10; break;
                case SDLK_2: octave = 20; break;
                case SDLK_3: octave = 30; break;
                case SDLK_4: octave = 40; break;
                case SDLK_5: octave = 50; break;
                case SDLK_6: octave = 60; break;
                case SDLK_7: octave = 70; break;
                case SDLK_8: octave = 80; break;
                case SDLK_9: octave = 90; break;

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
