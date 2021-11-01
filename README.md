# kwin4_effect_nocontentmove
A simple and badly-implemented kwin effect plugin that hides content when moving a window, for Plasma/5.22, and should work for other Plasma versions.

- tested on compositor with OpenGL backend.
- inspired by `kwin/src/effects/resize`.

Prerequisite:
```bash
pacman -S cmake extra-cmake-modules
```

Installation:
```bash
git clone https://github.com/bigtit/kwin4_effect_nocontentmove
cd kwin4_effect_nocontentmove
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
# open `System Settings -> Workspace Behavior -> Desktop Effects`
# Search `no content move`, check the box and apply
```
Uninstallation:
```bash
# uncheck the setting box
sudo make uninstall
```
