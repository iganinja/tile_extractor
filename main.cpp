#include <iostream>
#include <string>
#include <algorithm>
#include "lodepng.h"

constexpr unsigned PixelSize = 4; // In bytes

using ImageData = std::vector<unsigned char>; // We assume it is in RGBA format
using ImageIt = ImageData::const_iterator;


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
            const auto tileFileName = "tile" + std::to_string(tileId) + ".png";
            const auto tileBeginIt = image.cbegin() + ((tileY * tileHeight) * imageWidth + (tileX * tileWidth)) * PixelSize;

            saveTile(tileFileName, tileBeginIt, tileWidth, tileHeight, imageWidth);
        }
    }

    return 0;
}
