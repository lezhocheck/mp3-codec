#include <iostream>
#include <cstring>
#include <cmath>
#include <cstdio>
#include "audio.h"
#include "compress.h"
#include "mpeg.h"

size_t DecompressMpeg::GetSamples(AudioSample* outBuff, size_t
	wanted) {
	long remaining = wanted;
	while (remaining > 0) {
		if (_samplesRemaining == 0) { 
			NextFrame(); 
			if (_samplesRemaining == 0)
				return wanted - remaining;
		}
		switch (_channels) {
		case 1: 
			while ((_samplesRemaining > 0) && (remaining > 0)) {
				*outBuff++ = *_pcmSamples[0]++;
				_samplesRemaining--;
				remaining--;
			}
			break;
		case 2: 
			while ((_samplesRemaining > 0) && (remaining > 0)) {
				*outBuff++ = *_pcmSamples[0]++;
				*outBuff++ = *_pcmSamples[1]++;
				_samplesRemaining--;
				remaining -= 2;
			}
			break;
		}
	}
	return wanted - remaining;
}

void DecompressMpeg::NextFrame() {
	if (ParseHeader()) { 
		_samplesRemaining = 0;
		return;
	}

	_pcmSamples[0] = _sampleStorage[0];
	_pcmSamples[1] = _sampleStorage[1];
	switch (_layer) {
	case 1: LayerlDecode(); break;
	case 2: Layer2Decode(); break;
	case 3: Layer3Decode(); break;
	}

	_pcmSamples[0] = _sampleStorage[0];
	_pcmSamples[1] = _sampleStorage[1];
}

void DecompressMpeg::FillBuffer() {

	if (_header > _bufferStorage + sizeof(_bufferStorage)) {

		cerr << "Internal error; buffer exhausted!\n";
		_buffer = _header;
	}
	if (_buffer > _header) { 

		cerr << "Synchronization error; frame too big!\n";
		_buffer = _header;
	}

	if (_buffer < (_bufferStorage + 512)) return;

	int totalBufferSize = sizeof(_bufferStorage);
	int bufferSize = _bufferEnd - _buffer;
	memmove(_bufferStorage, _buffer, bufferSize);
	_header -= _buffer - _bufferStorage;
	_bufferEnd -= _buffer - _bufferStorage;
	_buffer = _bufferStorage;
	_bufferEnd += ReadBytes(_bufferEnd, totalBufferSize - bufferSize);
}

void DecompressMpeg::ResetBits() {
	_bitsRemaining = 8;
}
int masks[] = { 0,1,3,7,0xF,0x1F,0x3F,0x7F,0xFF };
long DecompressMpeg::GetBits(int numBits) {
	if (_bitsRemaining == 0) { 
		_buffer++; 
		_bitsRemaining = 8;
	}
	if (_bitsRemaining >= numBits) { 
		_bitsRemaining -= numBits;
		return (*_buffer >> _bitsRemaining) & masks[numBits];
	}

	long result = (*_buffer & masks[_bitsRemaining])
		<< (numBits - _bitsRemaining);
	numBits -= _bitsRemaining; 

	_bitsRemaining = 8; 
	_buffer++;
	return result | GetBits(numBits);
}

static short bitRateTable[2][4][16] = { 
	{ 
		{0 }, 
		{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,0 },
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0 },
		{0,8,16,24,32,40,48,56,64,80,96,112, 128,144, 160, 0 },
	}, 
	{ 
		{0 },
		{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0 },
		{0,32,48,56,64,80,96,112,128,160,192,224,256,320,384,0 },
		{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0 },
	} 
};

static long samplingRateTable[2][4] = { 
	{22050,24000,16000,0}, 
	{44100,48000,32000,0}
};

