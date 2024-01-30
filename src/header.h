#ifndef HEADER_H
#define HEADER_H

#include <fstream>
#include <vector>

struct frameHeader
{
    int mpegVersion;
    int layer;
    int protection;
    int bitrate;
    int samplingFrequency;
    int padding;
    int privateBit;
    int mode;
    int modeExtension;
    int copyright;
    int original;
    int emphasis;
    int calculatedFrameSize;
};

frameHeader breakDownHeader(std::ifstream &mp3fileStream);
int calculateID3Size(char *ID3Header);

#endif