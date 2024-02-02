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

    unsigned char ID3Header[10];
    file.read(reinterpret_cast<char *>(&ID3Header), 10);
    unsigned int ID3Size = calculateID3Size(reinterpret_cast<char *>(&ID3Header));
    file.seekg(ID3Size + 10, std::ios::beg);

    auto current_position = file.tellg();
    std::cout << current_position << std::endl;

    while (!file.eof())
    {
        unsigned char syncword[4];
        file.read(reinterpret_cast<char *>(&syncword), std::ios::cur);
        if ((syncword[0] == 0xFF) && ((syncword[1] & 0xE0) == 0xE0))
        {
            // Not quite syncing properly after ID3 Tag, will get there tomorrow, still sick.
            std::cout << "Test" << std::endl;
            file.seekg(-sizeof(syncword), std::ios::cur);
            frameHeader currentFrame = breakDownHeader(file);
            frameVector.push_back(currentFrame);
            file.seekg(currentFrame.calculatedFrameSize, std::ios::cur);
        }
        else
        {
            // Breaks for now just for testing purposes, will continue in the future.
            break;
        }
    }

    file.close();

    return 0;
}
