# kwin-rounded-screen-corners

A KWin effect that renders smooth, anti-aliased rounded corners on your screen edges using SDF shaders.

Currently draws corners at the top of the internal (laptop) screen only.

## Building

Requires KDE Plasma 6 / KWin 6 development packages.

```bash
cmake -B build
cmake --build build
sudo cmake --install build
```

## Enabling

After installing, enable the effect in **System Settings > Desktop Effects > Rounded Screen Corners**.

## License

GPL-3.0 — see [LICENSE](LICENSE) for details.
