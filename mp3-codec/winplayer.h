#pragma once
#include "audio.h"
#include "aplayer.h"
#include <windows.h>
#include <mmsystem.h>

#define winBufferSize 10000  // Number of samples per buffer

class WinPlayer : public AbstractPlayer {
private:
    HWAVEOUT _device;   // Windows audio device to open
    volatile bool _paused; // true -> device is paused
    int   _sampleWidth; // width of data to output

    int SelectDevice(void); // Open a suitable device

    // Allow the callback to see our members 
    friend void CALLBACK WaveOutCallback(HWAVEOUT hwo, UINT uMsg,
        DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

    // The callback function defined above is just a wrapper that
    // invokes this method
    void NextBuff(WAVEHDR*);
public:
    WinPlayer(AudioAbstract* a) : AbstractPlayer(a) {
        _device = 0;
        _paused = true;
        _sampleWidth = 0;
    };
    ~WinPlayer() {};

    void Play();  // Actually play the sound source
};