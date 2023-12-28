#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <bitset>
#include <string>
#include <unordered_map>

int decodeMp3(std::string mp3file)
{

    std::ifstream mp3fileStream(mp3file, std::ios::binary);

    if (!mp3fileStream.is_open())
    {
        std::cerr << "Error opening the MP3 file." << std::endl;
        return 1;
    }

    char header[4];
    mp3fileStream.read(header, 4);

    if (header[0] == 'I' && header[1] == 'D' && header[2] == '3')
    {
        std::cout << "Skipping ID3 Tag" << std::endl;
        mp3fileStream.seekg(10, std::ios::cur);
    }
    else
    {
        mp3fileStream.seekg(0);
    }

    while (!mp3fileStream.eof())
    {
        char frameHeader[4];
        mp3fileStream.read(frameHeader, 4);

        uint32_t sync = (frameHeader[0] << 24) | (frameHeader[1] << 16) | (frameHeader[2] << 8) | frameHeader[3];

        if ((sync & 0xFFE00000) == 0xFFE00000)
        {
            // Going to use the bitrate index total for a switch statement involving the bitrate index.
            std::bitset<32> syncbits(sync);
            // Test output.
            // std::cout << syncbits << std::endl;

            // Extract the information from the frame header.
            // http://www.mp3-tech.org/programmer/frame_header.html
            // we already found the frame header sync bits so then we need to find:
            // The MPEG Audio version ID which is 2 bits,
            std::bitset<2> extractedMpegVersion((syncbits.to_ulong() >> 19) & 0b11);
            // Test output.
            // std::cout << extractedMpegVersion << std::endl;

            // The Layer description which is 2,
            std::bitset<2> extractedLayerDescription((syncbits.to_ulong() >> 17) & 0b11);
            // Test output.
            // std::cout << extractedLayerDescription << std::endl;

            // The Protection bit which is 1,
            std::bitset<1> extractedProtectionBit(syncbits.to_ulong() >> 16);
            // Test output.
            // std::cout << extractedProtectionBit << std::endl;

            // Then we need the Bitrate index which is 4 bits and is equal to a certain value in a table.
            std::bitset<4> extractedBitrateIndex((syncbits.to_ulong() >> 12) & 0b1111);
            // Test output.
            // std::cout << extractedBitrateIndex << std::endl;

            // Then we have 2 bits for the frequency index which we should also create a table for.
            std::bitset<2> extractedFrequencyIndex((syncbits.to_ulong() >> 10) & 0b11);
            // Test output.
            // std::cout << extractedFrequencyIndex << std::endl;

            // Here is the padding bit.
            std::bitset<1> extractedPaddingBit(syncbits.to_ulong() >> 9);
            // Test output.
            // std::cout << extractedPaddingBit << std::endl;

            // The Private bit. This one is only informative.
            std::bitset<1> extractedPrivateBit(syncbits.to_ulong() >> 8);
            // Test output.
            // std::cout << extractedPrivateBit << std::endl;

            // Then Channel Mode for 2 bits.
            std::bitset<2> extractedChannelMode((syncbits.to_ulong() >> 6) & 0b11);
            // Test output.
            // std::cout << extractedChannelMode << std::endl;

            // The next 2 is the Mode extension.
            std::bitset<2> extractedModeExtension((syncbits.to_ulong() >> 4) & 0b11);
            // Test output.
            // std::cout << extractedModeExtension << std::endl;

            // Then there is 1 bit for copyright.
            std::bitset<1> extractedCopyright(syncbits.to_ulong() >> 3);
            // Test output.
            // std::cout << extractedCopyright << std::endl;

            // 1 bit for original or not.
            std::bitset<1> extractedOriginal(syncbits.to_ulong() >> 2);
            // Test output.
            // std::cout << extractedOriginal << std::endl;

            // Finally there are 2 bits for emphasis, which is rarely used.
            std::bitset<2> extractedEmphasisBits((syncbits.to_ulong() >> 0) & 0b11);
            // Test output.
            // std::cout << extractedEmphasisBits << std::endl;

            // Each frame is 1152 samples.
            // We need to create a 5x16 table for our bitrates, this will dictate what will be passed to the next functions for decoding and decompressing.
            // The factors that matter in the table are the bitrate index, the version and the layer.

            // My unordered_map containing bitrates mentioned above.
            std::unordered_map<std::bitset<8>, std::string> bitrateMap;

            // Here I am going to create a separate variable that concatenates previous variables in order to better find our bitrate.
            std::bitset<8> bitrateIndexValue = (((syncbits.to_ulong() >> 12) & 0b1111) << ((syncbits.to_ulong() >> 17) & 0b11) << ((syncbits.to_ulong() >> 19) & 0b11));

            bitrateMap[std::bitset<8>("00000110")] = "Free";
            bitrateMap[std::bitset<8>("00001010")] = "Free";
            bitrateMap[std::bitset<8>("00001110")] = "Free";
            bitrateMap[std::bitset<8>("00000111")] = "Free";
            bitrateMap[std::bitset<8>("00001011")] = "Free";
            bitrateMap[std::bitset<8>("00001111")] = "Free";

            bitrateMap[std::bitset<8>("00010110")] = "8";
            bitrateMap[std::bitset<8>("00011010")] = "32";
            bitrateMap[std::bitset<8>("00011110")] = "32";
            bitrateMap[std::bitset<8>("00010111")] = "32";
            bitrateMap[std::bitset<8>("00011011")] = "32";
            bitrateMap[std::bitset<8>("00011111")] = "32";

            bitrateMap[std::bitset<8>("00100110")] = "16";
            bitrateMap[std::bitset<8>("00101010")] = "16";
            bitrateMap[std::bitset<8>("00101110")] = "48";
            bitrateMap[std::bitset<8>("00100111")] = "40";
            bitrateMap[std::bitset<8>("00101011")] = "48";
            bitrateMap[std::bitset<8>("00101111")] = "64";

            bitrateMap[std::bitset<8>("00110110")] = "24";
            bitrateMap[std::bitset<8>("00111010")] = "24";
            bitrateMap[std::bitset<8>("00111110")] = "56";
            bitrateMap[std::bitset<8>("00110111")] = "48";
            bitrateMap[std::bitset<8>("00111011")] = "56";
            bitrateMap[std::bitset<8>("00111111")] = "96";

            bitrateMap[std::bitset<8>("01000110")] = "32";
            bitrateMap[std::bitset<8>("01001010")] = "32";
            bitrateMap[std::bitset<8>("01001110")] = "64";
            bitrateMap[std::bitset<8>("01000111")] = "56";
            bitrateMap[std::bitset<8>("01001011")] = "64";
            bitrateMap[std::bitset<8>("01001111")] = "128";

            bitrateMap[std::bitset<8>("01010110")] = "40";
            bitrateMap[std::bitset<8>("01011010")] = "40";
            bitrateMap[std::bitset<8>("01011110")] = "80";
            bitrateMap[std::bitset<8>("01010111")] = "64";
            bitrateMap[std::bitset<8>("01011011")] = "80";
            bitrateMap[std::bitset<8>("01011111")] = "160";

            bitrateMap[std::bitset<8>("01100110")] = "48";
            bitrateMap[std::bitset<8>("01101010")] = "48";
            bitrateMap[std::bitset<8>("01101110")] = "96";
            bitrateMap[std::bitset<8>("01100111")] = "80";
            bitrateMap[std::bitset<8>("01101011")] = "96";
            bitrateMap[std::bitset<8>("01101111")] = "192";

            bitrateMap[std::bitset<8>("01110110")] = "56";
            bitrateMap[std::bitset<8>("01111010")] = "56";
            bitrateMap[std::bitset<8>("01111110")] = "112";
            bitrateMap[std::bitset<8>("01110111")] = "96";
            bitrateMap[std::bitset<8>("01111011")] = "112";
            bitrateMap[std::bitset<8>("01111111")] = "224";

            bitrateMap[std::bitset<8>("10000110")] = "64";
            bitrateMap[std::bitset<8>("10001010")] = "64";
            bitrateMap[std::bitset<8>("10001110")] = "128";
            bitrateMap[std::bitset<8>("10000111")] = "112";
            bitrateMap[std::bitset<8>("10001011")] = "128";
            bitrateMap[std::bitset<8>("10001111")] = "256";

            bitrateMap[std::bitset<8>("10010110")] = "80";
            bitrateMap[std::bitset<8>("10011010")] = "80";
            bitrateMap[std::bitset<8>("10011110")] = "144";
            bitrateMap[std::bitset<8>("10010111")] = "128";
            bitrateMap[std::bitset<8>("10011011")] = "160";
            bitrateMap[std::bitset<8>("10011111")] = "288";

            bitrateMap[std::bitset<8>("10100110")] = "96";
            bitrateMap[std::bitset<8>("10101010")] = "96";
            bitrateMap[std::bitset<8>("10101110")] = "160";
            bitrateMap[std::bitset<8>("10100111")] = "160";
            bitrateMap[std::bitset<8>("10101011")] = "192";
            bitrateMap[std::bitset<8>("10101111")] = "320";

            bitrateMap[std::bitset<8>("10110110")] = "112";
            bitrateMap[std::bitset<8>("10111010")] = "112";
            bitrateMap[std::bitset<8>("10111110")] = "176";
            bitrateMap[std::bitset<8>("10110111")] = "192";
            bitrateMap[std::bitset<8>("10111011")] = "224";
            bitrateMap[std::bitset<8>("10111111")] = "352";

            bitrateMap[std::bitset<8>("11000110")] = "128";
            bitrateMap[std::bitset<8>("11001010")] = "128";
            bitrateMap[std::bitset<8>("11001110")] = "192";
            bitrateMap[std::bitset<8>("11000111")] = "224";
            bitrateMap[std::bitset<8>("11001011")] = "256";
            bitrateMap[std::bitset<8>("11001111")] = "384";

            bitrateMap[std::bitset<8>("11010110")] = "144";
            bitrateMap[std::bitset<8>("11011010")] = "144";
            bitrateMap[std::bitset<8>("11011110")] = "224";
            bitrateMap[std::bitset<8>("11010111")] = "256";
            bitrateMap[std::bitset<8>("11011011")] = "320";
            bitrateMap[std::bitset<8>("11011111")] = "416";

            bitrateMap[std::bitset<8>("11100110")] = "160";
            bitrateMap[std::bitset<8>("11101010")] = "160";
            bitrateMap[std::bitset<8>("11101110")] = "256";
            bitrateMap[std::bitset<8>("11100111")] = "320";
            bitrateMap[std::bitset<8>("11101011")] = "384";
            bitrateMap[std::bitset<8>("11101111")] = "448";

            bitrateMap[std::bitset<8>("11110110")] = "bad";
            bitrateMap[std::bitset<8>("11111010")] = "bad";
            bitrateMap[std::bitset<8>("11111110")] = "bad";
            bitrateMap[std::bitset<8>("11110111")] = "bad";
            bitrateMap[std::bitset<8>("11111011")] = "bad";
            bitrateMap[std::bitset<8>("11111111")] = "bad";

            std::string finalBitrate = bitrateMap[bitrateIndexValue];

            mp3fileStream.seekg(4, std::ios::cur);
        }
        else
        {
            // If not a valid frame sync, move to the next byte and continue searching for a valid frame sync
            mp3fileStream.seekg(1, std::ios::cur);
        }
    }

    // Close the file
    mp3fileStream.close();

    return 0;
}

int mp3AudioDataRead()
{
    return 0;
}

int main()
{
    // Look at how a wave file is representated as data
    std::string filename = "test.mp3";
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }

    int sync = decodeMp3(filename);

    return 0;
}
