#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <bitset>
#include <string>
#include <map>

// This will calculate the bitrate instead of using a table system. Credit to FlorisCreyf, I took the general structure from his
// Mp3-decoder repo: https://github.com/FlorisCreyf/mp3-decoder/blob/master/mp3.cpp
int calculateBitrate(std::bitset<2> mpegVersion, std::bitset<2> layer, std::bitset<4> bitrateIndex)
{
    int bitrate;
    if (mpegVersion == std::bitset<2>("11") && layer == std::bitset<2>("11"))
    {
        bitrate = bitrateIndex.to_ulong() * 32;
        std::cout << bitrate << std::endl;
    }
    else if (mpegVersion == std::bitset<2>("11") && layer == std::bitset<2>("10"))
    {
        bitrate = bitrateIndex.to_ulong() * 0.5 + 1;
    }
    else if (mpegVersion == std::bitset<2>("11") && layer == std::bitset<2>("01"))
    {
        bitrate = bitrateIndex.to_ulong() * 0.25 + 1;
    }
    else if (mpegVersion == std::bitset<2>("10") && layer == std::bitset<2>("11"))
    {
        bitrate = bitrateIndex.to_ulong() * 16 + 32;
    }
    else if (mpegVersion == std::bitset<2>("10") && layer == std::bitset<2>("01"))
    {
        bitrate = bitrateIndex.to_ulong() * 8 + 8;
    }
    else
    {
        bitrate = bitrateIndex.to_ulong() * 8 + 8;
    }
    return bitrate;
}

/*


    "SamplesPerFrame depends on both the MPEG version and Layer.  For MPEG-1.0 Layer-III and Layer-II the samples per frame is 1152 bytes.
    For MPEG-1.0 Layer-I it's 384 bytes.  For MPEG-2.0 and MPEG-2.5, Layer-I is 384 bytes, Layer-II is 1152 bytes, and Layer-III is 576 bytes.
    PaddingSize is determined by the layer number.  Layer-I is 4 bytes and Layer-II and Layer-III are 1 byte.  It's the same across MPEG-1.0, 2.0, and 2.5.
    IsPadding is used on a per-frame basis, you have to check it for each frame."


    -Jud, https://hydrogenaud.io/index.php/topic,32036.0.html
*/

int calculateSampleRate(std::bitset<2> mpegVersion, std::bitset<2> layer)
{
    // Samples per frame in bytes.
    int samplesPerFrame;
    // Version 1
    // Layer 1
    if (mpegVersion == std::bitset<2>("11") && layer == std::bitset<2>("11"))
    {
        samplesPerFrame = 384;
    }
    // Layer 2
    else if (mpegVersion == std::bitset<2>("11") && layer == std::bitset<2>("10"))
    {
        samplesPerFrame = 1152;
    }
    // Layer 3
    else if (mpegVersion == std::bitset<2>("11") && layer == std::bitset<2>("01"))
    {
        samplesPerFrame = 1152;
    }

    // Version 2 and 2.5
    // Layer 1
    else if (mpegVersion == std::bitset<2>("10") && layer == std::bitset<2>("11"))
    {
        samplesPerFrame = 384;
    }
    // Layer 2
    else if (mpegVersion == std::bitset<2>("10") && layer == std::bitset<2>("10"))
    {
        samplesPerFrame = 1152;
    }
    // Layer 3
    else if (mpegVersion == std::bitset<2>("10") && layer == std::bitset<2>("01"))
    {
        samplesPerFrame = 576;
    }

    // Reserved
    else
    {
        samplesPerFrame = 0;
    }
    return samplesPerFrame;
}

// FrameSize = Bitrate * 1000/8 * SamplesPerFrame / Frequency + IsPadding * PaddingSize

