#pragma once
#include "audio.h"
#include "compress.h"
#include<sstream>


class DecompressMpeg : public AbstractDecompressor {
private:
	void FillBuffer(); // Поддержка буферов данных
	// в заполненном состоянии.
	void ResetBits(); // Перезапуск процесса извлечения битов.
	long GetBits(int numBits); // Извлечение битов.
	AudioByte _bufferStorage[2048];
	AudioByte* _buffer; // Начало области активных
	// данных в буфере.
	int _bitsRemaining; // Осталось битов в старшем байте.
	AudioByte* _bufferEnd; // Конец области активных данных
	// в буфере.
	AudioByte* _header; // Положение заголовка в буфере.
	bool ParseHeader(); // Разбор заголовка очередного фрейма.
	char _id; // 1 для MPEG-1, 0 для расширений MPEG-2.
	char _layer;
	char _protection; // 1, если проверка CRC не производится.
	long _bitRate; // Общее количество битов в секунду.
	long _samplingRate; // Отсчеты в секунду.
	bool _padding; // У этого пакета есть
	// дополнительная страница.
	char _private; // Неофициальный бит.
	char _mode; // Одноканальный, двухканальный,
	// стерео и т.д.
	char _modeExtension; // Тип кодировки стерео.
	char _bound; // Поддиапазон, в котором меняется
	// тип кодировки стерео.
	bool _copyright; // Истина, если есть авторские права.
	bool _original; // Истина, если оригинал.
	char _emphasis; // Порядок постобработки аудио.
	int _channels; // Количество каналов.
	int _headerSpacing; // B байтах.

	long* _V[2][16]; // Синтезируемый интервал для левого/
	// правого каналов.
	void Layerl2Synthesis(long* V[16], long* in, int inSamples,	AudioSample* out);
	void LayerlDecode(); // Декомпрессия данных формата Layer1.
	void Layer2Decode(); // Декомпрессия данных формата Layer2.
	void Layer3Decode(); 
	
	AudioSample _sampleStorage[2][1152];
	AudioSample* _pcmSamples[2]; // Отсчеты для левого/правого каналов,

	int _samplesRemaining; // Количество оставшихся
	// от последнего фрейма отсчетов.
	void NextFrame(); // Чтение и декомпрессия
	// очередного фрейма.
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