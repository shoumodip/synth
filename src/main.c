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
 * Render the window
 *
 * @param renderer *SDL_Renderer The renderer of the window
 * @param down *bool The array of keys down, defaults to {false, false, ... n(12)}
 */
void render(SDL_Renderer *renderer, bool *down)
{
    // The window dimensions
    int x, y;
    SDL_GetRendererOutputSize(renderer, &x, &y);

    // The recording indicator dimensions
    const size_t recording_side = 20, recording_padding = 4, recording_width = 2;

    // The keys
    const size_t keys_white_count = 12;
    const size_t keys_white_padding = 4;

    // Render the window
    SDL_SetRenderDrawColor(renderer, 0x2B, 0x2B, 0x2B, 0xFF);
    SDL_RenderClear(renderer);

    // Render the keys
    SDL_Rect keys = {
        .x = keys_white_padding,
        .y = recording_side + 2 * recording_padding,
        .w = x / keys_white_count - keys_white_padding,
        .h = y - recording_side - 2 * recording_padding - keys_white_padding
    };

    for (size_t i = 0; i < keys_white_count; ++i) {
        bool pressed = down ? down[i] : false;

        if (i == 1 || i == 3 || i == 6 || i == 8 || i == 10) {
            if (pressed) {
                SDL_SetRenderDrawColor(renderer, 0x4B, 0x4B, 0x4B, 0xFF);
            } else {
                SDL_SetRenderDrawColor(renderer, 0x38, 0x38, 0x38, 0xFF);
            }
        } else {
            if (pressed) {
                SDL_SetRenderDrawColor(renderer, 0x98, 0x98, 0x90, 0xFF);
            } else {
                SDL_SetRenderDrawColor(renderer, 0xDC, 0xDC, 0xCC, 0xFF);
            }
        }

        SDL_RenderFillRect(renderer, &keys);
        keys.x += keys.w + keys_white_padding;
    }

    // Render the recording indicator
    SDL_Rect recording = {
        .x = x - recording_side - recording_padding,
        .y = recording_padding,
        .w = recording_side,
        .h = recording_side
    };

    SDL_SetRenderDrawColor(renderer, 0xCC, 0x93, 0x93, 0xFF);
    SDL_RenderFillRect(renderer, &recording);

    // Hollow out the recording indicator if currently not recording
    if (wav.file == NULL) {
        recording.x += recording_width;
        recording.y += recording_width;
        recording.w -= 2 * recording_width;
        recording.h -= 2 * recording_width;

        SDL_SetRenderDrawColor(renderer, 0x2B, 0x2B, 0x2B, 0xFF);
        SDL_RenderFillRect(renderer, &recording);
    }

    SDL_RenderPresent(renderer);
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

/*
 * Start a recording
 *
 * @param wav *Wav The wav context
 * @param start time_t The starting time
 */
void start_recording(Wav *wav, time_t start)
{
    size_t length = snprintf(NULL, 0, OUTPUT_FILE_FMT, start);
    char output[length + 1];
    snprintf(output, length + 1, OUTPUT_FILE_FMT, start);

    wav->file = fopen(output, "wb");
}

int main(void)
{
    // Initialize SDL
    scc(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));

    // Setup the window
    SDL_Window *window = scp(SDL_CreateWindow("Synth",
                                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              640, 480, SDL_WINDOW_SHOWN));

    SDL_Renderer *renderer =  SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    render(renderer, NULL);

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
    bool down[12] = {0};        // The keys down
    SDL_Event e;                // The event
    bool running = true;        // Whether the application is running
    time_t start;
    int head = -1;

    while (running) {
        render(renderer, down);
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT: running = false; break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                case 'z': octave = -8; break;
                case 'x': octave = 4; break;
                case 'c': octave = 16; break;
                case 'v': octave = 28; break;
                case 'b': octave = 40; break;
                case 'n': octave = 52; break;
                case 'm': octave = 64; break;
                case ',': octave = 76; break;
                case '.': octave = 88; break;

                case 'a': // C
                    head = 0;
                    down[head] = true;
                    break;

                case 'w': // C#
                    head = 1;
                    down[head] = true;
                    break;

                case 's': // D
                    head = 2;
                    down[head] = true;
                    break;

                case 'e': // D#
                    head = 3;
                    down[head] = true;
                    break;

                case 'd': // E
                    head = 4;
                    down[head] = true;
                    break;

                case 'f': // F
                    head = 5;
                    down[head] = true;
                    break;

                case 'u': // F#
                    head = 6;
                    down[head] = true;
                    break;

                case 'j': // G
                    head = 7;
                    down[head] = true;
                    break;

                case 'i': // G#
                    head = 8;
                    down[head] = true;
                    break;

                case 'k': // A
                    head = 9;
                    down[head] = true;
                    break;

                case 'o': // A#
                    head = 10;
                    down[head] = true;
                    break;

                case 'l': // B
                    head = 11;
                    down[head] = true;
                    break;

                case 'q': // Quick the application
                    running = false;
                    break;

                case 'r': // Toggle recording
                    if (wav.file) {
                        end_recording(&wav, start);
                    } else {
                        start = time(NULL);
                        start_recording(&wav, start);
                    }
                }
                break;

            case SDL_KEYUP:
                switch (e.key.keysym.sym) {
                case SDLK_a:
                    down[0] = false;
                    break;
                case SDLK_w:
                    down[1] = false;
                    break;
                case SDLK_s:
                    down[2] = false;
                    break;
                case SDLK_e:
                    down[3] = false;
                    break;
                case SDLK_d:
                    down[4] = false;
                    break;
                case SDLK_f:
                    down[5] = false;
                    break;
                case SDLK_u:
                    down[6] = false;
                    break;
                case SDLK_j:
                    down[7] = false;
                    break;
                case SDLK_i:
                    down[8] = false;
                    break;
                case SDLK_k:
                    down[9] = false;
                    break;
                case SDLK_o:
                    down[10] = false;
                    break;
                case SDLK_l:
                    down[11] = false;
                    break;
                }
                break;
            }

            freq = any(down, 12) ? normalize(get_freq(octave + head)) : 0;
        }
    }

    if (wav.file) end_recording(&wav, start);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_CloseAudio();
    SDL_Quit();
}
