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
        std::cout << currentFrame.calculatedFrameSize << std::endl;
        frameVector.push_back(currentFrame);

        file.seekg(currentFrame.calculatedFrameSize - 4, std::ios::cur);
    }

    file.close();

    return 0;
}
