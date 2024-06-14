#include <iostream>
#include <vector>
#include "libjpeg-turbo/turbojpeg.h"
#include <fstream>
#include <string>
// 用于保存图像的辅助函数
bool saveJPEG(const char *filename, unsigned char *buffer, int width, int height, int jpegSubsamp, int quality = 85)
{
    tjhandle tjInstance = tjInitCompress();
    if (!tjInstance)
    {
        std::cerr << "Error initializing TurboJPEG compressor." << std::endl;
        return false;
    }

    unsigned char *compressedBuffer = nullptr;
    unsigned long compressedSize = 0;

    int tjStatus = tjCompress2(tjInstance, buffer, width, 0, height, TJPF_RGB, &compressedBuffer, &compressedSize, jpegSubsamp, quality, TJFLAG_FASTDCT);
    if (tjStatus != 0)
    {
        std::cerr << "Error compressing JPEG: " << tjGetErrorStr2(tjInstance) << std::endl;
        tjDestroy(tjInstance);
        return false;
    }

    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Error opening output file." << std::endl;
        tjDestroy(tjInstance);
        return false;
    }

    outFile.write(reinterpret_cast<char *>(compressedBuffer), compressedSize);
    outFile.close();

    tjFree(compressedBuffer);
    tjDestroy(tjInstance);

    return true;
}

int main()
{
    const char *filename = "unity_texture2d_big.jpg";

    // 打开文件并读取
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 1;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        std::cerr << "Failed to read file: " << filename << std::endl;
        return 1;
    }

    file.close();

    tjhandle tjInstance = tjInitDecompress();
    if (!tjInstance)
    {
        std::cerr << "Error initializing TurboJPEG decompressor." << std::endl;
        return 1;
    }

    int width, height, jpegSubsamp, jpegColorspace;
    if (tjDecompressHeader3(tjInstance, reinterpret_cast<unsigned char *>(buffer.data()), size, &width, &height, &jpegSubsamp, &jpegColorspace) != 0)
    {
        std::cerr << "Error reading JPEG header: " << tjGetErrorStr2(tjInstance) << std::endl;
        tjDestroy(tjInstance);
        return 1;
    }

    std::vector<unsigned char> imgBuffer(width * height * 3); // Assuming RGB format

    if (tjDecompress2(tjInstance, reinterpret_cast<unsigned char *>(buffer.data()), size, imgBuffer.data(), width, 0, height, TJPF_RGB, TJFLAG_FASTDCT) != 0)
    {
        std::cerr << "Error decompressing JPEG: " << tjGetErrorStr2(tjInstance) << std::endl;
        tjDestroy(tjInstance);
        return 1;
    }

    tjDestroy(tjInstance);

    int rows = 5;
    int cols = 4;
    int tileWidth = width / cols;
    int tileHeight = height / rows;

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            std::vector<unsigned char> tileBuffer(tileWidth * tileHeight * 3); // Assuming RGB format

            for (int y = 0; y < tileHeight; ++y)
            {
                for (int x = 0; x < tileWidth; ++x)
                {
                    int srcX = col * tileWidth + x;
                    int srcY = row * tileHeight + y;
                    int srcIndex = (srcY * width + srcX) * 3;
                    int dstIndex = (y * tileWidth + x) * 3;
                    std::copy(imgBuffer.begin() + srcIndex, imgBuffer.begin() + srcIndex + 3, tileBuffer.begin() + dstIndex);
                }
            }

            std::string tileFilename = "sub" + std::to_string(row + rows * 4) + ".jpg";
            if (!saveJPEG(tileFilename.c_str(), tileBuffer.data(), tileWidth, tileHeight, jpegSubsamp))
            {
                std::cerr << "Error saving tile: " << tileFilename << std::endl;
                return 1;
            }
        }
    }

    std::cout << "Tiles generated successfully." << std::endl;
    return 0;
}
