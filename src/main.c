#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#define PI2 6.28318530718

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
        time += freq * PI2 / 48000.0;
        if(time >= PI2) time -= PI2;
    }
}

// See https://en.wikipedia.org/wiki/Piano_key_frequencies
double notes[] = {
    ['c'] = 261.625,
    ['d'] = 293.724,
    ['e'] = 329.724,
    ['f'] = 349.309,
    ['g'] = 392.089,
    ['a'] = 440.000,
    ['b'] = 493.858
};

int main(int argc, char **argv)
{
    assert(argc == 2);
    char *input = argv[1];

    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec spec, aspec;
    SDL_zero(spec);
    spec.freq = 48000;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 4096;
    spec.callback = callback;
    spec.userdata = NULL;

    // Open audio
    int id;
    if ((id = SDL_OpenAudioDevice(NULL, 0, &spec, &aspec, SDL_AUDIO_ALLOW_ANY_CHANGE)) <= 0) {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        exit(-1);
    }

    // Start playing
    SDL_PauseAudioDevice(id, 0);

    while(*input != '\0') {
        int note = tolower(*input++);
        int ms = strtol(input, &input, 10);

        freq = notes[note];
        SDL_Delay(ms);
    }
}
