#pragma once

#include <istream>
#include "audio.h"
#include "compress.h"

bool IsWaveFile(istream& file);

class WaveRead : public AudioAbstract {
private:
    istream& _stream;
    AbstractDecompressor* _decoder; // Current decompressor
    unsigned char* _formatData; // Contents of fmt chunk
    unsigned long _formatDataLength; // length of fmt chunk
public:
    WaveRead(istream& s);
    ~WaveRead();

private:
    // WAVE chunk stack
    struct {
        unsigned long type; // Type of chunk
        unsigned long size; // Size of chunk
        unsigned long remaining; // Bytes left to read
        bool isContainer;   // true if this is a container
        unsigned long containerType; // type of container
    } _chunk[5];

    int _currentChunk; // top of stack

    void NextChunk(void);
private:
    size_t GetSamples(AudioSample* buffer, size_t numSamples);
    void InitializeDecompression();
public:
    size_t ReadBytes(AudioByte* buffer, size_t numSamples);
    void MinMaxSamplingRate(long* min, long* max, long* preferred);
    void MinMaxChannels(int* min, int* max, int* preferred);
};