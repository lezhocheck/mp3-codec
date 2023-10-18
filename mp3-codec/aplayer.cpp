#include "aplayer.h"

AbstractPlayer::AbstractPlayer(AudioAbstract* a) : AudioAbstract(a) {
    _endOfSource = false;
    _endOfQueue = false;
    _queue = _queueEnd = _queueFirst = _queueLast = 0;
}

AbstractPlayer::~AbstractPlayer(void) {
    if (_queue)
        // MS VC++ 5.0 can't delete a volatile pointer?!?!
        delete[] const_cast<AudioSample*>(_queue);
}
void AbstractPlayer::InitializeQueue(unsigned long queueSize) {
    _queue = new AudioSample[queueSize];
    _queueEnd = _queue + queueSize;
    _queueFirst = _queueLast = _queue;

    FillQueue();
}
void AbstractPlayer::FillQueue() {
    if (!_endOfSource && (_queueLast >= _queueFirst)) {
        if (_queueFirst == _queue) // Don't fill buffer
            DataToQueue(_queueEnd - _queueLast - 1);
        else
            DataToQueue(_queueEnd - _queueLast);
    }
    if (!_endOfSource && (_queueFirst > (_queueLast + 1)))
        DataToQueue(_queueFirst - _queueLast - 1);
}

void AbstractPlayer::DataToQueue(long samplesNeeded) {
    long samplesRead;
    volatile AudioSample* pDest = _queueLast;

    // Make sure request is a multiple of channels
    samplesNeeded -= samplesNeeded % Channels();

    samplesRead = Previous()->GetSamples(
        const_cast<AudioSample*>(pDest), samplesNeeded);
    pDest += samplesRead;
    if (pDest >= _queueEnd) pDest = _queue;
    _queueLast = pDest;
    if (samplesRead < samplesNeeded)
        _endOfSource = true;
}
long AbstractPlayer::FromQueue(Sample16* pDest, long destSize) {
    long destRemaining = destSize;

    if (_queueLast < _queueFirst) {
        int copySize = _queueEnd - _queueFirst; // Number samples avail
        if (copySize > destRemaining)
            copySize = destRemaining;
        DataFromQueue(pDest, copySize);
        destRemaining -= copySize;
        pDest += copySize;
    }

    if ((destRemaining > 0) && (_queueLast > _queueFirst)) {
        int copySize = _queueLast - _queueFirst;
        if (copySize > destRemaining)
            copySize = destRemaining;
        DataFromQueue(pDest, copySize);
        destRemaining -= copySize;
        pDest += copySize;
    }

    if ((destRemaining > 0) && _endOfSource)
        _endOfQueue = true;

    return (destSize - destRemaining);
};

long AbstractPlayer::FromQueue(Sample8* pDest, long destSize) {
    long destRemaining = destSize;

    if (_queueLast < _queueFirst) {
        int copySize = _queueEnd - _queueFirst; // Number samples avail
        if (copySize > destRemaining)
            copySize = destRemaining;
        DataFromQueue(pDest, copySize);
        destRemaining -= copySize;
        pDest += copySize;
    }

    if ((destRemaining > 0) && (_queueLast > _queueFirst)) {
        int copySize = _queueLast - _queueFirst;
        if (copySize > destRemaining)
            copySize = destRemaining;
        DataFromQueue(pDest, copySize);
        destRemaining -= copySize;
        pDest += copySize;
    }

    if ((destRemaining > 0) && _endOfSource)
        _endOfQueue = true;

    return (destSize - destRemaining);
};
/* private: */
void AbstractPlayer::DataFromQueue(Sample16* pDest, long copySize) {
    volatile AudioSample* newQueueFirst = _queueFirst;
    for (int i = 0; i < copySize; i++)
        *pDest++ = *newQueueFirst++
        >> ((sizeof(*newQueueFirst) - sizeof(*pDest)) * 8);
    if (newQueueFirst >= _queueEnd)
        newQueueFirst = _queue;
    _queueFirst = newQueueFirst;
}

/* private: */
void AbstractPlayer::DataFromQueue(Sample8* pDest, long copySize) {
    volatile AudioSample* newQueueFirst = _queueFirst;
    for (int i = 0; i < copySize; i++)
        *pDest++ = *newQueueFirst++
        >> ((sizeof(*newQueueFirst) - sizeof(*pDest)) * 8);
    if (newQueueFirst >= _queueEnd)
        newQueueFirst = _queue;
    _queueFirst = newQueueFirst;
}
