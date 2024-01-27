#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include "header.h"

int main()
{
    std::string filename = "file_example_mp3_700KB.mp3";
    std::vector<frameHeader> frameVector;
    frameHeader currentFrame;

    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Error opening the MP3 file." << std::endl;
        return 1;
    }

    char id3Header[10];
    file.read(id3Header, 10);

    if (file && id3Header[0] == 'I' && id3Header[1] == 'D' && id3Header[2] == '3')
    {
        std::bitset<32> sizeBits((id3Header[6] << 21) | (id3Header[7] << 14) | (id3Header[8] << 7) | id3Header[9]);
        int id3DataSize = sizeBits.to_ulong();

        int totalId3Length = 10 + id3DataSize;

        std::cout << "Total length of ID3 information: " << totalId3Length << " bytes." << std::endl;

        file.seekg(totalId3Length, std::ios::beg);
    }
    else
    {
        file.seekg(0, std::ios::beg);
    }

    while (!file.eof())
    {
        char frameHeader[4];
        file.read(frameHeader, 4);

        if (!file)
        {
            if (file.eof())
            {
                break;
            }
            else
            {
                std::cerr << "Error Reading Frame Header." << std::endl;
                break;
            }
        }

        currentFrame = breakDownHeader(file);
        frameVector.push_back(currentFrame);

        file.seekg(currentFrame.calculatedFrameSize - 4, std::ios::cur);

        std::cout << "Bitrate: " << currentFrame.bitrate << std::endl;
    }

    file.close();

    return 0;
}
