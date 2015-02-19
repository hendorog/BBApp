#ifndef GL_SUB_VIEW_H
#define GL_SUB_VIEW_H

#include <QOpenGLFunctions>
#include <QGLWidget>
#include <QFont>
#include <QFontMetrics>

#include "lib/bb_lib.h"
#include "model/session.h"

#define sh_save_gl_state \
    glPushAttrib(GL_ALL_ATTRIB_BITS);

// Convenience wrapper of Font and Metrics for OpenGL rasterization
class GLFont {
public:
    GLFont(int size, const QString &family = "Arial", int weight = -1, bool italic = false) :
        font(family, size, weight, italic),
        fontMetrics(font)
    {
        //font.setStyleStrategy(QFont::OpenGLCompatible);
    }
    ~GLFont() {}

    const QFont& Font() const { return font; }
    const QFontMetrics& FontMetrics() const { return fontMetrics; }

    int GetTextWidth(const QString &s) const {
        return fontMetrics.width(s);
    }

    int GetTextHeight() const {
        return fontMetrics.ascent() + fontMetrics.descent();
        return fontMetrics.height();
    }

private:
    QFont font;
    QFontMetrics fontMetrics;
};

struct TextToDraw {
    QString s;
    QPoint p;

};

// Extend the QGLWidget class with custom text rendering functions
class GLSubView : public QGLWidget, public QOpenGLFunctions {
    Q_OBJECT

public:
    GLSubView(Session *sPtr, QWidget *parent = 0);
    ~GLSubView();

protected:
    Session* GetSession() const { return sessionPtr; }

    enum TextAlignment {
        LEFT_ALIGNED,
        RIGHT_ALIGNED,
        CENTER_ALIGNED
    };

    void DrawString(const QString &s,
                    const GLFont &f,
                    QPoint p,
                    TextAlignment alignment);
    void DrawString(const QString &s,
                    const GLFont &f,
                    int x, int y,
                    TextAlignment alignment);
    // Overpainting
    void DrawString(QPainter &painter,
                    const QString &s,
                    const GLFont &f,
                    QPoint p,
                    TextAlignment alignment);
    void DrawString(QPainter &painter,
                    const QString &s,
                    const GLFont &f,
                    int x, int y,
                    TextAlignment alignment);

    void SetGraticuleDimensions(QPoint bottomLeft, QPoint size)
    {
        grat_ll = bottomLeft;
        grat_sz = size;
        grat_ul.setX(grat_ll.x());
        grat_ul.setY(grat_ll.y() + grat_sz.y());
    }
    void SetGraticuleDimensions(int left, int bottom, int width, int height)
    {
        SetGraticuleDimensions(QPoint(left, bottom), QPoint(width, height));
    }

    void DrawGraticule();

    QPoint grat_ll;
    QPoint grat_ul;
    QPoint grat_sz;
    QRect grat;

    GLuint gratVBO;
    GLuint gratBorderVBO;
    int innerGratPoints, borderGratPoints;

private:
    Session *sessionPtr;

private:
    DISALLOW_COPY_AND_ASSIGN(GLSubView)
};

//// View synchronized via supplied mutex
//// Non-threaded drawing
//// Intended for use with demod views
//class GLSyncSubView : public GLSubView {
//    Q_OBJECT

//public:
//    GLSyncSubView(Session *sPtr, std::mutex &lock, QWidget *parent = 0) :
//        GLSubView(sPtr, parent),
//        syncLock(lock)
//    {
//    }
//    ~GLSyncSubView() {}

//protected:
//    virtual void Paint() = 0;

//private:
//    // Painting synchronized via external lock
//    // Drawing happens in Paint()
//    void paintEvent(QPaintEvent *)
//    {
//        if(syncLock.try_lock()) {
//            Paint();
//        }
//    }

//    // Does not own
//    std::mutex &syncLock;

//private:
//    DISALLOW_COPY_AND_ASSIGN(GLSyncSubView)
//};

#endif // GL_SUB_VIEW_H
