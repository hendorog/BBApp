#ifndef GL_SUB_VIEW_H
#define GL_SUB_VIEW_H

#include <QOpenGLFunctions>
#include <QGLWidget>
#include <QFont>
#include <QFontMetrics>

#include "lib/bb_lib.h"
#include "model/session.h"

// Cache Metrics with Font
class GLFont {
public:
    GLFont(const QString &family, int pointSize, int weight = -1, bool italic = false) :
        font(family, pointSize, weight, italic),
        fontMetrics(font) {}
    ~GLFont() {}

    const QFont& Font() const { return font; }
    const QFontMetrics& FontMetrics() const { return fontMetrics; }

    int GetTextWidth(const QString &s) const
    {
        return fontMetrics.width(s);
    }

private:
    QFont font;
    QFontMetrics fontMetrics;
};

// Extend the QGLWidget class with custom text rendering functions
class GLSubView : public QGLWidget, public QOpenGLFunctions {
    Q_OBJECT

public:
    GLSubView(Session *sPtr, QWidget *parent = 0) :
        QGLWidget(parent), sessionPtr(sPtr)
    {
        makeCurrent();
        initializeOpenGLFunctions();
        doneCurrent();

        setAutoFillBackground(false);
    }

protected:
    Session* GetSession() const { return sessionPtr; }

    enum TextAlignment {
        LEFT_ALIGNED, RIGHT_ALIGNED, CENTER_ALIGNED
    };

    void DrawString(const QString &s, const GLFont &f,
                    QPoint p, TextAlignment alignment)
    {
        if(alignment == RIGHT_ALIGNED) {
            p -= QPoint(f.GetTextWidth(s), 0);
        } else if(alignment == CENTER_ALIGNED) {
            p -= QPoint(f.GetTextWidth(s) / 2, 0);
        }
        renderText(p.x(), p.y(), 0, s, f.Font());
    }

    void DrawString(const QString &s, const GLFont &f,
                    int x, int y, TextAlignment alignment)
    {
        DrawString(s, f, QPoint(x, y), alignment);
    }

private:
    Session *sessionPtr;

private:
    DISALLOW_COPY_AND_ASSIGN(GLSubView)
};

// View synchronized via supplied mutex
// Non-threaded drawing
// Intended for use with demod views
class GLSyncSubView : public GLSubView {
    Q_OBJECT

public:
    GLSyncSubView(Session *sPtr, std::mutex &lock, QWidget *parent = 0) :
        GLSubView(sPtr, parent),
        syncLock(lock)
    {
    }
    ~GLSyncSubView() {}

protected:
    virtual void Paint() = 0;

private:
    // Painting synchronized via external lock
    // Drawing happens in Paint()
    void paintEvent(QPaintEvent *)
    {
        if(syncLock.try_lock()) {
            Paint();
        }
    }

    // Does not own
    std::mutex &syncLock;

private:
    DISALLOW_COPY_AND_ASSIGN(GLSyncSubView)
};

#endif // GL_SUB_VIEW_H
