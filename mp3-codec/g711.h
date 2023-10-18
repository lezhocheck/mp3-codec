#pragma once
#include "audio.h"
#include "compress.h"

class DecompressG711MuLaw : public AbstractDecompressor {
public:
	DecompressG711MuLaw(AudioAbstract& a);
	size_t GetSamples(AudioSample* buffer, size_t length);
};
AudioSample MuLawDecode(AudioByte);
AudioByte MuLawEncode(AudioSample);
class DecompressG711ALaw : public AbstractDecompressor {
private:
	static AudioSample* _decodeTable;
public:
	DecompressG711ALaw(AudioAbstract& a);
	size_t GetSamples(AudioSample* buffer, size_t length);
};

AudioSample ALawDecode(AudioByte);
AudioByte ALawEncode(AudioSample);