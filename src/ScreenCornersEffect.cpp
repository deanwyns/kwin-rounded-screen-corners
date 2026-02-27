#include "ScreenCornersEffect.h"

#include <kwin/core/output.h>
#include <kwin/core/rendertarget.h>
#include <kwin/core/renderviewport.h>
#include <kwin/effect/effecthandler.h>
#include <kwin/opengl/glshader.h>
#include <kwin/opengl/glshadermanager.h>
#include <kwin/opengl/glvertexbuffer.h>

#include <cmath>
#include <epoxy/gl.h>
#include <vector>

ScreenCornersEffect::ScreenCornersEffect() = default;
ScreenCornersEffect::~ScreenCornersEffect() = default;

bool ScreenCornersEffect::isActive() const
{
    return true;
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

    // projectionMatrix works in device pixel coordinates
    const qreal scale = viewport.scale();
    const auto targetSize = renderTarget.size();
    const float w = targetSize.width();
    const float h = targetSize.height();
    const float r = m_cornerRadius * scale;
    const int segments = 32;

    KWin::GLShader *shader = KWin::ShaderManager::instance()->pushShader(KWin::ShaderTrait::UniformColor);
    shader->setUniform(KWin::GLShader::Mat4Uniform::ModelViewProjectionMatrix, viewport.projectionMatrix());

    auto drawCorner = [&](float cornerX, float cornerY, float cx, float cy,
                          float startAngle) {
        std::vector<KWin::GLVertex2D> vertices;
        vertices.reserve(segments * 3);

        for (int i = 0; i < segments; i++) {
            const float a1 = startAngle + (M_PI / 2.0f) * i / segments;
            const float a2 = startAngle + (M_PI / 2.0f) * (i + 1) / segments;

            vertices.push_back({QVector2D(cornerX, cornerY), QVector2D(0, 0)});
            vertices.push_back({QVector2D(cx + r * std::cos(a1), cy - r * std::sin(a1)), QVector2D(0, 0)});
            vertices.push_back({QVector2D(cx + r * std::cos(a2), cy - r * std::sin(a2)), QVector2D(0, 0)});
        }

        KWin::GLVertexBuffer *vbo = KWin::GLVertexBuffer::streamingBuffer();
        vbo->reset();
        vbo->setVertices(vertices);
        vbo->render(GL_TRIANGLES);
    };

    // Top-left: corner at (0,0), circle center at (r, r)
    drawCorner(0, 0, r, r, M_PI / 2.0f);

    // Top-right: corner at (w,0), circle center at (w-r, r)
    drawCorner(w, 0, w - r, r, 0.0f);

    KWin::ShaderManager::instance()->popShader();
}