bool DecompressMpeg::ParseHeader() {
	FillBuffer(); 
	if (_bufferEnd - _header < 4)
		return true;
	if ((_header[0] != 0xFF) || ((_header[1] & 0xF0) != 0xF0)) {
		cerr << "Syncword not found.";
		return true; 
	}
	_id = (_header[1] & 8) >> 3; 
	_layer = -(_header[1] >> 1) & 3; 
	_protection = (~_header[1] & 1); 
	int bitrateIndex = (_header[2] & 0xF0) >> 4;
	_bitRate = bitRateTable[_id][_layer][bitrateIndex] * 1000;
	int samplingRateIndex = (_header[2] & 0x0C) >> 2;
	_samplingRate = samplingRateTable[_id][samplingRateIndex];
	_padding = (_header[2] & 0x02) >> 1;
	_private = (_header[2] & 0x01);
	_mode = (_header[3] & 0xC0) >> 6;
	_modeExtension = (_header[3] & 0x30) >> 4;
	switch (_mode) {
	case 0: 
		_channels = 2;
		_bound = 32;
		break;
	case 1:
		_channels = 2;
		_bound = (_modeExtension + 1) << 2;
		break;
	case 2: 
		_channels = 2;
		_bound = 32;
		break;
	case 3: 
		_channels = 1;
		_bound = 0;
		break;
	}
	_copyright = (_header[3] & 0x08);
	_original = (_header[3] & 0x04);
	_emphasis = (_header[3] & 0x03);
	_buffer = _header + 4;
	if (_protection) _buffer += 2; 

	if (_bitRate == 0) { 
		if (_headerSpacing) { 
			if (_layer == 1) { 
				_header += _headerSpacing * 4;
				if (_padding) _header += 4;
			}
			else { 
				_header += _headerSpacing;
				if (_padding) _header += 1;
			}
		}
		else { 
			int slotLength = (_layer == 1) ? 4 : 1;
			_headerSpacing = 1;
			_header += slotLength;
			while ((_header[0] != 0xFF) || ((_header[1] & 0xF0) != 0xF0)) {
				_header += slotLength;
				_headerSpacing++;
			}
			if (_padding) _headerSpacing--;
		}
	}
	else if (_layer == 1) { 
		_headerSpacing = 
			384 
			/ 32 
			* _bitRate 
			/ _samplingRate; 
		_header += _headerSpacing * 4; 
		if (_padding) _header += 4; 
	}
	else {
		_headerSpacing =
			1152 
			/ 8 
			* _bitRate 
			/ _samplingRate; 
		_header += _headerSpacing; 
		if (_padding) _header += 1; 
	} 

	return false; 
}


static long SynthesisWindowCoefficients[] = //wtf
{ 0, -1, -1, -1, -1, -1, -1, -2, -2, -2, -2, -3, -3, -4, -4, -5,
-5, -6, -7, -7, -8, -9, -10, -11, -13, -14, -16, -17, -19, -21,
-24, -26,
-29, -31, -35, -38, -41, -45, -49, -53, -58, -63, -68, -73, -79,
-85, -91, -97, -104, -111, -117, -125, -132, -139, -147, -154,
-161,-169, -176, -183, -190, -196, -202, -208,
213, 218, 222, 225, 227, 228, 228, 227, 224, 221, 215, 208, 200,
189, 177, 163, 146, 127, 106, 83, 57, 29, -2, -36, -72, -111,
-153, -197,-244, -294, -347, -401,
-459, -519, -581, -645, -711, -779, -848, -919, -991, -1064,
-1137, -1210, -1283, -1356, -1428, -1498, -1567, -1634, -1698,
-1759, -1817, -1870, -1919, -1962, -2001, -2032, -2057, -2075,
-2085, -2087, -2080, -2063,
2037, 2000, 1952, 1893, 1822, 1739, 1644, 1535, 1414, 1280, 1131,
970, 794, 605, 402, 185, -45, -288, -545, -814, -1095, -1388,
-1692, -2006, -2330, -2663, -3004, -3351, -3705, -4063, -4425,
-4788,
-5153, -5517, -5879, -6237, -6589, -6935, -7271, -7597, -7910,
-8209, -8491, -8755, -8998, -9219, -9416, -9585, -9727, -9838,
-9916, -9959, -9966, -9935, -9863, -9750, -9592, -9389, -9139,
-8840, -8492, -8092, -7640, -7134,
6574, 5959, 5288, 4561, 3776, 2935, 2037, 1082, 70, -998, -2122,
-3300, -4533, -5818, -7154, -8540, -9975, -11455, -12980, -14548,
-16155, -17799, -19478, -21189, -22929, -24694, -26482, -28289,
-30112, -31947, -33791, -35640,-37489, -39336, -41176, -43006,
-44821, -46617, -48390, -50137, -51853, -53534, -55178, -56778,
-58333, -59838, -61289, -62684, -64019, -65290, -66494, -67629,
-68692, -69679, -70590, -71420, -72169, -72835, -73415, -73908,
-74313, -74630, -74856, -74992, 75038, 74992, 74856, 74630,
74313, 73908, 73415, 72835, 72169, 71420, 70590, 69679, 68692,
67629, 66494, 65290, 64019, 62684, 61289, 59838, 58333, 56778,
55178, 53534, 51853, 50137, 48390, 46617, 44821, 43006, 41176,
39336, 37489, 35640, 33791, 31947, 30112, 28289, 26482, 24694,
22929, 21189, 19478, 17799, 16155, 14548, 12980, 11455, 9975,
8540, 7154, 5818, 4533, 3300, 2122, 998, -70, -1082, -2037,
-2935, -3776, -4561, -5288, -5959, 6574, 7134, 7640, 8092, 8492,
8840, 9139, 9389, 9592, 9750, 9863, 9935, 9966, 9959, 9916, 9838,
9727, 9585, 9416, 9219, 8998, 8755, 8491, 8209, 7910, 7597, 7271,
6935, 6589, 6237, 5879, 5517, 5153, 4788, 4425, 4063, 3705, 3351,
3004, 2663, 2330, 2006, 1692, 1388, 1095, 814, 545, 288, 45,
-185, -402, -605, -794, -970, -1131, -1280, -1414, -1535, -1644,
-1739, -1822, -1893, -1952, -2000, 2037, 2063, 2080, 2087, 2085,
2075, 2057, 2032, 2001, 1962, 1919, 1870, 1817, 1759, 1698, 1634,
1567, 1498, 1428, 1356, 1283, 1210, 1137, 1064, 991, 919, 848,
779, 711, 645, 581, 519,
459, 401, 347, 294, 244, 197, 153, 111, 72, 36, 2, -29, -57, -83,
-106, -127, -146, -163, -177, -189, -200, -208, -215, -221, -224,
-227, -228, -228, -227, -225, -222, -218,
213, 208, 202, 196, 190, 183, 176, 169, 161, 154, 147, 139, 132,
125, 117, 111, 104, 97, 91, 85, 79, 73, 68, 63, 58, 53, 49, 45,
41, 38, 35, 31,
29, 26, 24, 21, 19, 17, 16, 14, 13, 11, 10, 9, 8, 7, 7, 6, 5, 5,
4, 4, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1 };


