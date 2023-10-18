#include "audio.h"

void AudioAbstract::NegotiateSamplingRate(void) {
    if (Next()) // Are we the leftmost?
        Next()->NegotiateSamplingRate(); // No, keep goin
    else { // Yes, we are
        long min = 8000, max = 44100, preferred = 44100;
        MinMaxSamplingRate(&min, &max, &preferred); // Get preferred values
        if (min > max) { // Check for rediculous answers
            cerr << "Couldn't negotiate sampling rate.\n";
            exit(1);
        }
        SetSamplingRateRecursive(preferred); // Set them everywhere
    }
}
void AudioAbstract::MinMaxSamplingRate(long* min, long* max,
    long* preferred) {
    if (Previous()) Previous()->MinMaxSamplingRate(min, max, preferred);
    if (_samplingRate) *preferred = _samplingRate;
    if (*preferred < *min) *preferred = *min;
    if (*preferred > *max) *preferred = *max;
}

void AudioAbstract::SetSamplingRateRecursive(long s) {
    if (Previous()) // Set towards the right first
        Previous()->SetSamplingRateRecursive(s);
    SamplingRate(s); // Set it
    _samplingRateFrozen = true; // Yes, we've negotiated
}
void AudioAbstract::NegotiateChannels(void) {
    if (Next())
        Next()->NegotiateChannels();
    else {
        int min = 1, max = 2, preferred = 1; // Some reasonable default
        MinMaxChannels(&min, &max, &preferred);
        if (min > max) {
            cerr << "Couldn't negotiate sampling rate.\n";
            exit(1);
        }
        SetChannelsRecursive(preferred);
    }
}

void AudioAbstract::MinMaxChannels(int* min, int* max, int* preferred) {
    if (Previous())  Previous()->MinMaxChannels(min, max, preferred);
    if (_channels) *preferred = _channels;
    if (*preferred < *min) *preferred = *min;
    if (*preferred > *max) *preferred = *max;
}

void AudioAbstract::SetChannelsRecursive(int ch) {
    if (Previous()) Previous()->SetChannelsRecursive(ch);
    Channels(ch);
    _channelsFrozen = true;
}
long ReadIntMsb(istream& in, int size) {
    if (size <= 0) return 0;
    long l = ReadIntMsb(in, size - 1) << 8;
    l |= static_cast<long>(in.get()) & 255;
    return l;
}

long BytesToIntMsb(void* vBuff, int size) {
    unsigned char* buff = reinterpret_cast<unsigned char*>(vBuff);
    if (size <= 0) return 0;
    long l = BytesToIntMsb(buff, size - 1) << 8;
    l |= static_cast<long>(buff[size - 1]) & 255;
    return l;
}

long ReadIntLsb(istream& in, int size) {
    if (size <= 0) return 0;
    long l = static_cast<long>(in.get()) & 255;
    l |= ReadIntLsb(in, size - 1) << 8;
    return l;
}

long BytesToIntLsb(void* vBuff, int size) {
    unsigned char* buff = reinterpret_cast<unsigned char*>(vBuff);
    if (size <= 0) return 0;
    long l = static_cast<long>(*buff) & 255;
    l |= BytesToIntLsb(buff + 1, size - 1) << 8;
    return l;
}

void SkipBytes(istream& in, int size) {
    while (size-- > 0)
        in.get();
}
void WriteIntMsb(ostream& out, long l, int size) {
    if (size <= 0) return;
    WriteIntMsb(out, l >> 8, size - 1); // Write MS Bytes
    out.put(l & 255); // Write LS Byte
}

void WriteIntLsb(ostream& out, long l, int size) {
    if (size <= 0) return;
    out.put(l & 255);  // Write LS Byte
    WriteIntLsb(out, l >> 8, size - 1); // Write rest
}
