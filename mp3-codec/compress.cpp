#include "audio.h"
#include "compress.h"
#include <iostream>
#include <cstddef>

size_t DecompressPcm8Signed::GetSamples(AudioSample* buffer,
    size_t length) {
    AudioByte* byteBuff =
        reinterpret_cast<AudioByte*>(buffer);
    size_t samplesRead = ReadBytes(byteBuff, length);
    for (long i = samplesRead - 1; i >= 0; i--)
        buffer[i] = static_cast<AudioSample>(byteBuff[i])
        << ((sizeof(AudioSample) - 1) * 8);
    return samplesRead;
}
size_t DecompressPcm8Unsigned::GetSamples(AudioSample* buffer,
    size_t length) {
    AudioByte* byteBuff =
        reinterpret_cast<AudioByte*>(buffer);
    size_t samplesRead = ReadBytes(byteBuff, length);
    for (long i = samplesRead - 1; i >= 0; i--)
        buffer[i] = static_cast<AudioSample>(byteBuff[i] ^ 0x80)
        << ((sizeof(AudioSample) - 1) * 8);
    return samplesRead;
}
size_t DecompressPcm16MsbSigned::GetSamples(AudioSample* buffer,
    size_t length) {
    AudioByte* byteBuff =
        reinterpret_cast<AudioByte*>(buffer);
    size_t read = ReadBytes(byteBuff, length * 2) / 2;
    for (long i = read - 1; i >= 0; i--) {
        short s = static_cast<AudioSample>(byteBuff[2 * i]) << 8;
        s |= static_cast<AudioSample>(byteBuff[2 * i + 1]) & 255;
        buffer[i] = static_cast<AudioSample>(s)
            << ((sizeof(AudioSample) - 2) * 8);
    }
    return read;
}
size_t DecompressPcm16LsbSigned::GetSamples(AudioSample* buffer,
    size_t length) {
    AudioByte* byteBuff =
        reinterpret_cast<AudioByte*>(buffer);
    size_t read = ReadBytes(byteBuff, length * 2) / 2;
    for (long i = read - 1; i >= 0; i--) {
        short s = static_cast<AudioSample>(byteBuff[2 * i + 1]) << 8;
        s |= static_cast<AudioSample>(byteBuff[2 * i]) & 255;
        buffer[i] = static_cast<AudioSample>(s)
            << ((sizeof(AudioSample) - 2) * 8);
    }
    return read;
}
