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

// core function
void NoContentMoveEffect::paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data) {
  if (m_active && w == m_moveWindow) {
    // draw the window content when moving
    // effects->paintWindow(w, mask, region, data);
    // so just replace the region with m_originalGeometry
    // but note that data (window content) is also modified when moving
    data.translate(m_originalGeometry.x()-m_currentGeometry.x(), m_originalGeometry.y()-m_currentGeometry.y(), 0);
    effects->paintWindow(w, mask, m_originalGeometry, data);
    // no intersected region will be painted with color
    // QRegion intersection = m_originalGeometry.intersected(m_currentGeometry);
    // QRegion paintRegion = QRegion(m_originalGeometry).united(m_currentGeometry).subtracted(intersection);
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
#ifdef KWIN_HAVE_XRENDER_COMPOSITIN
    if (effects->compositingType() == XRenderCompositing) {
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
    if (effects->compositingType() == QPainterCompositing) {
      QPainter* painter = effects->scenePainter();
      painter->save();
      color.setAlphaF(alpha);
      for (const QRect& r : paintRegion) painter->fillRect(r, color);
      painter->restore();
    }
  } else {
    AnimationEffect::paintWindow(w, mask, region, data);
  }
}

// following 3 functions are to update the memory data of moved window
void NoContentMoveEffect::slotWindowStartUserMovedResized(EffectWindow* w) {
  if (w->isUserMove()) {
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