int calculateFrameSize(int bitrate, int samplesPerFrame, std::bitset<2> mpegVersion, std::bitset<2> frequency, std::bitset<1> paddingBit)
{
    int sampleRate;
    // 44100Hz
    // Version 1
    if (frequency == std::bitset<2>("00") && mpegVersion == std::bitset<2>("11"))
    {
        sampleRate = 44100;
    }
    // 22050Hz
    // Version 2
    else if (frequency == std::bitset<2>("00") && mpegVersion == std::bitset<2>("10"))
    {
        sampleRate = 22050;
    }
    // 11025Hz
    // Version 2.5
    else if (frequency == std::bitset<2>("00") && mpegVersion == std::bitset<2>("00"))
    {
        sampleRate = 11025;
    }
    // 48000Hz
    // Version 1
    else if (frequency == std::bitset<2>("01") && mpegVersion == std::bitset<2>("11"))
    {
        sampleRate = 48000;
    }
    // 24000Hz
    // Version 2
    else if (frequency == std::bitset<2>("01") && mpegVersion == std::bitset<2>("10"))
    {
        sampleRate = 24000;
    }
    // 12000Hz
    // Version 2.5
    else if (frequency == std::bitset<2>("01") && mpegVersion == std::bitset<2>("00"))
    {
        sampleRate = 12000;
    }
    // 32000Hz
    // Version 1
    else if (frequency == std::bitset<2>("10") && mpegVersion == std::bitset<2>("11"))
    {
        sampleRate = 32000;
    }
    // 16000Hz
    // Version 2
    else if (frequency == std::bitset<2>("10") && mpegVersion == std::bitset<2>("10"))
    {
        sampleRate = 16000;
    }
    // 8000Hz
    // Version 2.5
    else
    {
        sampleRate = 8000;
    }

    int padding;

    if (paddingBit == std::bitset<1>("0"))
    {
        padding = 0;
    }
    else
    {
        padding = 1;
    }

    int frameSize = 144 * bitrate / ((sampleRate / 1000) + padding);
    return frameSize;
}

// Next step is to calculate the size of the frame data halves and then find the next frame header.

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

        if (!mp3fileStream)
        {
            if (mp3fileStream.eof())
            {
                break;
            }
            else
            {
                std::cerr << "Error Reading Frame Header." << std::endl;
                break;
            }
        }

        uint32_t sync = (frameHeader[0] << 24) | (frameHeader[1] << 16) | (frameHeader[2] << 8) | frameHeader[3];

        if ((sync & 0xFFE00000) == 0xFFE00000)
        {
            // Going to use the bitrate index total for a switch statement involving the bitrate index.
            std::bitset<32> syncbits(sync);

            // Extract the information from the frame header.
            // http://www.mp3-tech.org/programmer/frame_header.html
            // we already found the frame header sync bits so then we need to find:
            // The MPEG Audio version ID which is 2 bits,
            std::bitset<2> extractedMpegVersion((syncbits.to_ulong() >> 19) & 0b11);

            // The Layer description which is 2,
            std::bitset<2> extractedLayerDescription((syncbits.to_ulong() >> 17) & 0b11);

            // The Protection bit which is 1,
            std::bitset<1> extractedProtectionBit((syncbits.to_ulong() >> 16) & 0b1);

            // Then we need the Bitrate index which is 4 bits and is equal to a certain value in a table.
            std::bitset<4> extractedBitrateIndex((syncbits.to_ulong() >> 12) & 0b1111);

            int calculatedBitrate = calculateBitrate(extractedMpegVersion, extractedLayerDescription, extractedBitrateIndex);

            // Then we have 2 bits for the frequency index which we should also create a table for.
            std::bitset<2> extractedFrequencyIndex((syncbits.to_ulong() >> 10) & 0b11);

            // Here is the padding bit.
            std::bitset<1> extractedPaddingBit((syncbits.to_ulong() >> 9) & 0b1);

            int calculatedSampleRate = calculateSampleRate(extractedMpegVersion, extractedLayerDescription);

            int calculatedFrameSize = calculateFrameSize(calculatedBitrate, calculatedSampleRate, extractedMpegVersion, extractedFrequencyIndex, extractedPaddingBit);

            // The Private bit. This one is only informative.
            std::bitset<1> extractedPrivateBit((syncbits.to_ulong() >> 8) & 0b1);

            // Then Channel Mode for 2 bits.
            std::bitset<2> extractedChannelMode((syncbits.to_ulong() >> 6) & 0b11);

            // The next 2 is the Mode extension.
            std::bitset<2> extractedModeExtension((syncbits.to_ulong() >> 4) & 0b11);

            // Then there is 1 bit for copyright.
            std::bitset<1> extractedCopyright((syncbits.to_ulong() >> 3) & 0b1);

            // 1 bit for original or not.
            std::bitset<1> extractedOriginal((syncbits.to_ulong() >> 2) & 0b1);

            // Finally there are 2 bits for emphasis, which is rarely used.
            std::bitset<2> extractedEmphasisBits((syncbits.to_ulong() >> 0) & 0b11);

            mp3fileStream.seekg(calculatedFrameSize - 4, std::ios::cur);
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

int main()
{
    // Look at how a wave file is representated as data
    std::string filename = "file_example_mp3_700KB.mp3";

    int sync = decodeMp3(filename);

    return 0;
}
