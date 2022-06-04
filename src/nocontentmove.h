#ifndef KWIN_NOCONTENTMOVE_H
#define KWIN_NOCONTENTMOVE_H

#include <kwinanimationeffect.h>

namespace KWin {
class NoContentMoveEffect : public AnimationEffect {
  Q_OBJECT
public:
  NoContentMoveEffect();
  ~NoContentMoveEffect();
  inline bool provides(Effect::Feature ef) override { return ef == Effect::Resize; }
  inline bool isActive() const override { return m_active || AnimationEffect::isActive(); }
  void prePaintScreen(ScreenPrePaintData& data, std::chrono::milliseconds presentTime) override;
  void prePaintWindow(EffectWindow* w, WindowPrePaintData& data, std::chrono::milliseconds presentTime) override;
  void paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data) override;
  // void reonfigure(ReconfigureFlags) override;

  int requestedEffectChainPosition() const override { return 60; }

  static bool supported();

public Q_SLOTS:
  void slotWindowStartUserMovedResized(KWin::EffectWindow* w);
  void slotWindowStepUserMovedResized(KWin::EffectWindow* w, const QRect& geometry);
  void slotWindowFinishUserMovedResized(KWin::EffectWindow* w);

private:
  bool m_active;
  EffectWindow* m_moveWindow;
  // if x, y is enough, instead of over-used QRect
  QRect m_currentGeometry, m_originalGeometry;
};
}

#endif
