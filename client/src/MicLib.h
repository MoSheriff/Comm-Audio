#ifndef MIC_LIB_H
#define MIC_LIB_H

#include <mmsystem.h>

class MicLib 
{
public:
    MicLib(NetworkingComponent *nc);
    void record();
    void stop();

private:
    static const size_t BLOCKSIZE = 8192;
    static const size_t BLOCKCOUNT = 20;
    NetworkingComponent *netcomp;
    WAVEFORMATEX micFormat_;
};

#endif