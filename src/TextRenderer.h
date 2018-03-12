//
// Created by 何昊 on 2018/03/12.
//

#ifndef PROJECT_TEXTRENDERER_H
#define PROJECT_TEXTRENDERER_H


// The Free Type Font rendering library
#include <ft2build.h>
#include FT_FREETYPE_H

// GLM Math Library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLAD: A library that wraps OpenGL functions to make things easier
//       Note that GLAD MUST be included before GLFW
#include "glad/glad.h"

#include <map>
#include <string>

#include "Shader.h"

class TextRenderer
{
public:
    TextRenderer();
    ~TextRenderer() = default;

    void renderText(Shader &s, std::string text,
                    GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

    // Projection matrix used by text
    glm::mat4 projection;
private:
    struct Character {
        GLuint     TextureID;  // ID handle of the glyph texture
        glm::ivec2 Size;       // Size of glyph
        glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
        GLuint     Advance;    // Offset to advance to next glyph
    };

    std::map<GLchar, Character> Characters;

    unsigned int VAO, VBO;
};


#endif //PROJECT_TEXTRENDERER_H
