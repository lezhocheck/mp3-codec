#pragma once
#include "audio.h"
#include "compress.h"
#include<sstream>


class DecompressMpeg : public AbstractDecompressor {
private:
	void FillBuffer(); // ��������� ������� ������
	// � ����������� ���������.
	void ResetBits(); // ���������� �������� ���������� �����.
	long GetBits(int numBits); // ���������� �����.
	AudioByte _bufferStorage[2048];
	AudioByte* _buffer; // ������ ������� ��������
	// ������ � ������.
	int _bitsRemaining; // �������� ����� � ������� �����.
	AudioByte* _bufferEnd; // ����� ������� �������� ������
	// � ������.
	AudioByte* _header; // ��������� ��������� � ������.
	bool ParseHeader(); // ������ ��������� ���������� ������.
	char _id; // 1 ��� MPEG-1, 0 ��� ���������� MPEG-2.
	char _layer;
	char _protection; // 1, ���� �������� CRC �� ������������.
	long _bitRate; // ����� ���������� ����� � �������.
	long _samplingRate; // ������� � �������.
	bool _padding; // � ����� ������ ����
	// �������������� ��������.
	char _private; // ������������� ���.
	char _mode; // �������������, �������������,
	// ������ � �.�.
	char _modeExtension; // ��� ��������� ������.
	char _bound; // �����������, � ������� ��������
	// ��� ��������� ������.
	bool _copyright; // ������, ���� ���� ��������� �����.
	bool _original; // ������, ���� ��������.
	char _emphasis; // ������� ������������� �����.
	int _channels; // ���������� �������.
	int _headerSpacing; // B ������.

	long* _V[2][16]; // ������������� �������� ��� ������/
	// ������� �������.
	void Layerl2Synthesis(long* V[16], long* in, int inSamples,	AudioSample* out);
	void LayerlDecode(); // ������������ ������ ������� Layer1.
	void Layer2Decode(); // ������������ ������ ������� Layer2.
	void Layer3Decode(); 
	
	AudioSample _sampleStorage[2][1152];
	AudioSample* _pcmSamples[2]; // ������� ��� ������/������� �������,

	int _samplesRemaining; // ���������� ����������
	// �� ���������� ������ ��������.
	void NextFrame(); // ������ � ������������
	// ���������� ������.
public:
	DecompressMpeg(AudioAbstract& a, std::ostringstream& cerr);
	~DecompressMpeg();
	size_t GetSamples(AudioSample* outBuff, size_t len);
	void MinMaxSamplingRate(long* min, long* max, long* preferred)
	{
		*min = *max = *preferred = _samplingRate;
	};
	void MinMaxChannels(int* min, int* max, int* preferred) {
		*min = *max = *preferred = _channels;
	};
};

bool IsMpegFile(istream& file);

class MpegRead : public AudioAbstract {
public:
	istream& _stream;
	AbstractDecompressor* _decoder;
public:
	MpegRead(istream& input, std::ostringstream& log);
	~MpegRead();
	size_t GetSamples(AudioSample* buffer, size_t numSamples);
	size_t ReadBytes(AudioByte* buffer, size_t length);
	void MinMaxSamplingRate(long* min, long* max, long
		* preferred);
	void MinMaxChannels(int* min, int* max, int* preferred);
};