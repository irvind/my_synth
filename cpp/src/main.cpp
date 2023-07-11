#include <iostream>
#include "yswavfile.h"

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

    return 0;
}
