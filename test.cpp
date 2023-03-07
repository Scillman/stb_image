#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <assert.h>
#include "stb_image_cpp.h"

static std::vector<char>
readFile(const char* pFilePath)
{
    std::filesystem::path filePath(pFilePath);
    size_t fileSize = std::filesystem::file_size(filePath);

    std::vector<char> buffer(fileSize);

    std::fstream stream(filePath, (std::ios_base::in | std::ios_base::binary));
    bool read = (stream.read(buffer.data(), buffer.size()) ? true : false);
    assert(read == true);

    return buffer;
}

static bool
writeFile(const char* pFilePath, const std::vector<char>& buffer)
{
    std::fstream stream(pFilePath, (std::ios_base::out | std::ios_base::binary | std::ios_base::trunc));
    bool written = (stream.write(buffer.data(), buffer.size()) ? true : false);
    assert(written == true);

    return written;
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "USAGE: " << argv[0] << " <file path in> <file path out>" << std::endl;
        return 0;
    }

    const char* pSourceFile = argv[1];
    const char* pDestFile = argv[2];

    std::vector<char> buffer = readFile(pSourceFile);
    stb::Image image;

    if (image.load(buffer.data(), buffer.size()))
    {
        std::cout << "\tLOADED" << std::endl;

        switch (image.type())
        {
            case stb::Image::Type::JPEG:
                std::cout << "\tTYPE: JPEG" << std::endl;
                break;

            case stb::Image::Type::PNG:
                std::cout << "\tTYPE: PNG" << std::endl;
                break;

            default:
                assert(image.type() == stb::Image::Type::Invalid);
                std::cout << "\tTYPE: INVALID" << std::endl;
                break;
        }

        if (image.resize(image.width() >> 1, image.height() >> 1))
        {
            std::cout << "\tRESIZED" << std::endl;

            if (image.save(pDestFile))
            {
                std::cout << "\tSAVED" << std::endl;
            }
            else
            {
                std::cout << "\tSAVE FAILED" << std::endl;
            }
        }
        else
        {
            std::cout << "\tRESIZE FAILED" << std::endl;
        }
    }
    else
    {
        std::cout << "\tLOAD FAILED" << std::endl;
    }

    std::cout << "COMPLETED" << std::endl;
    return 0;
}
