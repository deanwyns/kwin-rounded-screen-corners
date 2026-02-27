#include "ScreenCornersEffect.h"

#include <kwin/effect/effect.h>
#include <kwin/effect/effecthandler.h>

KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(
    ScreenCornersEffect,
    "metadata.json",
    return KWin::effects->isOpenGLCompositing();,
    return true;
)

#include "plugin.moc"
