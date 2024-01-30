#include "header_helper.h"

int calculateBitrate(int mpegVersion, int layer, std::bitset<4> bitrateIndex)
{
    int bitrateTable[5][16] = {
        // V1, L1
        {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1},
        // V1, L2
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1},
        // V1, L3
        {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, -1},
        // V2, L1, L2, L3
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, -1},
        {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1}};

    int index;

    if (mpegVersion == 1 && layer == 1)
    {
        index = 0;
    }
    else if (mpegVersion == 1 && layer == 2)
    {
        index = 1;
    }
    else if (mpegVersion == 1 && layer == 3)
    {
        index = 2;
    }
    else if (mpegVersion == 2 && layer == 1)
    {
        index = 3;
    }
    else
    {
        index = 4;
    }

    int bitrate = bitrateTable[index][bitrateIndex.to_ulong()];
    return bitrate;
}

/*


    "SamplesPerFrame depends on both the MPEG version and Layer.  For MPEG-1.0 Layer-III and Layer-II the samples per frame is 1152 bytes.
    For MPEG-1.0 Layer-I it's 384 bytes.  For MPEG-2.0 and MPEG-2.5, Layer-I is 384 bytes, Layer-II is 1152 bytes, and Layer-III is 576 bytes.
    PaddingSize is determined by the layer number.  Layer-I is 4 bytes and Layer-II and Layer-III are 1 byte.  It's the same across MPEG-1.0, 2.0, and 2.5.
    IsPadding is used on a per-frame basis, you have to check it for each frame."


    -Jud, https://hydrogenaud.io/index.php/topic,32036.0.html
*/
int calculateSampleRate(int mpegVersion, int layer)
{
    // Samples per frame in bytes.
    int samplesPerFrame;
    // Version 1
    // Layer 1
    if (mpegVersion == 1 && layer == 1)
    {
        samplesPerFrame = 384;
    }
    // Layer 2
    else if (mpegVersion == 1 && layer == 2)
    {
        samplesPerFrame = 1152;
    }
    // Layer 3
    else if (mpegVersion == 1 && layer == 3)
    {
        samplesPerFrame = 1152;
    }

    // Version 2 and 2.5
    // Layer 1
    else if (mpegVersion == 2 && layer == 1)
    {
        samplesPerFrame = 384;
    }
    // Layer 2
    else if (mpegVersion == 2 && layer == 2)
    {
        samplesPerFrame = 1152;
    }
    // Layer 3
    else if (mpegVersion == 2 && layer == 3)
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

int calculateFrameSize(int bitrate, int samplesPerFrame, int mpegVersion, std::bitset<2> frequency, std::bitset<1> paddingBit, int crcBits)
{
    int sampleRate;
    // 44100Hz
    // Version 1
    if (frequency == std::bitset<2>("00") && mpegVersion == 1)
    {
        sampleRate = 44100;
    }
    // 22050Hz
    // Version 2
    else if (frequency == std::bitset<2>("00") && mpegVersion == 2)
    {
        sampleRate = 22050;
    }
    // 11025Hz
    // Version 2.5
    else if (frequency == std::bitset<2>("00") && mpegVersion == 2)
    {
        sampleRate = 11025;
    }
    // 48000Hz
    // Version 1
    else if (frequency == std::bitset<2>("01") && mpegVersion == 1)
    {
        sampleRate = 48000;
    }
    // 24000Hz
    // Version 2
    else if (frequency == std::bitset<2>("01") && mpegVersion == 2)
    {
        sampleRate = 24000;
    }
    // 12000Hz
    // Version 2.5
    else if (frequency == std::bitset<2>("01") && mpegVersion == 2)
    {
        sampleRate = 12000;
    }
    // 32000Hz
    // Version 1
    else if (frequency == std::bitset<2>("10") && mpegVersion == 1)
    {
        sampleRate = 32000;
    }
    // 16000Hz
    // Version 2
    else if (frequency == std::bitset<2>("10") && mpegVersion == 2)
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

    /*
    TODO: this was a mistake I made with calculating the framesize, we shouldnt use the 144 magic number.
    The "144" is not a magic number, it's Bits_Per_Sample, which = (Samples_Per_Frame / 8).
    The number of samples in a frame is 1152, divided by 8 gives 144. However, the sample header describes MPEG 2 Layer 3,
    which uses 576 samples per frame.  This yields 72 for Bits_Per_Sample.  So, 260 bytes is the size of two frames, not one.
    */
    int samples = samplesPerFrame / 8;
    int frameSize = samples * bitrate / ((sampleRate / 1000) + padding);
    return frameSize + crcBits;
}

int calculateErrorCheckSize(std::bitset<1> protectionBit)
{
    if (protectionBit == std::bitset<1>("1"))
    {
        return 16;
    }
    else
    {
        return 0;
    }
}

int calculateChannelMode(std::bitset<2> channelBit)
{
    if (channelBit == std::bitset<2>("00"))
    {
        // Stereo
        return 0;
    }
    else if (channelBit == std::bitset<2>("01"))
    {
        // joint_stereo
        return 1;
    }
    else if (channelBit == std::bitset<2>("10"))
    {
        // dual_channel
        return 2;
    }
    else
    {
        return 3;
    }
}

int calculateModeExtension(std::bitset<2> modeExtBit, int channelMode, std::bitset<2> layer)
{
    if (layer != std::bitset<2>("01") && channelMode == 1)
    {
        if (modeExtBit == std::bitset<2>("00"))
        {
            // subbands 4-31 in intensity_stereo, bound == 4
            return 0;
        }
        else if (modeExtBit == std::bitset<2>("01"))
        {
            // subbands 8 - 31 bound == 8
            return 1;
        }
        else if (modeExtBit == std::bitset<2>("10"))
        {
            // subbands 12-31 bound == 12
            return 2;
        }
        else
        {
            // subbands 16-31 bound 16
            return 3;
        }
    }
    // Layer 3
    else
    {
        if (modeExtBit == std::bitset<2>("00"))
        {
            // intensity stereo off and ms stereo off
            return 0;
        }
        else if (modeExtBit == std::bitset<2>("01"))
        {
            // Intense on ms off
            return 1;
        }
        else if (modeExtBit == std::bitset<2>("10"))
        {
            // intense off ms on
            return 2;
        }
        else
        {
            // both on
            return 3;
        }
    }
}

