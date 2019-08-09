#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QWidget>
#include <QtOpenGL>
namespace Ui {
class OpenglWidget;
}

class OpenglWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit OpenglWidget(QWidget *parent = 0);
    ~OpenglWidget();
    void setpictowidget(QImage image);
    int width;
    int height;
    bool changebtnflag;
    bool confirmbtnflag;
    bool lineflag;
    bool setsqurae;
    bool drawlines;
    bool blackflag;
    bool grayflag;
    int subpicclicked;
    void zoomback();
    void setblackback();
    void setgrayback();
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void wheelEvent(QWheelEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private:
    QImage imagenow;
    Ui::OpenglWidget *ui;
    bool readImage(QImage image, GLuint tex_id);
    GLuint texture;
    GLuint confirmtex;
    GLuint canceltex;
    GLuint confirmhovertex;
    GLuint cancelhovertex;
    int MouseCurrentX;
    int MouseCurrentY;
    float deviationOriginX;
    float deviationOriginY;
    float zoomratio;
    float lastzoomratio;
    float imageStartX,imageStartY;
    QPointF getridofblack(float PosX , float PosY, float heightz, float widthz);
    QPointF counterpoint[4];
    QPointF DragstartPos;
    int MoveFlag;
    float DragLastPosX;
    float DragLastPosY;
    void changezuobiao(float X,float Y);
    bool pressflag;
    bool nextbtnflag;
    bool prebtnflag;
    bool yesbtnflag;
    bool nobtnflag;


    QPointF drawstartpos;
    QPointF drawendpos;
    QPointF origindrawstartpos;
    QPointF origindrawendpos;
    QPointF changecoordinate(QPointF point);

    void paintprebtn();
    void paintnextbtn();
    void paintyesbtn();
    void painthoveryesbtn();
    void paintnobtn();
    void paintnohoverbtn();

signals:
    void hasclickedpre();
    void hasclickednext();
    void hasclickedyes();
    void hasclickedno();
    void squraepos(QPointF,QPointF);
    void hasclickedsubpic(int);
    //画方框左上角与右下角xy相对坐标
};

#endif // OPENGLWIDGET_H
