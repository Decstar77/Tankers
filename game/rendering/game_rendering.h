#pragma once

#include "../../shared/shared_defines.h"

#include "../../vendor/glm/glm/glm.hpp"
#include "../../vendor/glm/glm/gtc/matrix_transform.hpp"
#include "../../vendor/glm/glm/gtc/type_ptr.hpp"

struct ShaderProgram {
    u32 id;
};

ShaderProgram ShaderProgramCreate(const char * vertexShaderSource, const char * fragmentShaderSource);
void          ShaderProgramBind(const ShaderProgram & program);
void          ShaderProgramSetUniform(const ShaderProgram & program, const char * name, i32 value);


void InitFontRendering();
void FontRenderSomeText(f32 w, f32 h);