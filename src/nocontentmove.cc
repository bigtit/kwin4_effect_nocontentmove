#include "nocontentmove.h"

// #include "nocontentmoveconfig.h"

#include <kwinglutils.h>
#ifdef KWIN_HAVE_XRENDER_COMPOSITIN
#include "kwinxrenderutils.h"
#endif

#include <KColorScheme>
#include <QVector2D>
#include <QPainter>

namespace KWin {
NoContentMoveEffect::NoContentMoveEffect() : AnimationEffect(), m_active(false), m_moveWindow(nullptr) {
  connect(effects, &EffectsHandler::windowStartUserMovedResized, this, &NoContentMoveEffect::slotWindowStartUserMovedResized);
  connect(effects, &EffectsHandler::windowStepUserMovedResized, this, &NoContentMoveEffect::slotWindowStepUserMovedResized);
  connect(effects, &EffectsHandler::windowFinishUserMovedResized, this, &NoContentMoveEffect::slotWindowFinishUserMovedResized);
}

NoContentMoveEffect::~NoContentMoveEffect() {}

void NoContentMoveEffect::prePaintScreen(ScreenPrePaintData& data, std::chrono::milliseconds presentTime) {
  if (m_active) data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
  AnimationEffect::prePaintScreen(data, presentTime);
}

void NoContentMoveEffect::prePaintWindow(EffectWindow* w, WindowPrePaintData& data, std::chrono::milliseconds presentTime) {
  if (m_active && w == m_moveWindow) data.mask |= PAINT_WINDOW_TRANSFORMED;
  AnimationEffect::prePaintWindow(w, data, presentTime);
}

bool NoContentMoveEffect::supported() {
  // restrict this effect to opengl only
  // return KWin::effects->isOpenGLCompositing();
  return true;
}

// each frame painting
void NoContentMoveEffect::paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data) {
  if (m_active && w == m_moveWindow) {
    // from Plasma/5.24, `data` will be changed by user's operatios in real-time
    // specifically, window layout in `data` will be changed when resized
    // so the resize effect is impossible to be implemented like before Plasma/5.24
    data += (m_originalGeometry.topLeft() - m_currentGeometry.topLeft());
    effects->paintWindow(w, mask, region, data);
    QRegion paintRegion = QRegion(m_currentGeometry);
    float alpha = 0.3f;
    QColor color = KColorScheme(QPalette::Normal, KColorScheme::Selection).background().color();

    if (effects->isOpenGLCompositing()) {
      GLVertexBuffer* vbo = GLVertexBuffer::streamingBuffer();
      vbo->reset();
      vbo->setUseColor(true);
      ShaderBinder binder(ShaderTrait::UniformColor);
      binder.shader()->setUniform(GLShader::ModelViewProjectionMatrix, data.screenProjectionMatrix());
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      color.setAlphaF(alpha);
      vbo->setColor(color);
      QVector<float> verts;
      verts.reserve(paintRegion.rectCount()*12);
      for (const QRect& r : paintRegion) {
        // two triangles for each rectangle
        verts << r.x() + r.width() << r.y();
        verts << r.x() << r.y();
        verts << r.x() << r.y() + r.height();
        verts << r.x() << r.y() + r.height();
        verts << r.x() + r.width() << r.y() + r.height();
        verts << r.x() + r.width() << r.y();
      }
      vbo->setData(verts.count()/2, 2, verts.data(), nullptr);
      vbo->render(GL_TRIANGLES);
      glDisable(GL_BLEND);
    }
    else if (effects->compositingType() == QPainterCompositing) {
      QPainter* painter = effects->scenePainter();
      painter->save();
      color.setAlphaF(alpha);
      for (const QRect& r : paintRegion) painter->fillRect(r, color);
      painter->restore();
    }
#ifdef KWIN_HAVE_XRENDER_COMPOSITIN
    else if (effects->compositingType() == XRenderCompositing) {
      QVector<xcb_rectangle_t> rects;
      for (const QRect& r : paintRegion) {
        xcb_rectangle_t rect = {int16_t(r.x()), int16_t(r.y()), uint16_t(r.width()), uint16_t(r.height())};
        rects << rect;
      }
      xcb_render_fill_rectangles(xcbConnection(), XCB_RENDER_PICT_OP_OVER,
                                 effects->xrenderBufferPicture(), preMultiply(color, alpha),
                                 rects.count(), rects.constData());
    }
#endif
  } else {
    AnimationEffect::paintWindow(w, mask, region, data);
  }
}

// following 3 functions are to update the memory data of moved window
void NoContentMoveEffect::slotWindowStartUserMovedResized(EffectWindow* w) {
  if (w->isUserMove() && !w->isUserResize()) {
    m_active = true;
    m_moveWindow = w;
    m_originalGeometry = w->geometry();
    m_currentGeometry = w->geometry();
    w->addRepaintFull();
  }
}
void NoContentMoveEffect::slotWindowFinishUserMovedResized(EffectWindow* w) {
  if (m_active && w == m_moveWindow) {
    m_active = false;
    m_moveWindow = nullptr;
    effects->addRepaintFull();
  }
}
void NoContentMoveEffect::slotWindowStepUserMovedResized(EffectWindow* w, const QRect& geometry) {
  if (m_active && w == m_moveWindow) {
    m_currentGeometry = geometry;
    effects->addRepaintFull();
  }
}

}
