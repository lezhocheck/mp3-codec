#include "open.h"

#include "wav.h"  
#include "mpeg.h" 

AudioAbstract* OpenFormat(istream& file) {
	if (IsWaveFile(file)) {
		file.seekg(0);
		return new WaveRead(file);
	} else if (IsMpegFile(file)) {
		file.seekg(0);
		std::ostringstream out;
		return new MpegRead(file, out);
	}
	return 0;
}
