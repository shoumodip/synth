#ifndef WAV_H
#define WAV_H

/*
 * Stolen from https://faculty.fiu.edu/~wgillam/wavfiles.html
 * Only the identifier names and the code style has been changed
 * to match with SDL and prevent confusion in the future.
 */

#include <stdint.h>
#include <stdio.h>
  
typedef struct {
    uint_fast16_t channels;     // 1 for mono, 2 for stereo, etc
    uint_fast16_t sample_bits;  // 16 for CD, 24 for high-res, etc
    uint_fast32_t freq;         // 44100 for CD, 88200, 96000, 192000, etc
    uint_fast32_t samples;      // total number of samples per channel
    FILE *file;                 // The output file
} Wav;

void write_wav_hdr(Wav *wav);
void write_sample(Wav *wav, const int_fast32_t sample);

#endif // WAV_H
