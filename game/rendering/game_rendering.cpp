#include "game_rendering.h"

static f32              mainSurfaceWidth = 0;
static f32              mainSurfaceHeight = 0;
static glm::vec4        viewport = {};
static Camera           camera = {};
static glm::mat4        cameraProjection = glm::mat4(1);
static glm::mat4        screenProjection = glm::mat4(1);

void InitRendering(f32 w, f32 h) {
    camera.width = 480;
    camera.height = 270;
    camera.pos = glm::vec2(0, 0);
    GLResetSurface(w, h);

    GLInitShapeRendering();
    GLInitFontRendering();
}

Camera & GetCamera() {
    return camera;
}

glm::mat4 GetCameraProjectionMatrix() {
    return cameraProjection;
}

void DrawRect(DrawCommands & drawCommands, glm::vec2 bl, glm::vec2 tr, glm::vec4 color) {
    DrawCommand cmd = {};
    cmd.type = DRAW_RECT;
    cmd.color = color;

    cmd.rect.bl = bl;
    cmd.rect.tr = tr;
    cmd.rect.br = glm::vec2(cmd.rect.tr.x, cmd.rect.bl.y);
    cmd.rect.tl = glm::vec2(cmd.rect.bl.x, cmd.rect.tr.y);

    drawCommands.commands.Add(cmd);
}

void DrawRect(DrawCommands & drawCommands, glm::vec2 center, glm::vec2 dim, f32 rot, glm::vec4 color) {
    DrawCommand cmd = {};
    cmd.type = DRAW_RECT;
    cmd.color = color;

    cmd.rect.bl = -dim / 2.0f;
    cmd.rect.tr = dim / 2.0f;
    cmd.rect.br = glm::vec2(cmd.rect.tr.x, cmd.rect.bl.y);
    cmd.rect.tl = glm::vec2(cmd.rect.bl.x, cmd.rect.tr.y);

    glm::mat2 rotationMatrix = glm::mat2(cos(rot), -sin(rot), sin(rot), cos(rot));
    cmd.rect.bl = rotationMatrix * cmd.rect.bl;
    cmd.rect.tr = rotationMatrix * cmd.rect.tr;
    cmd.rect.br = rotationMatrix * cmd.rect.br;
    cmd.rect.tl = rotationMatrix * cmd.rect.tl;

    cmd.rect.bl += center;
    cmd.rect.tr += center;
    cmd.rect.br += center;
    cmd.rect.tl += center;

    drawCommands.commands.Add(cmd);
}

void RenderCommit(DrawCommands & cmds) {
    const i32 cmdCount = cmds.commands.GetCount();
    for (i32 cmdIndex = 0; cmdIndex < cmdCount; cmdIndex++) {
        const DrawCommand & cmd = cmds.commands[cmdIndex];
        switch (cmd.type) {
        case DRAW_RECT:                 GLShapeRenderRect(cmd); break;
        case DRAW_CIRCLE:               GLShapeRenderCircle(cmd); break;
        case DRAW_ROUNDED_RECT:         GLShapeRenderRoundedRect(cmd); break;
        default: Assert(false); break;
        }
    }
}

#include "../../vendor/glad/glad.h"

void GLResetSurface(f32 w, f32 h) {
    f32 cameraWidth = camera.width;
    f32 cameraHeight = camera.height;

    f32 ratioX = (f32)w / (f32)cameraWidth;
    f32 ratioY = (f32)h / (f32)cameraHeight;
    f32 ratio = ratioX < ratioY ? ratioX : ratioY;

    i32 viewWidth = (i32)(cameraWidth * ratio);
    i32 viewHeight = (i32)(cameraHeight * ratio);

    i32 viewX = (i32)((w - cameraWidth * ratio) / 2);
    i32 viewY = (i32)((h - cameraHeight * ratio) / 2);

    glViewport(viewX, viewY, viewWidth, viewHeight);

    mainSurfaceWidth = w;
    mainSurfaceHeight = h;
    viewport = glm::vec4(viewX, viewY, viewWidth, viewHeight);
    cameraProjection = glm::ortho(0.0f, (f32)cameraWidth, 0.0f, (f32)cameraHeight, -1.0f, 1.0f);
    screenProjection = glm::ortho(0.0f, (f32)w, (f32)h, 0.0f, -1.0f, 1.0f);
}

ShaderProgram GLShaderProgramCreate(const char * vertexShaderSource, const char * fragmentShaderSource) {
    ShaderProgram program = {};
    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    i32 success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
        Assert(false);
    }

    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
        Assert(false);
    }

    program.id = glCreateProgram();
    glAttachShader(program.id, vertexShader);
    glAttachShader(program.id, fragmentShader);
    glLinkProgram(program.id);

    glGetProgramiv(program.id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program.id, sizeof(infoLog), nullptr, infoLog);
        Assert(false);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void GLShaderProgramBind(const ShaderProgram & program) {
    glUseProgram(program.id);
}

void GLShaderProgramSetInt(const ShaderProgram & program, const char * name, i32 value) {
    glUniform1i(glGetUniformLocation(program.id, name), value);
}

void GLShaderProgramSetVec4(const ShaderProgram & program, const char * name, const glm::vec4 & vec) {
    glUniform4f(glGetUniformLocation(program.id, name), vec.x, vec.y, vec.z, vec.w);
}

void GLShaderProgramSetMat4(const ShaderProgram & program, const char * name, const glm::mat4 & mat) {
    glUniformMatrix4fv(glGetUniformLocation(program.id, name), 1, GL_FALSE, glm::value_ptr(mat));
}

VertexBuffer GLVertexBufferCreate(VertexBufferType type, i32 size) {
    VertexBuffer buffer = {};
    buffer.size = size;
    buffer.type = type;
    glGenVertexArrays(1, &buffer.vao);
    glGenBuffers(1, &buffer.vbo);
    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

    switch (type) {
    case VERTEX_BUFFER_TYPE_POS: {
        i32 stride = 2 * sizeof(f32);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *)0);
        buffer.stride = stride;
        buffer.vertexCount = size / stride;
        printf("Drawing %d vertices\n", buffer.vertexCount);
    } break;
    case VERTEX_BUFFER_TYPE_POS_TEX: {
        i32 stride = 4 * sizeof(f32);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(2 * sizeof(f32)));
        buffer.stride = stride;
        buffer.vertexCount = size / stride;
    } break;
    default: Assert(false); break;
    }

    return buffer;
}

void GLVertexBufferBind(const VertexBuffer & buffer) {
    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
}

void GLVertexBufferUpdateData(const VertexBuffer & buffer, const void * data) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer.size, data);
}

void GLVertexBufferDraw(const VertexBuffer & buffer) {
    glDrawArrays(GL_TRIANGLES, 0, buffer.vertexCount);
}

void GLEnableBlending() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GLEnablePremultipliedAlphaBlending() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}
