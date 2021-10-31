########################
# Effect

# Source files
set(effect_SRCS
  nocontentmove.cc
  main.cc
)

add_library (kwin4_effect_nocontentmove SHARED ${effect_SRCS})

target_link_libraries(kwin4_effect_nocontentmove
  Qt5::Core
  Qt5::Gui
  KF5::ConfigCore
  KF5::ConfigWidgets
  KF5::ConfigGui
  KF5::CoreAddons
  KF5::WindowSystem
  kwineffects::kwineffects
  kwineffects::kwinglutils
  epoxy::epoxy
)

install(
  TARGETS
    kwin4_effect_nocontentmove
  DESTINATION
    ${PLUGIN_INSTALL_DIR}/kwin/effects/plugins/
)
