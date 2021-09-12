#include "wav.h"
#include <stdlib.h>

/*
 * Write a standard 44-byte PCM format RIFF/WAVE header to the beginning. Again,
 * the seek position will be left at the beginning of the data section, so one
 * can immediately begin writing samples.
 *
 * @param wav *Wav The wav context
 *
 */ 
void write_wav_hdr(Wav *wav)
{
    // We'll need the following:
    uint32_t data_size = (wav->samples) * (wav->channels) * (wav->sample_bits) / 8;
    uint32_t RCS = data_size + 36;
    uint32_t byte_rate = (wav->freq) * (wav->channels) * (wav->sample_bits) / 8;
    uint16_t block_align = (wav->channels) * (wav->sample_bits) / 8;

    // Prepare a standard 44 byte WAVE header from the info in wav
    uint8_t h[44];

    // Bytes 1-4 are the ASCII codes for the four characters "RIFF"
    h[0] = 0x52;
    h[1] = 0x49;
    h[2] = 0x46;
    h[3] = 0x46;

    // Bytes 5-8 are RCS (i.e. data_size plus the remaining 36 bytes in the header)
    // in Little Endian format
    for (int i = 0; i < 4; ++i) h[4 + i] = (RCS >> (8 * i)) & 0xFF;

    // Bytes 9-12 are the ASCII codes for the four characters "WAVE"
    h[8] = 0x57;
    h[9] = 0x41;
    h[10] = 0x56;
    h[11] = 0x45;

    // Bytes 13-16 are the ASCII codes for the four characters "fmt "
    h[12] = 0x66;
    h[13] = 0x6D;
    h[14] = 0x74;
    h[15] = 0x20;

    // Bytes 17-20 are the integer 16 (the size of the "fmt " subchunk
    // in the RIFF header we are writing) as a four-byte integer in
    // Little Endian format
    h[16] = 0x10;
    h[17] = 0x00;
    h[18] = 0x00;
    h[19] = 0x00;

    // Bytes 21-22 are the integer 1 (to indicate PCM format),
    // written as a two-byte Little Endian
    h[20] = 0x01;
    h[21] = 0x00;

    // Bytes 23-24 are channels as a two-byte Little Endian
    for (int j = 0; j < 2; ++j) h[22 + j] = (wav->channels >> (8 * j)) & 0xFF;

    // Bytes 25-26 are freq as a four-byte L.E.
    for (int i = 0; i < 4; ++i) h[24 + i] = (wav->freq >> (8 * i)) & 0xFF;

    // Bytes 27-30 are byte_rate as a four-byte L.E.
    for (int i = 0; i < 4; ++i) h[28 + i] = (byte_rate >> (8 * i)) & 0xFF;

    // Bytes 31-34 are block_align as a two-byte L.E.
    for (int j = 0; j < 2; ++j) h[32 + j] = (block_align >> (8 * j)) & 0xFF;

    // Bytes 35-36 are sample_bits as a two-byte L.E.
    for (int j = 0; j < 2; ++j) h[34 + j] = (wav->sample_bits >> (8 * j)) & 0xFF;

    // Bytes 37-40 are the ASCII codes for the four characters "data"
    h[36] = 0x64;
    h[37] = 0x61;
    h[38] = 0x74;
    h[39] = 0x61;

    // Bytes 41-44 are data_size as a four-byte L.E.
    for (int i = 0; i < 4; ++i) h[40 + i] = (data_size >> (8 * i)) & 0xFF;

    // Write the header to the beginning
    if (fseek(wav->file, 0, SEEK_SET)) {
        fprintf(stderr, "Error with fseek in write_wav_header in wav.c\n");
        exit(EXIT_FAILURE);
    }

    fwrite(h, 1, 44, wav->file);
}

/*
 * Write a sample in the correct Little Endian format. Sample should be an array
 * with w->channels elements. Note that we use the int_fast32_t datatype to
 * hold samples, which should be changed if you want to use sample_bits >
 * 32. Also, if you use, say, sample_bits=24, then you want to make sure
 * that your actual samples are going to fit into a 3-byte Little Endian integer
 * in twos-complement encoding. If you're only going to use sample_bits=16,
 * you could use int_fast16_t instead of int_fast32_t. Note also that the WAVE
 * file format expects 8-bit samples to be UNsigned, so if you're going to use
 * sample_bits=8, then you should use uint_fast8_t to hold your samples.
 *
 * @param wav *Wav The wav context
 * @param sample int_fast32_t The sample
 */ 
void write_sample(Wav *wav, const int_fast32_t sample)
{
    // We'll assume wav->sample_bits is divisible by 8, otherwise one should do
    // bytes_per_sample++ and make sure the last (wav->sample_bits % 8) bits of
    // each sample[i] are zero

    int b = wav->sample_bits / 8; // Bytes per sample
    uint8_t x[b];

    // Populate x with sample in Little Endian format, then write it
    for (int i = 0; i < b; ++i) x[i] = (sample >> (8 * i)) & 0xFF;
    fwrite(x, 1, b, wav->file);
}
