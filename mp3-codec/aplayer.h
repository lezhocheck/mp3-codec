#pragma once
#include "audio.h"

class AbstractPlayer : public AudioAbstract {
protected:
	typedef short Sample16;
	typedef signed char Sample8;

	volatile AudioSample* _queue, * _queueEnd; // Begin/end of queue memory
	volatile AudioSample* volatile _queueFirst;  // First sample
	volatile AudioSample* volatile _queueLast; // Last sample

	void InitializeQueue(unsigned long queueSize); // Create Queue
	void FillQueue(void); // Fill it up
	long FromQueue(Sample8* pDest, long bytes);
	long FromQueue(Sample16* pDest, long bytes);
private:
	void DataToQueue(long); // Used by FillQueue
	void DataFromQueue(Sample8*, long); // Used by FromQueue(Sample8...)
	void DataFromQueue(Sample16*, long); // Used by FromQueue(Sample16...)
private:
	size_t GetSamples(AudioSample*, size_t) { exit(1); return 0; };
protected:
	bool _endOfSource; // true -> last data read from source
	bool _endOfQueue; // true -> last data read from queue
public:
	AbstractPlayer(AudioAbstract* a);
	~AbstractPlayer();
	virtual void Play() = 0;  // Actually play the sound source
};