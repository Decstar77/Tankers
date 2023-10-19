#include "game_rendering.h"

#include "../../vendor/glad/glad.h"

static const char * vShaderColored = R"(
        #version 330 core
        layout (location = 0) in vec2 pos;
        void main()
        {
            gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
        }
)";

static const char * fShaderColored = R"(
        #version 330 core
        out vec4 FragColor;

        uniform int  mode;
        uniform vec4 color;
        uniform vec4 shapePosAndSize;
        uniform vec4 shapeRadius;

        // from http://www.iquilezles.org/www/articles/distfunctions/distfunctions

        float CircleSDF(vec2 r, vec2 p, float rad) {
            return 1 - max(length(p - r) - rad, 0);
        }

        float BoxSDF(vec2 r, vec2 p, vec2 s) {
            return 1 - length(max(abs(p - r) - s, 0));
        }

        float RoundedBoxSDF(vec2 r, vec2 p, vec2 s, float rad) {
            return 1 - (length(max(abs(p - r) - s + rad, 0)) - rad);
        }

        void main() {
            vec2 s = shapePosAndSize.zw;
            vec2 r = shapePosAndSize.xy;
            vec2 p = gl_FragCoord.xy;

            if (mode == 0) {
                FragColor = color;
            } else if (mode == 1) {
                float d = CircleSDF(r, p, shapeRadius.x);
                d = clamp(d, 0.0, 1.0);
                FragColor = color * d; //vec4(color.xyz, color.w * d);
                //FragColor = color;
            } else if (mode == 2) {
                float d = RoundedBoxSDF(r, p, s / 2, shapeRadius.x);
                d = clamp(d, 0.0, 1.0);
                FragColor = vec4(color.xyz, color.w * d);
            } else {
                FragColor = vec4(1, 0, 1, 1);
            }
        }
)";

static ShaderProgram shapeProgram = {};
static VertexBuffer shapeVertexBuffer = {};

void GLInitShapeRendering() {
    shapeProgram = GLShaderProgramCreate(vShaderColored, fShaderColored);
    shapeVertexBuffer = GLVertexBufferCreate(VERTEX_BUFFER_TYPE_POS, 6 * 2 * sizeof(f32));
}

void GLShapeRenderRect(const DrawCommand & cmd) {
    glm::mat4 p = GetCameraProjectionMatrix();

    glm::vec4 v1 = p * glm::vec4(cmd.rect.tl.x, cmd.rect.tl.y, 0.0f, 1.0f);
    glm::vec4 v2 = p * glm::vec4(cmd.rect.bl.x, cmd.rect.bl.y, 0.0f, 1.0f);
    glm::vec4 v3 = p * glm::vec4(cmd.rect.br.x, cmd.rect.br.y, 0.0f, 1.0f);
    glm::vec4 v4 = p * glm::vec4(cmd.rect.tl.x, cmd.rect.tl.y, 0.0f, 1.0f);
    glm::vec4 v5 = p * glm::vec4(cmd.rect.br.x, cmd.rect.br.y, 0.0f, 1.0f);
    glm::vec4 v6 = p * glm::vec4(cmd.rect.tr.x, cmd.rect.tr.y, 0.0f, 1.0f);

    f32 vertices[6][2] = {
        { v1.x, v1.y },
        { v2.x, v2.y },
        { v3.x, v3.y },
        { v4.x, v4.y },
        { v5.x, v5.y },
        { v6.x, v6.y },
    };

    GLEnableBlending();
    GLShaderProgramBind(shapeProgram);
    GLShaderProgramSetInt(shapeProgram, "mode", 0);
    GLShaderProgramSetVec4(shapeProgram, "color", cmd.color);

    GLVertexBufferBind(shapeVertexBuffer);
    GLVertexBufferUpdateData(shapeVertexBuffer, vertices);
    GLVertexBufferDraw(shapeVertexBuffer);
}

void GLShapeRenderCircle(const DrawCommand & cmd) {
    Assert(false);
}

void GLShapeRenderRoundedRect(const DrawCommand & cmd) {
    Assert(false);
}
