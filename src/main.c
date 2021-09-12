#include <math.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include "wav.h"

#define TAU 6.28318530718
float SDLtime = 0;
float freq = 0;

#define OUTPUT_FILE_FMT "recording-%zu.wav"
Wav wav = {0};

/*
 * The callback for the sound wave in SDL2
 * See https://gist.github.com/amirrajan/fa6ce9fdc8918e06ca9759c3358e4cd2
 *
 * @param userdata *void Not used, present for compilation purposes
 * @param stream Uint8* The sound stream
 * @param len int The length of the stream
 */
void callback(void* userdata, Uint8* stream, int len)
{
    (void) userdata;

    short *snd = (short *) stream;
    len /= sizeof(*snd);
    for (int i = 0; i < len; i++) {
        snd[i] = 32000 * sin(SDLtime);
        if (wav.file) write_sample(&wav, snd[i]);

        SDLtime += freq * TAU / 48000.0;
        if(SDLtime >= TAU) SDLtime -= TAU;
    }
}

/*
 * Get the frequency of a key in a classical piano.
 * See https://en.wikipedia.org/wiki/Piano_key_frequencies
 *
 * @param n int The key to get the frequency of
 * @param float The frequency of the key
 */
float get_freq(int n)
{
    return powf(2, (n - 49) / 12.0) * 440;
}

/*
 * If a float is negative, make it 0
 *
 * @param n float The float to normalize
 * @return float The normalized float
 */
float normalize(float n)
{
    return (n > 0.0) ? n : 0.0;
}

/*
 * Check the exit code of SDL functions which return integers.
 * An exit code less than zero indicates an error
 *
 * @param c int The exit code to check
 * @return int The exit code
 */
int scc(int c)
{
    if (c < 0) {
        fprintf(stderr, "error: %s\n", SDL_GetError());
        exit(1);
    }
    return c;
}

/*
 * Check the value of SDL functions which return pointers.
 * A NULL pointer indicates an error
 *
 * @param p *void The pointer to check
 * @return int The pointer
 */
void *scp(void *p)
{
    if (p == NULL) {
        fprintf(stderr, "error: %s\n", SDL_GetError());
        exit(1);
    }
    return p;
}

/*
 * Check if anything in a bool array is true
 *
 * @param xs *bool The array to check
 * @param count size_t The number of elements to check
 * @return bool Whether a true value was encountered
 */
bool any(bool *xs, size_t count)
{
    for (size_t i = 0; i < count; ++i) if (xs[i]) return true;
    return false;
}

/*
 * Update the window
 *
 * @param window *SDL_Window The window to update
 * @param surface *SDL_Surface The surface of the window
 */
void update(SDL_Window *window, SDL_Surface *surface)
{
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0x2B, 0x2B, 0x2B));
    SDL_UpdateWindowSurface(window);
}

/*
 * End the current recording
 *
 * @param wav *Wav The wav context
 * @param start time_t The starting time 
 */
void end_recording(Wav *wav, time_t start)
{
    size_t duration = time(NULL) - start + 1;
    wav->samples = wav->freq * duration;
    write_wav_hdr(wav);
    fclose(wav->file);
    wav->file = NULL;
}

int main(void)
{
    // Initialize SDL
    scc(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));

    // Setup the window
    SDL_Window *window = scp(SDL_CreateWindow("Synth",
                                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              640, 480, SDL_WINDOW_SHOWN));

    SDL_Surface *surface = SDL_GetWindowSurface(window);
    update(window, surface);

    // Setup the wav
    wav.channels = 1;
    wav.sample_bits = 16;
    wav.freq = 44100;

    // Setup the audio
    SDL_AudioSpec spec, aspec;
    SDL_zero(spec);
    spec.freq = 44100;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 4096;
    spec.callback = callback;
    spec.userdata = NULL;

    int id = scc(SDL_OpenAudioDevice(NULL, 0,
                                     &spec, &aspec,
                                     SDL_AUDIO_ALLOW_ANY_CHANGE));
    SDL_PauseAudioDevice(id, 0);

    // The event loop
    int octave = 40;            // Default in the fourth octave
    bool down[13] = {0};        // The keys down
    bool play = false;          // Whether the sound is playing
    SDL_Event e;                // The event
    bool running = true;        // Whether the application is running
    time_t start;

    while (running) {
        update(window, surface);
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT: running = false; break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                case 'a': // C
                    down[0] = true;
                    freq = normalize(get_freq(octave + 0));
                    break;

                case 'w': // C#
                    down[1] = true;
                    freq = normalize(get_freq(octave + 1));
                    break;

                case 's': // D
                    down[2] = true;
                    freq = normalize(get_freq(octave + 2));
                    break;

                case 'e': // D#
                    down[3] = true;
                    freq = normalize(get_freq(octave + 3));
                    break;

                case 'd': // E
                    down[4] = true;
                    freq = normalize(get_freq(octave + 4));
                    break;

                case 'f': // F
                    down[5] = true;
                    freq = normalize(get_freq(octave + 5));
                    break;

                case 'u': // F#
                    down[6] = true;
                    freq = normalize(get_freq(octave + 6));
                    break;

                case 'j': // G
                    down[7] = true;
                    freq = normalize(get_freq(octave + 7));
                    break;

                case 'i': // G#
                    down[8] = true;
                    freq = normalize(get_freq(octave + 8));
                    break;

                case 'k': // A
                    down[9] = true;
                    freq = normalize(get_freq(octave + 9));
                    break;

                case 'o': // A#
                    down[10] = true;
                    freq = normalize(get_freq(octave + 10));
                    break;

                case 'l': // B
                    down[11] = true;
                    freq = normalize(get_freq(octave + 11));
                    break;

                case 'z': octave = -8; break;
                case 'x': octave = 4; break;
                case 'c': octave = 16; break;
                case 'v': octave = 28; break;
                case 'b': octave = 40; break;
                case 'n': octave = 52; break;
                case 'm': octave = 64; break;
                case ',': octave = 76; break;
                case '.': octave = 88; break;

                case 'q': // Quick the application
                    running = false;
                    break;

                case 'r': // Toggle recording
                    if (wav.file) {
                        end_recording(&wav, start);
                    } else {
                        size_t length = snprintf(NULL, 0, OUTPUT_FILE_FMT, start);
                        char output[length + 1];
                        snprintf(output, length + 1, OUTPUT_FILE_FMT, start);

                        wav.file = fopen(output, "wb");
                        start = time(NULL);
                    }
                }
                break;

            case SDL_KEYUP:
                switch (e.key.keysym.sym) {
                case SDLK_a: down[0] = false; break;
                case SDLK_w: down[1] = false; break;
                case SDLK_s: down[2] = false; break;
                case SDLK_e: down[3] = false; break;
                case SDLK_d: down[4] = false; break;
                case SDLK_f: down[5] = false; break;
                case SDLK_u: down[6] = false; break;
                case SDLK_j: down[7] = false; break;
                case SDLK_i: down[8] = false; break;
                case SDLK_k: down[9] = false; break;
                case SDLK_o: down[10] = false; break;
                case SDLK_l: down[11] = false; break;
                case SDLK_SEMICOLON: down[12] = false; break;
                }
                break;
            }

            bool new = any(down, 13);
            if (play != new) {
                play = new;
                if (!play) freq = 0;
            }
        }
    }

    if (wav.file) end_recording(&wav, start);

    SDL_DestroyWindow(window);
    SDL_CloseAudio();
    SDL_Quit();
}
