#include <cstdlib>
#include "compress.h"
#include "imaadpcm.h"

static const int indexAdjustTable[16] = {
   -1, -1, -1, -1,  // +0 - +3, decrease the step size
    2, 4, 6, 8,     // +4 - +7, increase the step size
   -1, -1, -1, -1,  // -0 - -3, decrease the step size
    2, 4, 6, 8,     // -4 - -7, increase the step size
};
static const int _stepSizeTable[89] = {
   7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34,
   37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
   157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494,
   544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552,
   1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026,
   4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442,
   11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623,
   27086, 29794, 32767
};
AudioSample ImaAdpcmDecode(AudioByte deltaCode, ImaState& state) {
    // Get the current step size
    int step = _stepSizeTable[state.index];

    // Construct the difference by scaling the current step size
    // This is approximately: difference = (deltaCode+.5)*step/4
    int difference = step >> 3;
    if (deltaCode & 1) difference += step >> 2;
    if (deltaCode & 2) difference += step >> 1;
    if (deltaCode & 4) difference += step;
    if (deltaCode & 8) difference = -difference;

    // Build the new sample
    state.previousValue += difference;
    if (state.previousValue > 32767) state.previousValue = 32767;
    else if (state.previousValue < -32768) state.previousValue = -32768;

    // Update the step for the next sample
    state.index += indexAdjustTable[deltaCode];
    if (state.index < 0) state.index = 0;
    else if (state.index > 88) state.index = 88;

    return state.previousValue;
}
AudioByte ImaAdpcmEncode(AudioSample sample, ImaState& state) {
    int diff = sample - state.previousValue;
    int step = _stepSizeTable[state.index];
    int deltaCode = 0;

    // Set sign bit
    if (diff < 0) { deltaCode = 8; diff = -diff; }

    // This is essentially deltaCode = (diff<<2)/step,
    // except the roundoff is handled differently.
    if (diff >= step) { deltaCode |= 4;  diff -= step; }
    step >>= 1;
    if (diff >= step) { deltaCode |= 2;  diff -= step; }
    step >>= 1;
    if (diff >= step) { deltaCode |= 1;  diff -= step; }

    ImaAdpcmDecode(deltaCode, state);  // update state
    return deltaCode;
}
DecompressImaAdpcmMs::DecompressImaAdpcmMs
(AudioAbstract& a, int packetLength, int channels)
    : AbstractDecompressor(a) {
    cerr << "Encoding: IMA ADPCM (Microsoft variant)";
    if (channels == 2) { cerr << " (stereo)"; }
    cerr << "\n";
    _channels = channels;
    _samplesPerPacket = packetLength;
    _bytesPerPacket = (_samplesPerPacket + 7) / 2 * channels;
    _packet = new AudioByte[_bytesPerPacket];
    _samples[1] = _samples[0] = 0;
    while (channels-- > 0)
        _samples[channels] = new AudioSample[_samplesPerPacket];
    _samplesRemaining = 0;
};

