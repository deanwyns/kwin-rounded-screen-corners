kcoreaddons_add_plugin(kwin_effect_roundedscreencorners
    SOURCES
        plugin.cpp
        ScreenCornersEffect.cpp
    INSTALL_NAMESPACE "kwin/effects/plugins"
)

target_link_libraries(kwin_effect_roundedscreencorners PRIVATE
    KWin::kwin
    Qt6::Core
    Qt6::Gui
    epoxy::epoxy
)
