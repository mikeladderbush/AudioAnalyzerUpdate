#include <iostream>
#include <fstream>
#include <vector>
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

    char header[4];
    file.read(header, 4);

    if (header[0] == 'I' && header[1] == 'D' && header[2] == '3')
    {
        std::cout << "Skipping ID3 Tag" << std::endl;
        file.seekg(10, std::ios::cur);
    }
    else
    {
        file.seekg(0);
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