DecompressImaAdpcmMs::~DecompressImaAdpcmMs() {
    if (_samples[0]) delete[] _samples[0];
    if (_samples[1]) delete[] _samples[1];
    if (_packet) delete[] _packet;
};
size_t  DecompressImaAdpcmMs::NextPacket() {
    // Pull in the packet and check the header
    size_t bytesRead = ReadBytes(_packet, _bytesPerPacket);
    if (bytesRead < _bytesPerPacket) { return 0; }
    AudioByte* bytePtr = _packet;

    // Reset the decompressor
    ImaState state[2];  // One decompressor state for each channel
    // Read the four-byte header for each channel
    for (int ch = 0; ch < _channels; ch++) {
        state[ch].previousValue =
            static_cast<signed char>(bytePtr[1]) * 0x100
            + static_cast<signed char>(bytePtr[0]);

        if (bytePtr[2] > 88)
            cerr << "IMA ADPCM Format Error (bad index value)\n";
        else
            state[ch].index = bytePtr[2];
        if (bytePtr[3])
            cerr << "IMA ADPCM Format Error (synchronization error)\n";
        bytePtr += 4; // Skip this header

        _samplePtr[ch] = _samples[ch];
        // Decode one sample for the header
        *_samplePtr[ch]++ = state[ch].previousValue;
    }

    // Decompress nybbles
    size_t remaining = _samplesPerPacket - 1;

    while (remaining > 0) {
        remaining -= 8;

        int i;
        // Decode 8 left samples
        for (i = 0; i < 4; i++) {
            AudioByte b = *bytePtr++;
            *_samplePtr[0]++ = ImaAdpcmDecode(b & 0xF, state[0]);
            *_samplePtr[0]++ = ImaAdpcmDecode((b >> 4) & 0xF, state[0]);
        }
        if (_channels < 2)
            continue; // If mono, skip rest of loop
        // Decode 8 right samples
        for (i = 0; i < 4; i++) {
            AudioByte b = *bytePtr++;
            *_samplePtr[1]++ = ImaAdpcmDecode(b & 0xF, state[1]);
            *_samplePtr[1]++ = ImaAdpcmDecode((b >> 4) & 0xF, state[1]);
        }
    };
    return _samplesPerPacket;
};
size_t DecompressImaAdpcmMs::GetSamples(AudioSample* outBuff, size_t len) {
    size_t wanted = len;
    while (wanted > 0) { // Still want data?
        if (_samplesRemaining == 0) { // Need to read more from disk?
            _samplesRemaining = NextPacket();
            if (_samplesRemaining == 0) { return len - wanted; }
            _samplePtr[0] = _samples[0];
            _samplePtr[1] = _samples[1];
        }
        switch (_channels) { // Copy data into outBuff
        case 1: // Mono: Just copy left channel data
            while ((_samplesRemaining > 0) && (wanted > 0)) {
                *outBuff++ = *_samplePtr[0]++;
                _samplesRemaining--;
                wanted--;
            }
            break;
        case 2: // Stereo: Interleave samples
            while ((_samplesRemaining > 0) && (wanted > 0)) {
                *outBuff++ = *_samplePtr[0]++; // left
                *outBuff++ = *_samplePtr[1]++; // right
                _samplesRemaining--;
                wanted -= 2;
            }
            break;
        default: exit(1);
        }
    }
    return len - wanted;
}
DecompressImaAdpcmApple::DecompressImaAdpcmApple
(AudioAbstract& a, int channels) : AbstractDecompressor(a) {
    cerr << "Encoding: IMA ADPCM (Apple variant)";
    if (channels == 2) { cerr << " (stereo)"; }
    cerr << "\n";
    _samplesRemaining = 0;
    _state[0].previousValue = 0;
    _state[0].index = 0;
    _state[1] = _state[0];
    _channels = channels;
};
size_t  DecompressImaAdpcmApple::NextPacket(AudioSample* sampleBuffer,
    ImaState& state) {
    AudioByte _packet[34];
    // Pull in the packet and check the header
    size_t bytesRead = ReadBytes(_packet, 34);
    if (bytesRead < 34) return 0;

    // Check the decompressor state
    state.index = _packet[1] & 0x7F;
    if (state.index > 88) {
        cerr << "Synchronization error.\n";
        exit(1);
    }

    // Recover upper nine bits of last sample
    AudioSample lastSample =
        (static_cast<signed char>(_packet[0]) * 0x100 +
            static_cast<signed char>(_packet[1])) & 0xFF80;

    // If ours doesn't match, reset to the value in the file
    if ((state.previousValue & 0xFF80) != lastSample)
        state.previousValue = lastSample;

    // Decompress nybbles
    for (int i = 0; i < 32; i++) {
        *sampleBuffer++ = ImaAdpcmDecode(_packet[i + 2] & 0xF, state);
        *sampleBuffer++ = ImaAdpcmDecode((_packet[i + 2] >> 4) & 0xF, state);
    }
    return 64;
};
size_t DecompressImaAdpcmApple::GetSamples(AudioSample* outBuff,
    size_t wanted) {
    size_t remaining = wanted;
    while (remaining > 0) {
        if (_samplesRemaining == 0) {
            for (int i = 0; i < _channels; i++) {
                _samplesRemaining = NextPacket(_samples[i], _state[i]);
                if (_samplesRemaining == 0) { return wanted - remaining; }
                _samplePtr[i] = _samples[i];
            }
        }
        switch (_channels) {
        case 1:
            while ((_samplesRemaining > 0) && (remaining > 0)) {
                *outBuff++ = *_samplePtr[0]++;
                _samplesRemaining--;
                remaining--;
            }
            break;
        case 2:
            while ((_samplesRemaining > 0) && (remaining > 0)) {
                *outBuff++ = *_samplePtr[0]++; // Left
                *outBuff++ = *_samplePtr[1]++; // Right
                _samplesRemaining--;
                remaining -= 2;
            }
            break;
        }
    }
    return wanted - remaining;
};
