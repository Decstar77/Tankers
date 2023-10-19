#pragma once

#include "../../shared/shared_defines.h"
#include "../../shared/shared_containers.h"

#include "../../vendor/glm/glm/glm.hpp"
#include "../../vendor/glm/glm/vec2.hpp"
#include "../../vendor/glm/glm/vec3.hpp"
#include "../../vendor/glm/glm/vec4.hpp"
#include "../../vendor/glm/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/glm/gtc/type_ptr.hpp"

enum DrawCommandType {
    INVALID = 0,
    DRAW_RECT,
    DRAW_CIRCLE,
    DRAW_ROUNDED_RECT,
    DRAW_TEXT,
    DRAW_SPRITE,
};

struct DrawCommand {
    DrawCommandType type;
    glm::vec4 color;
    union {
        struct {
            glm::vec2 tl;
            glm::vec2 br;
            glm::vec2 bl;
            glm::vec2 tr;
        } rect;
        struct {
            glm::vec2 center;
            f32 radius;
        } circle;
        struct {
            glm::vec2 tl;
            glm::vec2 br;
            glm::vec2 bl;
            glm::vec2 tr;
            f32 radius;
        } roundedRect;
        struct {
            glm::vec2 pos;
            SmallString text;
        } text;
    };
};

struct DrawCommands {
    FixedList<DrawCommand, 1024> commands;
};

enum VertexBufferType {
    VERTEX_BUFFER_TYPE_POS,
    VERTEX_BUFFER_TYPE_POS_TEX,
};

struct VertexBuffer {
    u32 vao;
    u32 vbo;
    i32 size;
    i32 stride;
    i32 vertexCount;
    VertexBufferType type;
};

struct ShaderProgram {
    u32 id;
};

void            GLResetSurface(f32 w, f32 h);

ShaderProgram   GLShaderProgramCreate(const char * vertexShaderSource, const char * fragmentShaderSource);
void            GLShaderProgramBind(const ShaderProgram & program);
void            GLShaderProgramSetUniform(const ShaderProgram & program, const char * name, i32 value);
void            GLShaderProgramSetInt(const ShaderProgram & program, const char * name, i32 value);
void            GLShaderProgramSetVec4(const ShaderProgram & program, const char * name, const glm::vec4 & vec);
void            GLShaderProgramSetMat4(const ShaderProgram & program, const char * name, const glm::mat4 & mat);

VertexBuffer    GLVertexBufferCreate(VertexBufferType type, i32 size);
void            GLVertexBufferBind(const VertexBuffer & buffer);
void            GLVertexBufferUpdateData(const VertexBuffer & buffer, const void * data);
void            GLVertexBufferDraw(const VertexBuffer & buffer);

void            GLEnableBlending();
void            GLEnablePremultipliedAlphaBlending();

void            GLInitShapeRendering();
void            GLShapeRenderRect(const DrawCommand & cmd);
void            GLShapeRenderCircle(const DrawCommand & cmd);
void            GLShapeRenderRoundedRect(const DrawCommand & cmd);

void            GLInitFontRendering();

void            FontRenderSomeText(f32 w, f32 h);

struct Camera {
    glm::vec2   pos;
    f32         width;
    f32         height;
};

void            InitRendering(f32 w, f32 h);

Camera &        GetCamera();
glm::mat4       GetCameraProjectionMatrix();

void            DrawRect(DrawCommands & drawCommands, glm::vec2 bl, glm::vec2 tr, glm::vec4 color = glm::vec4(1, 1, 1, 1));
void            DrawRect(DrawCommands & cmds, glm::vec2 center, glm::vec2 dim, f32 rot, glm::vec4 color = glm::vec4(1, 1, 1, 1));
// void            DrawCircle(glm::vec2 center, f32 radius, glm::vec4 color = glm::vec4(1, 1, 1, 1));
// void            DrawRoundedRect(glm::vec2 center, glm::vec2 size, f32 radius, glm::vec4 color = glm::vec4(1, 1, 1, 1));
// void            DrawText(glm::vec2 pos, const char * text, glm::vec4 color = glm::vec4(1, 1, 1, 1));
// void            DrawSprite(glm::vec2 pos, glm::vec2 size, glm::vec4 color = glm::vec4(1, 1, 1, 1));

void            RenderCommit(DrawCommands & cmds);