// C
#include <cstdint>
#include <cstring>
// C++
#include <array>
#include <filesystem>
#include <fstream>
#include <utility>
// OS
#include <assert.h>
// STB
#define STBI_WINDOWS_UTF8
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
//#define STBI_NO_PNG
#define STBI_NO_BMP
#define STBI_NO_GIF
#define STBI_NO_PSD
#define STBI_NO_PIC
//#define STBI_NO_JPEG
#define STBI_NO_PNM
#define STBI_NO_HDR
#define STBI_NO_TGA
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define STBIW_WINDOWS_UTF8
#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
// This
#define JPEG_COMPRESSION_LEVEL      90  // 1-100, 90 = default
#include "stb_image_cpp.h"

/**
 * @brief Compile time generation of the CRC32 table as an std::array.
 * @return An array forming the CRC32 table.
 */
constexpr std::array<uint32_t, 256>
generate_crc32_table()
{
    std::array<uint32_t, 256> array{ 0 };

    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t c = i;

        for (size_t j = 0; j < 8; j++)
        {
            if (c & 1)
            {
                c = polynomial ^ (c >> 1);
            }
            else
            {
                c >>= 1;
            }
        }

        array[i] = c;
    }

    return array;
}

/**
 * @brief Generates the CRC32 for the given @param pData for the given @param size number of bytes.
 * @param pData A pointer to the data to generate the CRC32 for.
 * @param size The number of bytes the data is comprised of.
 * @param initial The initial CRC32 value. (Default = 0)
 * @return The CRC32 for the data of size.
 */
static int32_t
generate_crc32(const void* pData, size_t size, int32_t initial)
{
    const std::array<uint32_t, 256> CRC_TABLE = generate_crc32_table();

    uint32_t c = initial ^ 0xFFFFFFFF;
    const uint8_t* u = static_cast<const uint8_t*>(pData);

    for (size_t i = 0; i < size; ++i)
    {
        c = CRC_TABLE[(c ^ u[i]) & 0xFF] ^ (c >> 8);
    }

    return c ^ 0xFFFFFFFF;
}

/**
 * @brief Writes the output bytes to a memory buffer.
 * @param pContext The writing context. (e.g. std::vector<char>*)
 * @param pData A pointer to the data to write.
 * @param dataSize The number of bytes to write.
 * @remarks Multiple calls may be made to generate a complete file.
 */
static void
writeToMemory(void* pContext, void* pData, int dataSize)
{
    std::vector<char>* pBuffer = reinterpret_cast<std::vector<char>*>(pContext);
    size_t written = pBuffer->size();
    pBuffer->resize(written + dataSize);
    std::memcpy(pBuffer->data() + written, pData, dataSize);
}

stb::Image::Image() :
    m_pPixelData(nullptr),
    m_crc(0u),
    m_width(0),
    m_height(0),
    m_compression(0),
    m_type(Type::Invalid)
{

}

stb::Image::Image(const char* pFilePath) :
    m_pPixelData(nullptr),
    m_crc(0u),
    m_width(0),
    m_height(0),
    m_compression(0),
    m_type(Type::Invalid)
{
    load(pFilePath);
}

stb::Image::Image(const char* pData, size_t dataSize) :
    m_pPixelData(nullptr),
    m_crc(0u),
    m_width(0),
    m_height(0),
    m_compression(0),
    m_type(Type::Invalid)
{
    load(pData, dataSize);
}

stb::Image::~Image()
{
    stbi_image_free(m_pPixelData);
}

stb::Image::Image(Image&& other) noexcept :
    m_fileData(std::move(other.m_fileData)),
    m_pPixelData(std::exchange(other.m_pPixelData, nullptr)),
    m_crc(std::exchange(other.m_crc, 0)),
    m_width(std::exchange(other.m_width, 0)),
    m_height(std::exchange(other.m_height, 0)),
    m_compression(std::exchange(other.m_compression, 0)),
    m_type(std::exchange(other.m_type, Type::Invalid))
{

}

stb::Image&
stb::Image::operator=(Image&& other) noexcept
{
    m_fileData = std::move(other.m_fileData);

    std::swap(m_pPixelData, other.m_pPixelData);
    std::swap(m_crc, other.m_crc);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
    std::swap(m_compression, other.m_compression);
    std::swap(m_type, other.m_type);

    return *this;
}

bool
stb::Image::load(const char* pFilePath)
{
    std::filesystem::path filePath(pFilePath);

    size_t fileSize = std::filesystem::file_size(filePath);
    m_fileData.resize(fileSize);

    std::fstream stream(filePath, (std::ios_base::in | std::ios_base::binary));
    if (!stream.read(m_fileData.data(), m_fileData.size()))
    {
        return false;
    }

    return load();
}

bool
stb::Image::load(const char* pData, size_t dataSize)
{
    m_fileData.resize(dataSize);
    std::memcpy(m_fileData.data(), pData, m_fileData.size());

    return load();
}

bool
stb::Image::setType()
{
    stbi__context s;
    stbi__start_mem(&s, reinterpret_cast<const stbi_uc*>(m_fileData.data()), static_cast<int>(m_fileData.size()));

    if (stbi__png_test(&s))
    {
        m_type = Type::PNG;
        return true;
    }

    if (stbi__jpeg_test(&s))
    {
        m_type = Type::JPEG;
        return true;
    }

    return false;
}

bool
stb::Image::load()
{
    if (!setType())
    {
        return false;
    }

    m_pPixelData = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(m_fileData.data()),
        static_cast<int>(m_fileData.size()),
        &m_width,
        &m_height,
        &m_compression,
        0
    );

    if (m_pPixelData == nullptr)
    {
        m_width = 0;
        m_height = 0;
        m_compression = 0;
        return false;
    }

    m_crc = generate_crc32(m_fileData.data(), m_fileData.size(), 0);
    return true;
}

bool
stb::Image::save(const char* pFilePath)
{
    std::filesystem::path filePath(pFilePath);
    std::fstream stream(filePath, (std::ios_base::out | std::ios_base::binary | std::ios_base::trunc));

    if (!stream.write(m_fileData.data(), m_fileData.size()))
    {
        return false;
    }

    return true;
}

bool
stb::Image::resize(int32_t newWidth, int32_t newHeight)
{
    std::vector<unsigned char> pixelData(newWidth * newWidth * 8);

    int result;

    result = stbir_resize_uint8(
        m_pPixelData,
            m_width,
            m_height,
            0,
        pixelData.data(),
            newWidth,
            newHeight,
            0,
        m_compression
    );

    if (!result)
    {
        return false;
    }

    std::vector<char> imageData;
    imageData.reserve(newWidth * newHeight * 3);

    if (m_type == Type::JPEG)
    {
        result = stbi_write_jpg_to_func(&writeToMemory, &imageData,
            newWidth,
            newHeight,
            m_compression,
            pixelData.data(),
            JPEG_COMPRESSION_LEVEL
        );
    }
    else
    {
        assert(m_type == Type::PNG);

        result = stbi_write_png_to_func(&writeToMemory, &imageData,
            newWidth,
            newHeight,
            m_compression,
            pixelData.data(),
            0
        );
    }

    if (!result)
    {
        return false;
    }

    return load(imageData.data(), imageData.size());
}
