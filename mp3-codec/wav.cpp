#include <cstdlib>
#include "audio.h"
#include "compress.h"
#include "imaadpcm.h"
#include "wav.h"
#include "g711.h"

#define ChunkName(a,b,c,d) (                 \
    ((static_cast<unsigned long>(a)&255)<<24)          \
  + ((static_cast<unsigned long>(b)&255)<<16)          \
  + ((static_cast<unsigned long>(c)&255)<<8)           \
  + ((static_cast<unsigned long>(d)&255)))

bool IsWaveFile(istream& file) {
    file.seekg(0); // Seek to beginning of file
    unsigned long form = ReadIntMsb(file, 4);
    if (form != ChunkName('R', 'I', 'F', 'F'))
        return false; // Not RIFF file
    SkipBytes(file, 4);  // Skip chunk size
    unsigned long type = ReadIntMsb(file, 4);
    if (type == ChunkName('W', 'A', 'V', 'E'))
        return true;
    return false; // RIFF file, but not WAVE file
}

void WaveRead::NextChunk(void) {
    if ((_currentChunk >= 0) && (!_chunk[_currentChunk].isContainer)) {
        unsigned long lastChunkSize = _chunk[_currentChunk].size;
        if (lastChunkSize & 1) {  // Is there padding?
            _chunk[_currentChunk].remaining++;
            lastChunkSize++; // Account for padding in the container update
        }
        SkipBytes(_stream, _chunk[_currentChunk].remaining); // Flush the chunk
        _currentChunk--;  // Drop chunk from the stack
        // Sanity check: containing chunk must be container
        if ((_currentChunk < 0) || (!_chunk[_currentChunk].isContainer)) {
            cerr << "Chunk contained in non-Container?!?!\n";
            exit(1);
        }
        // Reduce size of container
        if (_currentChunk >= 0) {
            // Sanity check: make sure container is big enough.
            // Also, avoid a really nasty underflow situation.
            if ((lastChunkSize + 8) > _chunk[_currentChunk].remaining) {
                cerr << "Error: Chunk is too large to fit in container!?!?\n";
                _chunk[_currentChunk].remaining = 0; // container is empty
            }
            else
                _chunk[_currentChunk].remaining -= lastChunkSize + 8;
        }
    }

    // There may be forms that are finished, drop them too
    while ((_currentChunk >= 0)  // there is a chunk
        && (_chunk[_currentChunk].remaining < 8)
        )
    {
        SkipBytes(_stream, _chunk[_currentChunk].remaining); // Flush it
        unsigned long lastChunkSize = _chunk[_currentChunk].size;
        _currentChunk--;  // Drop container chunk
        // Sanity check, containing chunk must be container
        if (!_chunk[_currentChunk].isContainer) {
            cerr << "Chunk contained in non-container?!?!\n";
            exit(1);
        }
        // Reduce size of container
        if (_currentChunk >= 0) {
            if ((lastChunkSize + 8) > _chunk[_currentChunk].remaining) {
                cerr << "Error in WAVE file: Chunk is too large to fit!?!?\n";
                lastChunkSize = _chunk[_currentChunk].remaining;
            }
            _chunk[_currentChunk].remaining -= lastChunkSize + 8;
        }
    }

    // Read the next chunk
    if (_stream.eof()) {
        _currentChunk = -1; // empty the stack
        return;
    }
    unsigned long type = ReadIntMsb(_stream, 4);
    unsigned long size = ReadIntLsb(_stream, 4);
    if (_stream.eof()) {
        _currentChunk = -1; // empty the stack
        return;
    }

    // Put this chunk on the stack
    _currentChunk++;
    _chunk[_currentChunk].type = type;
    _chunk[_currentChunk].size = size;
    _chunk[_currentChunk].remaining = size;
    _chunk[_currentChunk].isContainer = false;
    _chunk[_currentChunk].containerType = 0;


    if ((_currentChunk >= 0) &&
        (_chunk[0].type != ChunkName('R', 'I', 'F', 'F'))) {
        cerr << "Outermost chunk is not RIFF ?!?!\n";
        _currentChunk = -1;
        return;
    }
    if (type == ChunkName('R', 'I', 'F', 'F')) {
        _chunk[_currentChunk].isContainer = true;
        // Need to check size of container first.
        _chunk[_currentChunk].containerType = ReadIntMsb(_stream, 4);
        _chunk[_currentChunk].remaining -= 4;
        if (_currentChunk > 0) {
            cerr << "RIFF chunk seen at inner level?!?!\n";
        }
        return;
    }
    if (type == ChunkName('f', 'm', 't', ' ')) {
        if (_currentChunk != 1) {
            cerr << "FMT chunk seen at wrong level?!?!\n";
        }
        _formatData = new unsigned char[size + 2];
        _stream.read(reinterpret_cast<char*>(_formatData), size);
        _formatDataLength = _stream.gcount();
        _chunk[_currentChunk].remaining = 0;
        return;
    }
    if (type == ChunkName('d', 'a', 't', 'a')) {
        return;
    }

    char code[5] = "CODE";
    code[0] = (type >> 24) & 255;   code[1] = (type >> 16) & 255;
    code[2] = (type >> 8) & 255;   code[3] = (type) & 255;
    cerr << "Ignoring unrecognized `" << code << "' chunk\n";
}
void WaveRead::MinMaxSamplingRate(long* min, long* max, long* preferred) {
    InitializeDecompression();
    unsigned long samplingRate = BytesToIntLsb(_formatData + 4, 4);
    *max = *min = *preferred = samplingRate;
}

