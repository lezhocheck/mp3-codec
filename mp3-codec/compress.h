#pragma once

#include "audio.h"
#include <iostream>
#include <cstddef>

class AbstractDecompressor {
protected:
    AudioAbstract& _dataSource;  // The object to get raw bytes from
    size_t ReadBytes(AudioByte* buff, size_t length) {
        return _dataSource.ReadBytes(buff, length);
    }
public:
    AbstractDecompressor(AudioAbstract& a) : _dataSource(a) { };
    virtual ~AbstractDecompressor() {};
    virtual size_t GetSamples(AudioSample*, size_t) = 0;
    virtual void MinMaxSamplingRate(long*, long*, long*) {
        throw std::invalid_argument("MinMaxSamplingRate undefined");
    }
    virtual void MinMaxChannels(int*, int*, int*) {
        throw std::invalid_argument("MinMaxChannels undefined");
    }
};

class DecompressPcm8Signed : public AbstractDecompressor {
public:
    DecompressPcm8Signed(AudioAbstract& a) : AbstractDecompressor(a) {
    };
    size_t GetSamples(AudioSample* buffer, size_t length);
};
class DecompressPcm8Unsigned : public AbstractDecompressor {
public:
    DecompressPcm8Unsigned(AudioAbstract& a) : AbstractDecompressor(a) {
    };
    size_t GetSamples(AudioSample* buffer, size_t length);
};
class DecompressPcm16MsbSigned : public AbstractDecompressor {
public:
    DecompressPcm16MsbSigned(AudioAbstract& a) : AbstractDecompressor(a) {
    };
    size_t GetSamples(AudioSample* buffer, size_t length);
};
class DecompressPcm16LsbSigned : public AbstractDecompressor {
public:
    DecompressPcm16LsbSigned(AudioAbstract& a) : AbstractDecompressor(a) {
    };
    size_t GetSamples(AudioSample* buffer, size_t length);
};