#include "audio.h"
#include "compress.h"
#include "g711.h"

int Normalize(AudioSample* value);
/* The number of bits required by each value */
static unsigned char numBits[] = {
   0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
   6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
};

/* Mu-Law conversions */
static bool muLawDecodeTableInitialized = false;
static AudioSample muLawDecodeTable[256];

// Constructor initializes the decoding table
DecompressG711MuLaw::DecompressG711MuLaw(AudioAbstract& a)
    : AbstractDecompressor(a) {
    cerr << "Encoding: ITU G.711 mu-Law\n";
    if (!muLawDecodeTableInitialized) {
        muLawDecodeTableInitialized = true;
        for (int i = 0; i < 256; i++)
            muLawDecodeTable[i] = MuLawDecode(i);
    }
}
size_t DecompressG711MuLaw::GetSamples(AudioSample* buffer,
    size_t length) {
    AudioByte* byteBuff =
        reinterpret_cast<AudioByte*>(buffer);
    size_t read = ReadBytes(byteBuff, length);
    for (long i = read - 1; i >= 0; i--)
        buffer[i] = muLawDecodeTable[byteBuff[i]];
    return read;
};
AudioByte MuLawEncode(AudioSample s) {
    unsigned char sign = (s < 0) ? 0 : 0x80; // Save the sign
    if (s < 0) s = -s; // make sample positive
    signed long adjusted = static_cast<long>(s) << (16 - sizeof(AudioSample) * 8);
    adjusted += 128L + 4L;
    if (adjusted > 32767) adjusted = 32767;
    unsigned char exponent = numBits[(adjusted >> 7) & 0xFF] - 1;
    unsigned char mantissa = (adjusted >> (exponent + 3)) & 0xF;
    return ~(sign | (exponent << 4) | mantissa);
};
AudioSample MuLawDecode(AudioByte ulaw) {
    ulaw = ~ulaw;
    unsigned char exponent = (ulaw >> 4) & 0x7;
    unsigned char mantissa = (ulaw & 0xF) + 16;
    unsigned long adjusted = (mantissa << (exponent + 3)) - 128 - 4;
    return (ulaw & 0x80) ? adjusted : -adjusted;
};
static bool aLawDecodeTableInitialized = false;
static AudioSample aLawDecodeTable[256];

DecompressG711ALaw::DecompressG711ALaw(AudioAbstract& a)
    : AbstractDecompressor(a) {
    cerr << "Encoding: ITU G.711 A-Law\n";
    if (!aLawDecodeTableInitialized) {
        aLawDecodeTableInitialized = true;
        for (int i = 0; i < 256; i++)
            aLawDecodeTable[i] = ALawDecode(i);
    }
}

size_t DecompressG711ALaw::GetSamples(AudioSample* buffer, size_t length) {
    AudioByte* byteBuff =
        reinterpret_cast<AudioByte*>(buffer);
    size_t read = ReadBytes(byteBuff, length);
    for (long i = read - 1; i >= 0; i--)
        buffer[i] = aLawDecodeTable[byteBuff[i]];
    return read;
}

AudioByte ALawEncode(AudioSample s) {
    unsigned char sign = (s < 0) ? 0 : 0x80; // Save the sign
    if (s < 0) s = -s; // make sample positive
    /*signed long adjusted = static_cast<long>(s)+8L; // Round it
    if (adjusted > 32767) adjusted = 32767; // Clip it
    unsigned char exponent = numBits[(adjusted>>8)&0x7F];
    unsigned char mantissa = (adjusted >> (exponent + 4)) & 0xF;
    return sign | (((exponent << 4) | mantissa) ^ 0x55); */

    unsigned char exponent;
    unsigned char mantissa;
    if (s < 0x0100) {
        exponent = 0;
        mantissa = (s >> 4) & 0x000F;
    }
    else {
        int numBits = Normalize(&s);
        exponent = (7 - numBits) << 4;
        mantissa = (s & 0x3C00) >> 10;
    }
    return (sign | exponent | mantissa) ^ 0xD5;
};

AudioSample ALawDecode(AudioByte alaw) {
    alaw ^= 0x55;
    unsigned char exponent = (alaw >> 4) & 0x7;
    unsigned char mantissa = (alaw & 0xF) + (exponent ? 16 : 0);
    unsigned long adjusted = (mantissa << (exponent + 4));
    return (alaw & 0x80) ? -adjusted : adjusted;
};
int Normalize(AudioSample* value) {
    int numBits = 0;
    int msb = (value[0] & 0x8000) >> 15;
    int nextmsb = (value[0] & 0x4000) >> 14;
    while (msb == nextmsb) {
        value[0] = value[0] << 1;
        numBits++;
        msb = (value[0] & 0x8000) >> 15;
        nextmsb = (value[0] & 0x4000) >> 14;
    }
    return numBits;
}
