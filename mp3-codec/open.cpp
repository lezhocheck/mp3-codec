#include "open.h"

#include "wav.h"  
#include "mpeg.h" 

AudioAbstract* OpenFormat(istream& file) {
	if (IsWaveFile(file)) {
		file.seekg(0);
		return new WaveRead(file);
	}
	return 0;
}
