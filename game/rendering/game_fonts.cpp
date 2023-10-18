#define _CRT_SECURE_NO_WARNINGS

#include <memory>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define FONTSTASH_IMPLEMENTATION
#include "../../vendor/fontstash/src/fontstash.h"

#include "../../vendor/glad/glad.h"
#include "../../vendor/glfw/include/glfw/glfw3.h"

#define GLFONTSTASH_IMPLEMENTATION
#include "../../vendor/fontstash/src/gl3corefontstash.h"

#undef ZeroMemory
#include "game_rendering.h"

const char * vShaderColored = R"(
    uniform mat4 modelView;
    uniform mat4 projection;

    attribute vec4 vertexPosition;
    attribute vec2 vertexTexCoord;
    attribute vec4 vertexColor;

    varying vec2 interpolatedTexCoord;
    varying vec4 interpolatedColor;

    void main() {
        interpolatedColor = vertexColor;
        interpolatedTexCoord = vertexTexCoord;
        gl_Position = projection * modelView * vertexPosition;
    }
)";

const char * fShaderColored = R"(
    uniform sampler2D diffuse;

    varying vec2 interpolatedTexCoord;
    varying vec4 interpolatedColor;

    void main() {
        float alpha = texture2D(diffuse, interpolatedTexCoord).a;
        vec4 textColor = clamp(interpolatedColor, 0.0, 1.0);
        gl_FragColor = vec4(textColor.rgb * textColor.a, textColor.a) * alpha; // Premultiplied alpha.
    }
)";

static FONScontext * fs = nullptr;
static int fontId = FONS_INVALID;
static ShaderProgram fontShaderProgram = {};

void InitFontRendering() {
    fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
    Assert(fs != nullptr);
    fontId = fonsAddFont(fs, "sans", "C:\\Projects\\Play2\\content\\DroidSerif-Regular.ttf");
    fontShaderProgram = ShaderProgramCreate(vShaderColored, fShaderColored);
    Assert(fontId != FONS_INVALID);
}

void FontRenderSomeText(float w, float h) {
    ShaderProgramBind(fontShaderProgram);

    glm::mat4 projection = glm::ortho(0.0f, (f32)w, (f32)h, 0.0f, -1.0f, 1.0f);
    glm::mat4 modelView = glm::mat4(1.0f);

    GLint projectionMatrixLoc = glGetUniformLocation(fontShaderProgram.id, "projection");
    glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLint modelViewMatrixLoc = glGetUniformLocation(fontShaderProgram.id, "modelView");
    glUniformMatrix4fv(modelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelView));

    float dx = 100, dy = 100;
    unsigned int white = glfonsRGBA(255, 255, 255, 255);
    unsigned int brown = glfonsRGBA(192, 128, 0, 128);

    fonsSetFont(fs, fontId);
    fonsSetSize(fs, 32.0f);
    fonsSetColor(fs, white);
    fonsDrawText(fs, dx, dy, "The big sliver slut plays apex with us! ", NULL);

}
