/*

MIT License

Copyright Iñaki Griego (c) 2017 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_set>
#include "lodepng.h"

using Pixel = std::uint32_t; // RGBA
constexpr unsigned PixelSize = sizeof(Pixel);
using ImageData = std::vector<unsigned char>; // We assume it is in RGBA format
using ImageIt = ImageData::const_iterator;


std::unordered_set<Pixel> extractedPlainTilesColors; // A plain tile is a monochrome one. We only are going to extract one of each color

Pixel getPixel(ImageIt it)
{
    const std::uint32_t r = *(it + 0);
    const std::uint32_t g = *(it + 1);
    const std::uint32_t b = *(it + 2);
    const std::uint32_t a = *(it + 3);
    return r << 24 | g << 16 | b << 8 | a;
}

bool isPlainTile(ImageIt tileBeginIt, unsigned tileWidth, unsigned tileHeight, unsigned imageWidth)
{
    const auto firstPixel = getPixel(tileBeginIt);

    for(unsigned y = 0; y < tileHeight; ++ y)
    {
        for(unsigned x = 0; x < tileWidth; ++ x)
        {
            const auto it = tileBeginIt + (imageWidth * y + x) * PixelSize;
            if(firstPixel != getPixel(it))
            {
                return false;
            }
        }
    }

    return true;
}

void saveTile(const std::string& fileName, ImageIt tileBeginIt, unsigned tileWidth, unsigned tileHeight, unsigned imageWidth)
{
    ImageData image;
    image.resize(tileWidth * tileHeight * PixelSize);
    auto destinyIt = image.begin();

    for(unsigned y = 0; y < tileHeight; ++ y)
    {
        const auto lineBeginIt = tileBeginIt + (imageWidth * y) * PixelSize;
        const auto lineEndIt = lineBeginIt + tileWidth * PixelSize;
        std::copy(lineBeginIt, lineEndIt, destinyIt);
        destinyIt += tileWidth * PixelSize;
    }

    const auto error = lodepng::encode(fileName, image, tileWidth, tileHeight);

    if(error)
    {
        std::cout << "Cannot save " << fileName << " file (" << error << "): " << lodepng_error_text(error) << std::endl;
    }
}


int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        std::cout << "tile_extractor 1.0\n";
        std::cout << "Usage: tile_extractor file_with_tiles.png tile_width tile_height\n";
        std::cout << "Example: tile_extractor tileset.png 32 32\n";

        return 0;
    }

    const std::string fileName{argv[1]};
    const unsigned tileWidth = std::atoi(argv[2]);
    const unsigned tileHeight = std::atoi(argv[3]);

    ImageData image;
    unsigned imageWidth, imageHeight;

    const auto error = lodepng::decode(image, imageWidth, imageHeight, fileName);

    if(error)
    {
        std::cout << "Cannot open " << fileName << " file (" << error << "): " << lodepng_error_text(error) << std::endl;
    }

    const auto horizontalTiles = imageWidth / tileWidth;
    const auto verticalTiles = imageHeight / tileHeight;

    for(unsigned tileY = 0; tileY < verticalTiles; ++ tileY)
    {
        for(unsigned tileX = 0; tileX < horizontalTiles; ++ tileX)
        {
            const int tileId = horizontalTiles * tileY + tileX;
            const auto tileBeginIt = image.cbegin() + ((tileY * tileHeight) * imageWidth + (tileX * tileWidth)) * PixelSize;

            std::cout << "Processing tile " << tileId << "... ";

            if(isPlainTile(tileBeginIt, tileWidth, tileHeight, imageWidth))
            {
                const auto firstPixel = getPixel(tileBeginIt);
                if(extractedPlainTilesColors.find(firstPixel) != extractedPlainTilesColors.end())
                {
                    std::cout << "Already extracted plain tile, skipped\n";
                    continue;
                }
                extractedPlainTilesColors.insert(firstPixel);
            }

            const auto tileFileName = "tile" + std::to_string(tileId) + ".png";

            saveTile(tileFileName, tileBeginIt, tileWidth, tileHeight, imageWidth);

            std::cout << tileFileName << " saved\n";
        }
    }

    return 0;
}
