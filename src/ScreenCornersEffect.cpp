#include "ScreenCornersEffect.h"

#include <kwin/core/output.h>
#include <kwin/core/rendertarget.h>
#include <kwin/core/renderviewport.h>
#include <kwin/effect/effecthandler.h>
#include <kwin/opengl/glshader.h>
#include <kwin/opengl/glshadermanager.h>
#include <kwin/opengl/glvertexbuffer.h>

#include <epoxy/gl.h>

static const char *fragmentShaderSourceV2 = R"(#version 140
in vec2 v_uv;
out vec4 fragColor;
uniform float cornerX; // 0.0 = left, 1.0 = right
uniform float aaWidth; // anti-aliasing width in UV space
uniform float radius;  // circle radius in UV space (slightly > 1.0 to compensate for pixel centers)
void main() {
    vec2 center = vec2(1.0 - cornerX, 1.0);
    float dist = length(v_uv - center);
    float alpha = smoothstep(radius - aaWidth, radius + aaWidth, dist);
    fragColor = vec4(0.0, 0.0, 0.0, alpha);
}
)";

static const char *vertexShaderSourceV2 = R"(#version 140
in vec2 position;
in vec2 texcoord;
out vec2 v_uv;
uniform mat4 modelViewProjectionMatrix;
void main() {
    v_uv = texcoord;
    gl_Position = modelViewProjectionMatrix * vec4(position, 0.0, 1.0);
}
)";

ScreenCornersEffect::ScreenCornersEffect()
{
    m_shader = KWin::ShaderManager::instance()->loadShaderFromCode(
        QByteArray(vertexShaderSourceV2),
        QByteArray(fragmentShaderSourceV2));
}

ScreenCornersEffect::~ScreenCornersEffect() = default;

bool ScreenCornersEffect::isActive() const
{
    return m_shader && m_shader->isValid();
}

int ScreenCornersEffect::requestedEffectChainPosition() const
{
    return 99;
}

void ScreenCornersEffect::paintScreen(const KWin::RenderTarget &renderTarget,
                                       const KWin::RenderViewport &viewport,
                                       int mask,
                                       const KWin::Region &region,
                                       KWin::LogicalOutput *screen)
{
    KWin::effects->paintScreen(renderTarget, viewport, mask, region, screen);

    if (!KWin::effects->isOpenGLCompositing()) {
        return;
    }
    if (screen && !screen->isInternal()) {
        return;
    }
    if (!m_shader || !m_shader->isValid()) {
        return;
    }

    const qreal scale = viewport.scale();
    const auto targetSize = renderTarget.size();
    const float w = targetSize.width();
    const float r = m_cornerRadius * scale;

    // AA width in UV space: ~1.5 device pixels mapped to 0..1 UV range
    const float aaWidth = 1.5f / r;

    // Extend circle radius by 0.5 device pixels to compensate for pixel-center sampling,
    // so the rounded edge aligns flush with the screen edge instead of 1px inward.
    const float radius = 1.0f + 0.5f / r;

    KWin::ShaderManager::instance()->pushShader(m_shader.get());
    m_shader->setUniform(KWin::GLShader::Mat4Uniform::ModelViewProjectionMatrix, viewport.projectionMatrix());
    m_shader->setUniform("aaWidth", aaWidth);
    m_shader->setUniform("radius", radius);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto drawCorner = [&](float x, float y, float cornerX) {
        m_shader->setUniform("cornerX", cornerX);

        // Quad covering the corner area, UV always 0..1 from top-left to bottom-right
        const std::array<KWin::GLVertex2D, 6> vertices = {{
            {QVector2D(x, y),         QVector2D(0, 0)},
            {QVector2D(x + r, y),     QVector2D(1, 0)},
            {QVector2D(x + r, y + r), QVector2D(1, 1)},

            {QVector2D(x, y),         QVector2D(0, 0)},
            {QVector2D(x + r, y + r), QVector2D(1, 1)},
            {QVector2D(x, y + r),     QVector2D(0, 1)},
        }};

        KWin::GLVertexBuffer *vbo = KWin::GLVertexBuffer::streamingBuffer();
        vbo->reset();
        vbo->setVertices(vertices);
        vbo->render(GL_TRIANGLES);
    };

    // Top-left corner: quad at (0, 0), circle center at UV (1, 1)
    drawCorner(0, 0, 0.0f);

    // Top-right corner: quad at (w-r, 0), circle center at UV (0, 1)
    drawCorner(w - r, 0, 1.0f);

    KWin::ShaderManager::instance()->popShader();
    glDisable(GL_BLEND);
}
