#include <iostream>
#include "yswavfile.h"

int toSigned16Bit(unsigned char leastByte, unsigned char mostByte)
{
    int val = leastByte + mostByte*256;
    if(val >= 32768)
        val -= 65536;

    return val;
}

void printWavDataPositions(YsWavFile *wavFile, int posCount)
{
    const unsigned char *ptr = wavFile->DataPointer();

    for (int i = 0; i < posCount; i++)
        printf(
            "pos %3d: unsigned - %4u %4u / signed - %4d %4d / int - %d\n",
            i, ptr[i*2+1], ptr[i*2], (char)ptr[i*2+1], (char)ptr[i*2],
            toSigned16Bit(ptr[i*2], ptr[i*2+1])
        );
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Audio file is required" << std::endl;
        return 1;
    }

    YsWavFile wavFile;
    YSRESULT loadResult = wavFile.LoadWav(argv[1]);
    if(loadResult != YSOK)
    {
        std::cout << "Error occured during wav file parsing" << std::endl;
        return 1;
    }

    std::cout << "WAV file has been parsed" << std::endl;

    printWavDataPositions(&wavFile, 500);
    return 0;
}
