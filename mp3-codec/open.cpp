#include "open.h"

#include "wav.h"  // Microsoft RIFF WAVE

AudioAbstract* OpenFormat(istream& file) {
	if (IsWaveFile(file)) {
		file.seekg(0);
		return new WaveRead(file);
	}
	return 0;
}
