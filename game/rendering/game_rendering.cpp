#include "game_rendering.h"

#include "../../vendor/glad/glad.h"

ShaderProgram ShaderProgramCreate(const char * vertexShaderSource, const char * fragmentShaderSource) {
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

void ShaderProgramBind(const ShaderProgram & program) {
    glUseProgram(program.id);
}

void ShaderProgramSetUniform(const ShaderProgram & program, const char * name, i32 value) {
    glUniform1i(glGetUniformLocation(program.id, name), value);
}