static void Matrix(long* V, long* subbandSamples, int
	numSamples) {
	for (int i = numSamples; i < 32; i++)
		subbandSamples[i] = 0;
	static const double PI = 3.14159265358979323846;
	long* workR = V; 
	long workI[64]; 
	static const char order[] =
	{ 0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30,
	1,17,9,25,5,21,13,29,3,19,11,27,7,23,15,31 };
	long* pWorkR = workR; 
	long* pWorkI = workI; 
	const char* next = order;
	for (int n = 0; n < 16; n++) {
		long a = subbandSamples[*next++];
		long b = subbandSamples[*next++];
		*pWorkR++ = a + b; *pWorkI++ = 0;
		*pWorkR++ = a; *pWorkI++ = b;
		*pWorkR++ = a - b; *pWorkI++ = 0;
		*pWorkR++ = a; *pWorkI++ = -b;
	}
	
	static long phaseShiftsR[32], phaseShiftsI[32]; 
	static bool initializedPhaseShifts = false;
	if (!initializedPhaseShifts) { 
		for (int i = 0; i < 32; i++) { 
			phaseShiftsR[i] = static_cast<long>(16384.0 * cos(i * (PI /
				32.0)));
			phaseShiftsI[i] = static_cast<long>(16384.0 * sin(i * (PI /
				32.0)));
		}
		initializedPhaseShifts = true;
	}

	int phaseShiftIndex, phaseShiftStep = 8;
	for (int size = 4; size < 64; size <<= 1) {
		for (int n = 0; n < 64; n += 2 * size) {
			long tR = workR[n + size];
			workR[n + size] = (workR[n] - tR) >> 1;
			workR[n] = (workR[n] + tR) >> 1;
			long tI = workI[n + size];
			workI[n + size] = (workI[n] - tI) >> 1;
			workI[n] = (workI[n] + tI) >> 1;
		}
		phaseShiftIndex = phaseShiftStep;
		for (int fftStep = 1; fftStep < size; fftStep++) {
			long phaseShiftR = phaseShiftsR[phaseShiftIndex];
			long phaseShiftI = phaseShiftsI[phaseShiftIndex];
			phaseShiftIndex += phaseShiftStep;
			for (int n = fftStep; n < 64; n += 2 * size) {
				long tR = (phaseShiftR * workR[n + size]
					- phaseShiftI * workI[n + size]) >> 14;
				long tI = (phaseShiftR * workI[n + size]
					+ phaseShiftI * workR[n + size]) >> 14;
				workR[n + size] = (workR[n] - tR) >> 1;
				workI[n + size] = (workI[n] - tI) >> 1;
				workR[n] = (workR[n] + tR) >> 1;
				workI[n] = (workI[n] + tI) >> 1;
			}
		}
		phaseShiftStep /= 2;
	}

	{
		static long vShiftR[64], vShiftI[64]; 
		static bool initializedVshift = false;
		int n;
		if (!initializedVshift) { 
			for (n = 0; n < 32; n++) { 
				vShiftR[n] =
					static_cast<long>(16384.0 * cos((32 + n) * (PI / 64.0)));
				vShiftI[n] =
					static_cast<long>(16384.0 * sin((32 + n) * (PI / 64.0)));
			}
			initializedVshift = true;
		}
		long* pcmR = workR + 33; 
		long* pcmI = workI + 33; 
		V[16] = 0; 
		for (n = 1; n < 32; n++) { 
			V[n + 16] = (vShiftR[n] * *pcmR++ - vShiftI[n] *
				*pcmI++) >> 15;
		}
		V[48] = (-workR[0]) >> 1; 
		for (n = 0; n < 16; n++) V[n] = -V[32 - n];
		for (n = 1; n < 16; n++) V[48 + n] = V[48 - n];
	}
}

