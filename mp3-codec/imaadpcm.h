#pragma once
#include "audio.h"

struct ImaState {
	int index;    // Index into step size table
	int previousValue; // Most recent sample value
};

// Decode/Encode a single sample and update state
AudioSample ImaAdpcmDecode(AudioByte deltaCode, ImaState&);
AudioByte ImaAdpcmEncode(AudioSample, ImaState&);

class DecompressImaAdpcmMs : public AbstractDecompressor {
private:
	int  _channels;
	AudioSample* _samples[2];  // Left and right sample buffers
	AudioSample* _samplePtr[2]; // Pointers to current samples
	size_t   _samplesRemaining; // Samples remaining in each channel
	size_t   _samplesPerPacket; // Total samples per packet
public:
	DecompressImaAdpcmMs(AudioAbstract& a, int packetLength, int channels);
	~DecompressImaAdpcmMs();
	size_t GetSamples(AudioSample* outBuff, size_t len);
private:
	AudioByte* _packet;   // Temporary buffer for packets
	size_t   _bytesPerPacket; // Size of a packet
	size_t  NextPacket();
};
class DecompressImaAdpcmApple : public AbstractDecompressor {
private:
	int _channels;

	ImaState _state[2];
	AudioSample _samples[2][64];
	AudioSample* _samplePtr[2];

	size_t   _samplesRemaining;
	size_t  NextPacket(AudioSample* sampleBuffer, ImaState& state);

public:
	DecompressImaAdpcmApple(AudioAbstract& a, int channels);
	size_t GetSamples(AudioSample* outBuff, size_t len);
};