void WaveRead::MinMaxChannels(int* min, int* max, int* preferred) {
    InitializeDecompression();
    unsigned long channels = BytesToIntLsb(_formatData + 2, 2);
    *min = *max = *preferred = channels;
}

size_t WaveRead::GetSamples(AudioSample* buffer, size_t numSamples) {
    if (!_decoder) InitializeDecompression();
    return _decoder->GetSamples(buffer, numSamples);
}
void WaveRead::InitializeDecompression() {
    if (_decoder) return;

    // Make sure we've read the fmt chunk
    while (!_formatData) {
        NextChunk();
        if (_currentChunk < 0) {
            cerr << "No `fmt' chunk found?!?!\n";
            exit(1);
        }
    }

    // Select decompressor based on compression type
    unsigned long type = BytesToIntLsb(_formatData + 0, 2);


    if (type == 1) {  // PCM format
        unsigned long bitsPerSample = BytesToIntLsb(_formatData + 14, 2);
        if (bitsPerSample <= 8) {// Wave stores 8-bit data as unsigned
            _decoder = new DecompressPcm8Unsigned(*this);
        }
        else if (bitsPerSample <= 16) {// 16 bit data is signed
            _decoder = new DecompressPcm16LsbSigned(*this);
        }

    }
    if (type == 17) {  // IMA ADPCM format
        unsigned long bitsPerSample = BytesToIntLsb(_formatData + 14, 2);
        if (bitsPerSample != 4) {
            cerr << "IMA ADPCM requires 4 bits per sample, not ";
            cerr << bitsPerSample << "\n";
        }
        if (_formatDataLength < 20) {
            cerr << "IMA ADPCM requires additional decompression data.\n";
        }
        int packetLength = BytesToIntLsb(_formatData + 18, 2);
        int channels = BytesToIntLsb(_formatData + 2, 2);
        _decoder = new DecompressImaAdpcmMs(*this, packetLength, channels);
    }
    if (type == 6) {
        _decoder = new DecompressG711ALaw(*this);
    }
    if (type == 7) {
        _decoder = new DecompressG711MuLaw(*this);
    }
    if (type == 2) {
        cerr << "I don't support MS ADPCM compression.\n";
    }

    if (!_decoder) {
        cerr << "I don't support WAVE compression type " << type << "\n";
    }
}
size_t WaveRead::ReadBytes(AudioByte* buffer, size_t numBytes) {
    while (_chunk[_currentChunk].type != ChunkName('d', 'a', 't', 'a')) {
        NextChunk();
        if (_currentChunk < 0) {
            cerr << "I didn't find any sound data!?!?\n";
            return 0;
        }
    }
    if (numBytes > _chunk[_currentChunk].remaining)
        numBytes = _chunk[_currentChunk].remaining;
    _stream.read(reinterpret_cast<char*>(buffer), numBytes);
    numBytes = _stream.gcount();
    _chunk[_currentChunk].remaining -= numBytes;
    return numBytes;
}

WaveRead::WaveRead(istream& s) : _stream(s) {
    _decoder = 0;
    _formatData = 0;
    _formatDataLength = 0;

    _currentChunk = -1; // Empty the stack
    NextChunk();
    // Ensure first chunk is RIFF/WAVE container
    if ((_currentChunk != 0)
        || (_chunk[0].type != ChunkName('R', 'I', 'F', 'F'))
        || (_chunk[0].isContainer != true)
        || (_chunk[0].containerType != ChunkName('W', 'A', 'V', 'E'))
        ) {
        exit(1);
    }
    InitializeDecompression();
}

WaveRead::~WaveRead() {
    if (_decoder) delete _decoder;
    if (_formatData) { delete[] _formatData; }
}