void DecompressMpeg::Layerl2Synthesis(
	long* V[16],
	long* subbandSamples,
	int numSubbandSamples,
	AudioSample* pcmSamples) {
	long* t = V[15];
	for (int i = 15; i > 0; i--) 
		V[i] = V[i - 1];
	V[0] = t;

	Matrix(V[0], subbandSamples, numSubbandSamples);
	static long D[512];
	static bool initializedD = false;
	if (!initializedD) {
		long* nextD = D;
		for (int j = 0; j < 32; j++)
			for (int i = 0; i < 16; i += 2) {
				*nextD++ = SynthesisWindowCoefficients[j + 32 * i] >> 4;
				*nextD++ = SynthesisWindowCoefficients[j + 32 * i + 32] >> 4;
			}
		initializedD = true;
	}

	long* nextD = D;
	for (int j = 0; j < 32; j++) {
		long sample = 0; 
		for (int i = 0; i < 16; i += 2) {
			sample += (*nextD++ * V[i][j]) >> 8;
			sample += (*nextD++ * V[i + 1][j + 32]) >> 8;
		}
		*pcmSamples++ = sample >> 1; 
	}
}

static long* layerlScaleFactors = 0;

inline long LayerlRequant(long sample, int width, int scaleIndex)
{
	long levels = (1 << width) - 1;
	return (layerlScaleFactors[scaleIndex] *
		(((sample + sample + 1 - levels) << 15) / levels)
		) >> 14;
}

void DecompressMpeg::LayerlDecode() {
	int allocation[2][32]; 
	ResetBits();

	if (_channels == 1) {
		for (int sb = 0; sb < 32; sb++)
			allocation[0][sb] = GetBits(4);
	}
	else {
		int sb;
		for (sb = 0; sb < _bound; sb++) {
			allocation[0][sb] = GetBits(4);
			allocation[1][sb] = GetBits(4);
		}
		for (; sb < 32; sb++) {
			allocation[0][sb] = GetBits(4);
			allocation[1][sb] = allocation[0][sb];
		}
	}

	int scaleFactor[2][32];

	{
		for (int sb = 0; sb < 32; sb++)
			for (int ch = 0; ch < _channels; ch++)
				if (allocation[ch][sb] != 0) {
					scaleFactor[ch][sb] = GetBits(6);
				}
	}

	long sbSamples[2][32]; 
	for (int gr = 0; gr < 12; gr++) { 
		if (_channels == 1) {
			for (int sb = 0; sb < 32; sb++) 
				if (!allocation[0][sb]) 
					sbSamples[0][sb] = 0;
				else {
					int width = allocation[0][sb] + 1;
					long s = GetBits(width);
					sbSamples[0][sb] = LayerlRequant(s, width,
						scaleFactor[0][sb]);
				}
			Layerl2Synthesis(_V[0], sbSamples[0], 32, _pcmSamples[0]);
			_pcmSamples[0] += 32;
			_samplesRemaining += 32;
		}
		else {
			{ 
				for (int sb = 0; sb < _bound; sb++)
					for (int ch = 0; ch < 2; ch++)
						if (!allocation[ch][sb])
							sbSamples[ch][sb] = 0;
						else {
							int width = allocation[ch][sb] + 1;
							long s = GetBits(width); 
							sbSamples[ch][sb] = LayerlRequant(s, width,
								scaleFactor[ch][sb]);
						}
			}
			{ 
				for (int sb = 0; sb < _bound; sb++) {
					if (!allocation[0][sb])
						sbSamples[0][sb] = 0;
					else {
						int width = allocation[0][sb] + 1;
						long s = GetBits(width);
					
						sbSamples[0][sb] = LayerlRequant(s, width,
							scaleFactor[0][sb]);
						sbSamples[1][sb] = LayerlRequant(s, width,
							scaleFactor[1][sb]);
	
						for (int ch = 0; ch < _channels; ch++) {
							Layerl2Synthesis(_V[ch], sbSamples[ch], 32, _pcmSamples[ch]);
							_pcmSamples[ch] += 32;
						}
						_samplesRemaining += 32;
					}
				}
			}
		}
	}
}

