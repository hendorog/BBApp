#ifndef GL_SUB_VIEW_H
#define GL_SUB_VIEW_H

#include <QOpenGLFunctions>
#include <QGLWidget>
#include <QFont>
#include <QFontMetrics>

#include "lib/bb_lib.h"

// Cache Metrics with Font
class GLFont {
public:
    GLFont(const QString &family, int pointSize, int weight, bool italic) :
        font(family, pointSize, weight, italic),
        fontMetrics(font) {}
    ~GLFont();

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

    enum TextAlignment {
        LEFT_ALIGNED, RIGHT_ALIGNED, CENTER_ALIGNED
    };

public:
    GLSubView(QWidget *parent) :
        QGLWidget(parent)
    {
        makeCurrent();
        initializeOpenGLFunctions();
        doneCurrent();
    }

protected:
    void DrawString(const QString &s, const GLFont &f,
                    QPoint p, TextAlignment alignment)
    {
        if(align == RIGHT_ALIGNED) {
            p -= QPoint(f.GetTextWidth(s), 0);
        } else if(alignment == CENTER_ALIGNED) {
            p -= QPoint(f.GetTextWidth(s) / 2, 0);
        }
        renderText(p.x(), p.y(), 0, s, f);
    }

    void DrawString(const QString &s, const QFont &f,
                    int x, int y, TextAlignment alignment)
    {
        DrawString(s, f, QPoint(x, y), alignment);
    }

private:

private:
    DISALLOW_COPY_AND_ASSIGN(GLSubView)
};

#endif // GL_SUB_VIEW_H
