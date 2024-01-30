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

    char ID3Header[10];
    file.read(ID3Header, 10);
    unsigned int ID3Size = calculateID3Size(ID3Header);
    file.seekg(ID3Size + 10, std::ios::beg);

    // TODO: Now that I have skipped over the ID3 tag and can locate a syncword/frame header I have to
    // decode a frame in order to really get an idea of its size.
    while (!file.eof())
    {
        currentFrame = breakDownHeader(file);
        frameVector.push_back(currentFrame);

        // TODO: add function to read over the frameheader and begin to decode the samples themselves.
    }

    file.close();

    return 0;
}