struct Layer2QuantClass {
	long _levels; 
	char _bits; 
	bool _grouping; 
};

inline long Layer2Requant(long sample,
	Layer2QuantClass* quantClass,
	int scaleIndex) {
	long levels = quantClass->_levels;
	return (layerlScaleFactors[scaleIndex] *
		(((sample + sample + 1 - levels) << 15) / levels)
		) >> 14;
}

static Layer2QuantClass l2qc3 = { 3,5,true };
static Layer2QuantClass l2qc5 = { 5,7,true };
static Layer2QuantClass l2qc7 = { 7,3,false };
static Layer2QuantClass l2qc9 = { 9,10,true };
static Layer2QuantClass l2qc15 = { 15,4,false };
static Layer2QuantClass l2qc31 = { 31,5,false };
static Layer2QuantClass l2qc63 = { 63,6,false };
static Layer2QuantClass l2qc127 = { 127,7,false };
static Layer2QuantClass l2qc255 = { 255,8,false };
static Layer2QuantClass l2qc511 = { 511,9,false };
static Layer2QuantClass l2qc1023 = { 1023,10,false };
static Layer2QuantClass l2qc2047 = { 2047,11,false };
static Layer2QuantClass l2qc4095 = { 4095,12,false };
static Layer2QuantClass l2qc8191 = { 8191,13,false };
static Layer2QuantClass l2qc16383 = { 16383,14,false };
static Layer2QuantClass l2qc32767 = { 32767,15,false };
static Layer2QuantClass l2qc65535 = { 65535,16,false };

Layer2QuantClass* l2allocationA[] = { 0,&l2qc3,&l2qc7,&l2qc15,
&l2qc31,&l2qc63,&l2qc127,&l2qc255,&l2qc511,&l2qc1023,&l2qc2047,
&l2qc4095,&l2qc8191,&l2qc16383,&l2qc32767,&l2qc65535 };
Layer2QuantClass * l2allocationB[] =
{ 0,&l2qc3,&l2qc5,&l2qc7,&l2qc9,
&l2qc15,&l2qc31,&l2qc63,&l2qc127,&l2qc255,&l2qc511,&l2qc1023,&l2qc2047,
&l2qc4095,&l2qc8191,&l2qc65535 };
Layer2QuantClass * l2allocationC[] = { 0,&l2qc3,&l2qc5,&l2qc7,
&l2qc9,&l2qc15,&l2qc31,&l2qc65535 };
Layer2QuantClass* l2allocationD[] = { 0,&l2qc3,&l2qc5,&l2qc65535 };
Layer2QuantClass* l2allocationE[] = { 0,&l2qc3,&l2qc5,&l2qc9,
&l2qc15,&l2qc31,&l2qc63,&l2qc127,&l2qc255,&l2qc511,&l2qc1023,&l2qc2047,
&l2qc4095,&l2qc8191,&l2qc16383,&l2qc32767 };
Layer2QuantClass* l2allocationF[] =
{ 0,&l2qc3,&l2qc5,&l2qc7,&l2qc9,
&l2qc15,&l2qc31,&l2qc63,&l2qc127,&l2qc255,&l2qc511,&l2qc1023,&l2qc2047,
&l2qc4095,&l2qc8191,&l2qc16383 };

struct Layer2BitAllocationTableEntry {
	char _numberBits;
	Layer2QuantClass** _quantClasses;
};

