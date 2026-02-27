#pragma once

#include <kwin/effect/effect.h>

class ScreenCornersEffect : public KWin::Effect
{
    Q_OBJECT

public:
    ScreenCornersEffect();
    ~ScreenCornersEffect() override;

    void paintScreen(const KWin::RenderTarget &renderTarget,
                     const KWin::RenderViewport &viewport,
                     int mask,
                     const KWin::Region &region,
                     KWin::LogicalOutput *screen) override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override;

private:
    int m_cornerRadius = 20; // logical pixels
};
