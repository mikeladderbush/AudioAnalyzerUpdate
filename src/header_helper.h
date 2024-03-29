#ifndef HEADER_HELPER_H
#define HEADER_HELPER_H

#include <bitset>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <string>

int calculateBitrate(int mpegVersion, int layer, std::bitset<4> bitrateIndex);
int calculateSampleRate(int mpegVersion, int layer);
int calculateFrameSize(int bitrate, int samplesPerFrame, int mpegVersion, std::bitset<2> frequency, std::bitset<1> paddingBit, int crcBits);
int calculateErrorCheckSize(std::bitset<1> protectionBit);
int calculateChannelMode(std::bitset<2> channelBit);
int calculateModeExtension(std::bitset<2> modeExtBit, int channelMode, std::bitset<2> layer);
int calculateID3Size(char *ID3Header);

#endif // HEADER_HELPER_H