Layer2BitAllocationTableEntry Layer2AllocationB2a[32] = {
{ 4, l2allocationA }, { 4, l2allocationA }, { 4, l2allocationA },
{ 4, l2allocationB }, { 4, l2allocationB }, { 4, l2allocationB },
{ 4, l2allocationB }, { 4, l2allocationB }, { 4, l2allocationB },
{ 4, l2allocationB }, { 4, l2allocationB }, { 3, l2allocationC },
{ 3, l2allocationC }, { 3, l2allocationC }, { 3, l2allocationC },
{ 3, l2allocationC }, { 3, l2allocationC }, { 3, l2allocationC },
{ 3, l2allocationC }, { 3, l2allocationC }, { 3, l2allocationC },
{ 3, l2allocationC }, { 3, l2allocationC }, { 2, l2allocationD },
{ 2, l2allocationD }, { 2, l2allocationD }, { 2, l2allocationD },
{ 0,0}, { 0,0}, { 0,0}, { 0,0}, { 0,0}
};

Layer2BitAllocationTableEntry Layer2AllocationB2b[32] = {
{4, l2allocationA }, { 4, l2allocationA }, { 4, l2allocationA },
{4, l2allocationB }, { 4, l2allocationB }, { 4, l2allocationB },
{4, l2allocationB }, { 4, l2allocationB }, { 4, l2allocationB },
{4, l2allocationB }, { 4, l2allocationB }, { 3, l2allocationC },
{3, l2allocationC }, { 3, l2allocationC }, { 3, l2allocationC },
{3, l2allocationC }, { 3, l2allocationC }, { 3, l2allocationC },
{3, l2allocationC }, { 3, l2allocationC }, { 3, l2allocationC },
{3, l2allocationC }, { 3, l2allocationC }, { 2, l2allocationD },
{2, l2allocationD }, { 2, l2allocationD }, { 2, l2allocationD },
{2, l2allocationD }, { 2, l2allocationD }, { 2, l2allocationD },
{ 0,0}, { 0,0}
};

