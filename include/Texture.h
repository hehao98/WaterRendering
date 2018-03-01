//
// Simple wrapper class for OpenGL Texture
// Created by Hao He on 18-1-24.
//

#ifndef PROJECT_TEXTURE_H
#define PROJECT_TEXTURE_H

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>

class Texture
{
public:
    unsigned int ID;

    explicit Texture(const char *imagePath)
    {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        // Set default texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load and generate the texture
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(imagePath, &width, &height, &nrChannels, 0);
        if (data) {
            if (nrChannels == 3) { // RGB
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
                             0, GL_RGB, GL_UNSIGNED_BYTE, data);
            } else if (nrChannels == 4) { // RGBA
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load texture: " << imagePath << std::endl;
        }
        stbi_set_flip_vertically_on_load(false);
        stbi_image_free(data);
    }

    // activeTextureUnit should be a texture unit ID between 0 and 15
    void useTextureUnit(int activeTextureUnit = 0)
    {
        glActiveTexture(GL_TEXTURE0 + activeTextureUnit);
        glBindTexture(GL_TEXTURE_2D, ID);
    }
};


#endif //PROJECT_TEXTURE_H
