#ifndef DL_SDLTEMPLATE_HELPERS_H
#define DL_SDLTEMPLATE_HELPERS_H
#include "SDL3/SDL.h"
#include "stb_image.h"
inline SDL_Surface* LoadImage(const char* imgName, int desiredChannels)
{
    int width {0}, height {0}, channels {0};
    stbi_uc* textureAtlasBmp {stbi_load(imgName, &width, &height, &channels, 4)};
    if (!textureAtlasBmp)
    {
        printf("Load failed: %s\n", stbi_failure_reason());
        perror("Error");
    }
    return SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, textureAtlasBmp, width * 4);
}


#endif