Layer2BitAllocationTableEntry Layer2AllocationB2c[32] = {
{ 4, l2allocationE }, { 4, l2allocationE }, { 3,
l2allocationE },
{ 3, l2allocationE }, { 3, l2allocationE }, { 3,
l2allocationE },
{ 3, l2allocationE }, { 3, l2allocationE },
{0,0},{0,0},{0,0},{0,0} , {0,0} , {0,0} , {0,0} , {0,0} ,
{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} , {0,0} ,
	{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
};

Layer2BitAllocationTableEntry Layer2AllocationB2d[32] = {
{ 4, l2allocationE }, { 4, l2allocationE }, { 3, l2allocationE },
{ 3, l2allocationE }, { 3, l2allocationE }, { 3, l2allocationE },
{ 3, l2allocationE }, { 3, l2allocationE }, { 3, l2allocationE },
{ 3, l2allocationE }, { 3, l2allocationE }, { 3, l2allocationE },
{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
};

Layer2BitAllocationTableEntry Layer2AllocationB1[32] = {
{ 4, l2allocationF }, { 4, l2allocationF }, { 4, l2allocationF },
{ 4, l2allocationF }, { 3, l2allocationE }, { 3, l2allocationE },
{ 3, l2allocationE }, { 3, l2allocationE }, { 3, l2allocationE },
{ 3, l2allocationE }, { 3, l2allocationE }, { 2, l2allocationE },
{ 2, l2allocationE }, { 2, l2allocationE }, { 2, l2allocationE },
{ 2, l2allocationE }, { 2, l2allocationE }, { 2, l2allocationE },
{ 2, l2allocationE }, { 2, l2allocationE }, { 2, l2allocationE },
{ 2, l2allocationE }, { 2, l2allocationE }, { 2, l2allocationE },
{ 2, l2allocationE }, { 2, l2allocationE }, { 2, l2allocationE },
{ 2, l2allocationE }, { 2, l2allocationE }, { 2, l2allocationE },
{0,0},{0,0}
};

void DecompressMpeg::Layer2Decode() {
	int allocation[2][32]; 

	ResetBits();
	Layer2BitAllocationTableEntry* allocationMap;
	long bitRatePerChannel = _bitRate / _channels;
	if (_id == 0) 
		allocationMap = Layer2AllocationB1;
	else if (bitRatePerChannel <= 48000) {
		if (_samplingRate == 32000) allocationMap =
			Layer2AllocationB2d;
		else allocationMap = Layer2AllocationB2c;
	}
	else if (bitRatePerChannel < 96000)
		allocationMap = Layer2AllocationB2a;
	else if (_samplingRate == 48000)
		allocationMap = Layer2AllocationB2a;
	else
		allocationMap = Layer2AllocationB2b;

	int sblimit = 0; 
	{ 
		for (int sb = 0; sb < _bound; sb++) {
			if (allocationMap[sb]._numberBits) {
				allocation[0][sb] =
					GetBits(allocationMap[sb]._numberBits);
				allocation[1][sb] =
					GetBits(allocationMap[sb]._numberBits);
				if ((allocation[0][sb] || allocation[1][sb])
					&& (sb >= sblimit))
					sblimit = sb + 1;
			}
			else {
				allocation[0][sb] = 0;
				allocation[1][sb] = 0;
			}
		}
	}
	{ 
		for (int sb = _bound; sb < 32; sb++) {
			if (allocationMap[sb]._numberBits) {
				allocation[0][sb] =
					GetBits(allocationMap[sb]._numberBits);
				if (allocation[0][sb] && (sb >= sblimit))
					sblimit = sb + 1;
			}
			else allocation[0][sb] = 0;
			allocation[1][sb] = allocation[0][sb];
		}
	}

	int scaleFactor[3][2][32];
	long sbSamples[3][2][32];
	int scaleFactorSelection[2][32];
	{ 
		for (int sb = 0; sb < sblimit; sb++)
			for (int ch = 0; ch < _channels; ch++)
				if (allocation[ch][sb] != 0)
					scaleFactorSelection[ch][sb] = GetBits(2);
	}
	{ 
		for (int sb = 0; sb < sblimit; sb++)
			for (int ch = 0; ch < _channels; ch++)
				if (allocation[ch][sb] != 0) {
					switch (scaleFactorSelection[ch][sb]) {
					case 0: 
						scaleFactor[0][ch][sb] = GetBits(6);
						scaleFactor[1][ch][sb] = GetBits(6);
						scaleFactor[2][ch][sb] = GetBits(6);
						break;
					case 1: 
						scaleFactor[0][ch][sb] = GetBits(6);
						scaleFactor[1][ch][sb] = scaleFactor[0][ch][sb];
						scaleFactor[2][ch][sb] = GetBits(6);
						break;
					case 2: 
						scaleFactor[0][ch][sb] = GetBits(6);
						scaleFactor[1][ch][sb] = scaleFactor[0][ch][sb];
						scaleFactor[2][ch][sb] = scaleFactor[0][ch][sb];
						break;
					case 3: 
						scaleFactor[0][ch][sb] = GetBits(6);
						scaleFactor[1][ch][sb] = GetBits(6);
						scaleFactor[2][ch][sb] = scaleFactor[1][ch][sb];
						break;
					}
				}
	}

	for (int sf = 0; sf < 3; sf++) { 
		for (int gr = 0; gr < 4; gr++) {
			for (int sb = 0; sb < sblimit; sb++) {
				for (int ch = 0; ch < _channels; ch++) {
					if ((sb >= _bound) && (ch == 1)) {
						sbSamples[0][1][sb] = sbSamples[0][0][sb];
						sbSamples[1][1][sb] = sbSamples[1][0][sb];
						sbSamples[2][1][sb] = sbSamples[2][0][sb];
						continue;
					}
					Layer2QuantClass* quantClass
						= allocationMap[sb]._quantClasses ?
						allocationMap[sb]._quantClasses[
							allocation[ch][sb]]
						: 0;
							if (!allocation[ch][sb]) { 
								sbSamples[0][ch][sb] = 0;
								sbSamples[1][ch][sb] = 0;
								sbSamples[2][ch][sb] = 0;
							}
							else if (quantClass->_grouping) { 
						
								long s = GetBits(quantClass->_bits); 
		
								sbSamples[0][ch][sb]
									= Layer2Requant(s % quantClass->_levels, quantClass,
										scaleFactor[sf][ch][sb]);
								s /= quantClass->_levels;
								sbSamples[1][ch][sb]
									= Layer2Requant(s % quantClass->_levels, quantClass,
										scaleFactor[sf][ch][sb]);
								s /= quantClass->_levels;
								sbSamples[2][ch][sb]
									= Layer2Requant(s % quantClass->_levels, quantClass,
										scaleFactor[sf][ch][sb]);
							}
							else { 
								int width = quantClass->_bits;
								long s = GetBits(width);
								sbSamples[0][ch][sb]
									=
									Layer2Requant(s, quantClass, scaleFactor[sf][ch][sb]);
								s = GetBits(width); 
								sbSamples[1][ch][sb]
									= Layer2Requant(s, quantClass, scaleFactor[sf][ch][sb]);
								s = GetBits(width); 
								sbSamples[2][ch][sb]
									=
									Layer2Requant(s, quantClass, scaleFactor[sf][ch][sb]);
							}
				}
			}

			for (int ch = 0; ch < _channels; ch++) {
				Layerl2Synthesis(_V[ch], sbSamples[0][ch], sblimit, _pcmSamples[ch]);
				_pcmSamples[ch] += 32;
				Layerl2Synthesis(_V[ch], sbSamples[1][ch], sblimit, _pcmSamples[ch]);
				_pcmSamples[ch] += 32;
				Layerl2Synthesis(_V[ch], sbSamples[2][ch], sblimit, _pcmSamples[ch]);
				_pcmSamples[ch] += 32;
			}
			_samplesRemaining += 96;
		}
	}
}

void DecompressMpeg::Layer3Decode() {
	cerr << "I don't support MPEG Layer 3 decompression.\n";
	exit(1);
}

DecompressMpeg::DecompressMpeg(AudioAbstract& a, std::ostringstream& out)
	:AbstractDecompressor(a) {
	_samplesRemaining = 0;
	_buffer = _bufferStorage + sizeof(_bufferStorage);
	_header = _bufferEnd = _buffer;
	_headerSpacing = 0;

	for (int ch = 0; ch < 2; ch++) { 
		for (int i = 0; i < 16; i++) {
			_V[ch][i] = new long[64];
			for (int j = 0; j < 64; j++)
				_V[ch][i][j] = 0;
		}
	}

	if (!layerlScaleFactors) {
		layerlScaleFactors = new long[63];
		for (int i = 0; i < 63; i++) {
			layerlScaleFactors[i] = static_cast<long>
				(32767.0 * pow(2.0, 1.0 - i / 3.0));
		}
	}

	NextFrame(); 
	out << "Encoding: MPEG-" << ((_id == 0) ? "2" : "1") << "\n";
	out << "Layer " << static_cast<unsigned int>(_layer);
	out << "\n";
	out << "Sampling Rate: " << static_cast<unsigned int>(_samplingRate) << "\n";
	switch (_mode) {
	case 0: out << "Mode: Stereo\n"; break;
	case 1: out << "Mode: Joint Stereo\n"; break;
	case 2: out << "Mode: Dual Channel\n"; break;
	case 3: out << "Mode: Single Channel\n"; break;
	}
	out << "Bitrate: " << static_cast<unsigned int>(_bitRate) << "\n";
	switch (_emphasis) {
	case 0: out << "Emphasis: none\n"; break;
	case 1: out << "Emphasis: 50/15\n"; break;
	case 2: out << "Emphasis: reserved\n"; break;
	case 3: out << "Emphasis: ITU J.17\n"; break;
	}
	out << "Approximate Compression Ratio: " << _samplingRate * 16.0 * _channels / _bitRate << "\n";
}

DecompressMpeg::~DecompressMpeg() {
	for (int ch = 0; ch < 2; ch++)
		for (int i = 0; i < 16; i++)
			delete[] _V[ch][i];

	delete layerlScaleFactors;
	layerlScaleFactors = 0;
}

bool IsMpegFile(istream& file) {
	file.seekg(0); 
	long magic = ReadIntMsb(file, 2);
	if ((magic & 0xFFF0) == 0xFFF0) return true;
	else return false;
}
MpegRead::MpegRead(istream& input, std::ostringstream& log) :AudioAbstract(), _stream(input)
{
	log << "File Format: MPEG\n";
	_decoder = new DecompressMpeg(*this, log);
}

MpegRead::~MpegRead() {
	if (_decoder) delete _decoder;
}

size_t MpegRead::GetSamples(AudioSample* buffer, size_t
	numSamples) {
	return _decoder->GetSamples(buffer, numSamples);
}

size_t MpegRead::ReadBytes(AudioByte* buffer, size_t length) {
	_stream.read(reinterpret_cast<char*>(buffer), length);
	return _stream.gcount();
}

void MpegRead::MinMaxChannels(int* min, int* max, int* preferred)
{
	_decoder->MinMaxChannels(min, max, preferred);
}

void MpegRead::MinMaxSamplingRate(long* min, long* max, long
	* preferred) {
	_decoder->MinMaxSamplingRate(min, max, preferred);
}