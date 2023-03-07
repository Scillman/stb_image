#ifndef STB_IMAGE_CPP_H
#define STB_IMAGE_CPP_H

#include <cstdint>
#include <vector>

namespace stb
{

/**
 * @brief STB Image implementation as a C++ class.
 *
 * Uses:
 *   v2.27  stb_image
 *   v0.97  stb_image_resize
 *   v1.16  stb_image_write
 *
 * @see https://github.com/nothings/stb
 */
class Image
{
public:
    enum Type : int32_t
    {
        Invalid = 0,
        JPEG,
        PNG
    };

public:
    Image();
    explicit Image(const char* pFilePath);
    explicit Image(const char* pData, size_t dataSize);
    ~Image();

private:
    Image(const Image& other) = delete;
    Image& operator=(const Image& other) = delete;

public:
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

public:
    inline bool operator==(const Image& other) const noexcept
    {
        return m_crc == other.m_crc;
    }

    inline bool operator!=(const Image& other) const noexcept
    {
        return m_crc != other.m_crc;
    }

public:
    bool load(const char* pFilePath);
    bool load(const char* pData, size_t dataSize);

    bool save(const char* pFilePath);

    bool resize(int32_t newWidth, int32_t newHeight);

private:
    bool setType();
    bool load();

public:
    inline const char* pixels() const noexcept
    {
        return reinterpret_cast<char*>(m_pPixelData);
    }

    inline const char* data() const noexcept
    {
        return m_fileData.data();
    }

    inline size_t size() const noexcept
    {
        return m_fileData.size();
    }

    inline int32_t width() const noexcept
    {
        return m_width;
    }

    inline int32_t height() const noexcept
    {
        return m_height;
    }

    inline Type type() const noexcept
    {
        return m_type;
    }

private:
    std::vector<char>   m_fileData;
    unsigned char*      m_pPixelData;

    uint32_t    m_crc;
    int32_t     m_width;
    int32_t     m_height;
    int32_t     m_compression;
    Type        m_type;

}; // class Image

}; // namespace stb

#endif // STB_IMAGE_CPP_H
