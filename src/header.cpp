#include "header.h"
#include "header_helper.h"

int calculateID3Size(char *ID3Header)
{
    unsigned int totalSize = 0;

    for (int i = 6; i < 10; i++)
    {
        unsigned char byte = static_cast<unsigned char>(ID3Header[i]);
        std::bitset<8> binary(byte);
        std::bitset<8> mask(0b01111111);
        unsigned int binaryToInt = (binary & mask).to_ulong();
        totalSize = (totalSize << 7) | binaryToInt;
    }
    std::cout << "ID3 Tag Size: " << totalSize << std::endl;
    return totalSize;
}

frameHeader breakDownHeader(std::ifstream &mp3fileStream)
{

    int mpegVersion;
    int layer;
    int protection;
    int bitrate;
    int samplingFrequency;
    int padding;
    int privateBit = 0;
    int mode;
    int copyright;
    int original;
    int emphasis;
    int errorCheck;

    if (!mp3fileStream.is_open())
    {
        std::cerr << "Error opening the MP3 file." << std::endl;
    }

    char header[4];
    mp3fileStream.read(header, 4);

    uint32_t sync = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];

    std::bitset<32> syncbits(sync);

    // The MPEG Audio version ID which is 2 bits,
    std::bitset<2> extractedMpegVersion((syncbits.to_ulong() >> 19) & 0b11);

    if (extractedMpegVersion == std::bitset<2>("01"))
    {
        mpegVersion = 0;
    }
    else if (extractedMpegVersion == std::bitset<2>("11"))
    {
        mpegVersion = 1;
    }
    else
    {
        mpegVersion = 2;
    }

    // The Layer description which is 2,
    std::bitset<2> extractedLayerDescription((syncbits.to_ulong() >> 17) & 0b11);

    if (extractedLayerDescription == std::bitset<2>("00"))
    {
        layer = 0;
    }
    else if (extractedLayerDescription == std::bitset<2>("01"))
    {
        layer = 3;
    }
    else if (extractedLayerDescription == std::bitset<2>("10"))
    {
        layer = 2;
    }
    else
    {
        layer = 1;
    }

    // The Protection bit which is 1,
    std::bitset<1> extractedProtectionBit((syncbits.to_ulong() >> 16) & 0b1);

    if (extractedProtectionBit == std::bitset<1>("0"))
    {
        protection = 0;
    }
    else
    {
        protection = 16;
    }

    // Then we need the Bitrate index which is 4 bits and is equal to a certain value in a table.
    std::bitset<4> extractedBitrateIndex((syncbits.to_ulong() >> 12) & 0b1111);

    // Then we have 2 bits for the frequency index which we should also create a table for.
    std::bitset<2> extractedFrequencyIndex((syncbits.to_ulong() >> 10) & 0b11);

    // Here is the padding bit.
    std::bitset<1> extractedPaddingBit((syncbits.to_ulong() >> 9) & 0b1);

    if (extractedPaddingBit == std::bitset<1>("0"))
    {
        padding = 0;
    }
    else
    {
        padding = 1;
    }

    // The Private bit. This one is only informative.
    std::bitset<1> extractedPrivateBit((syncbits.to_ulong() >> 8) & 0b1);

    // Then Channel Mode for 2 bits.
    std::bitset<2> extractedChannelMode((syncbits.to_ulong() >> 6) & 0b11);

    // The next 2 is the Mode extension.
    std::bitset<2> extractedModeExtension((syncbits.to_ulong() >> 4) & 0b11);

    // Then there is 1 bit for copyright.
    std::bitset<1> extractedCopyright((syncbits.to_ulong() >> 3) & 0b1);

    if (extractedCopyright == std::bitset<1>("0"))
    {
        copyright = 0;
    }
    else
    {
        copyright = 1;
    }

    // 1 bit for original or not.
    std::bitset<1> extractedOriginal((syncbits.to_ulong() >> 2) & 0b1);

    if (extractedOriginal == std::bitset<1>("0"))
    {
        original = 0;
    }
    else
    {
        original = 1;
    }

    // Finally there are 2 bits for emphasis, which is rarely used.
    std::bitset<2> extractedEmphasisBits((syncbits.to_ulong() >> 0) & 0b11);

    if (extractedEmphasisBits == std::bitset<2>("00"))
    {
        emphasis = 0;
    }
    else if (extractedEmphasisBits == std::bitset<2>("01"))
    {
        emphasis = 1;
    }
    else if (extractedEmphasisBits == std::bitset<2>("10"))
    {
        emphasis = 2;
    }
    else
    {
        emphasis = 3;
    }
    /*
        First I need to determine whether the frame will be mono or stereo, if mono it gets broken into 2 halves, stereo
        will be 4 quarters. Each of those regions will then be broken down further into 5 regions, 3 "big", count1, and zero.
        These regions contain data representing different frequency bands. The "big" value regions are compressed using
        3 huffman tables we can get from the mp3 standard. These tables give 2 frequency samples based on the input.
        The last of the three is specified by the side information of the frame.

        The following is from http://blog.bjrn.se/2008/10/lets-build-mp3-decoder.html

        "To decode a big values region using table pair 1, we proceed as follows:
        Say the chunk contains the following bits: 000101010... First we decode the bits as we usually decode Huffman coded
        strings: The three bits 000 correspond to the two output samples 1 and 1, we call them x and y.
        Here’s where it gets interesting: The largest code table defined in the standard has samples no larger than 15.
        This is enough to represent most signals satisfactory, but sometimes a larger value is required.
        The second value in the table pair is known as the linbits (for some reason), and whenever we have found an output sample
        that is the maximum value (15) we read linbits number of bits, and add them to the sample. For table pair 1, the
        linbits is 0, and the maximum sample value is never 15, so we ignore it in this case. For some samples, linbits may be as
        large as 13, so the maximum value is 15+8191. When we have read linbits for sample x, we get the sign.
        If x is not 0, we read one bit. This determines of the sample is positive or negative.
        All in all, the two samples are decoded in these steps: Decode the first bits using the Huffman table.
        Call the samples x and y. If x = 15 and linbits is not 0, get linbits bits and add to x. x is now at most 8206.
        If x is not 0, get one bit. If 1, then x is –x. Do step 2 and 3 for y. The count1 region codes the frequencies that are so
        high they have been compressed tightly, and when decoded we have samples in the range –1 to 1.
        There are only two possible tables for this region; these are known as the quad tables as each code word corresponds to
        4 output samples. There are no linbits for the count1 region, so decoding is only a matter of using the appropriate table
        and get the sign bits. Decode the first bits using the Huffman table. Call the samples v, w, x and y.
        If v is not 0, get one bit. If 1, then v is –v. Do step 2 for w, x and y."
    */

    int calculatedBitrate = calculateBitrate(mpegVersion, layer, extractedBitrateIndex);

    int calculatedSampleRate = calculateSampleRate(mpegVersion, layer);

    int crcBits = calculateErrorCheckSize(extractedProtectionBit);

    int calculatedFrameSize = calculateFrameSize(calculatedBitrate, calculatedSampleRate, mpegVersion, extractedFrequencyIndex, extractedPaddingBit, crcBits);

    int channelMode = calculateChannelMode(extractedChannelMode);

    int modeExtension = calculateModeExtension(extractedModeExtension, channelMode, extractedLayerDescription);

    frameHeader currentFrame = {
        mpegVersion,
        layer,
        protection,
        calculatedBitrate,
        calculatedSampleRate,
        padding,
        privateBit,
        channelMode,
        modeExtension,
        copyright,
        original,
        emphasis,
        calculatedFrameSize};

    return currentFrame;
}