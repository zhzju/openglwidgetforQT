#include "openglwidget.h"
#include "ui_openglwidget.h"
#include <math.h>
#define zoomRatioMax 50
#define zoomRationMin 1

const GLfloat pi = 3.1415926536f;
OpenglWidget::OpenglWidget(QWidget *parent) :
    QGLWidget(parent),
    ui(new Ui::OpenglWidget)
{
    ui->setupUi(this);
    width = 0;
    height = 0;
    zoomratio = 1.0;
    lastzoomratio = 1.0;
    deviationOriginX = 0;
    deviationOriginY = 0;
    imageStartX = 0;
    imageStartY = 0;
    DragstartPos = QPointF(0.0f,0.0f);
    counterpoint[0]=QPointF(-1.0f,1.0f);
    counterpoint[1]=QPointF(1.0f,1.0f);
    counterpoint[2]=QPointF(1.0f,-1.0f);
    counterpoint[3]=QPointF(-1.0f,-1.0f);
    DragLastPosX = 0;
    DragLastPosY = 0;
    MoveFlag = 0;
    setMouseTracking(true);
    pressflag = false;
    lineflag = false;
    drawlines = false;
    GLuint tex[6];
    glGenTextures(6, tex); //分配纹理id,必须这样写
    texture = tex[1];
    confirmtex = tex[2];
    canceltex = tex[3];
    confirmhovertex = tex[4];
    cancelhovertex = tex[5]; //五个纹理id可选

    drawstartpos = QPointF(0.0f,0.0f);
    drawendpos = QPointF(0.0f,0.0f);
    origindrawstartpos = QPointF(0.0f,0.0f);
    origindrawendpos = QPointF(0.0f,0.0f);
    setsqurae = false; // 用于开启/关闭人工画线框的功能
    blackflag = false; //用于置背景色为全黑
    grayflag = false; //用于置背景色为全灰
    subpicclicked = -1; //用于处理点击小图片的函数，零部件分析板块
}

OpenglWidget::~OpenglWidget()
{
    delete ui;
}

void OpenglWidget::initializeGL()
/***************************
 Effect:初始化opengl：打开2D贴图映射，打开抗锯齿状态，清空颜色表，清空深度缓存，打开深度缓存使能
 parameter:无
 ***************************/
{
    makeCurrent();
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0,0.0,0.0,0.0);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void OpenglWidget::wheelEvent(QWheelEvent *e)
/***************************
 Effect:鼠标滚轮时间，根据鼠标滚轮的状态设定图片放大倍数，内容包含坐标变换，将Qt坐标系变换为opengl的世界坐标系
 parameter:无
 ***************************/
{
    MouseCurrentX = e->pos().x();
    MouseCurrentY = e->pos().y();
    if(MouseCurrentX <0 || MouseCurrentX > width || MouseCurrentY < 0 || MouseCurrentY > height)
        return;
    float deltaDegree = e->delta()*1.0/720*2.2;
    if (zoomratio+deltaDegree > zoomRatioMax ||
            zoomratio+deltaDegree < zoomRationMin-0.1)
        return;
    zoomratio += deltaDegree;
    float MousecalX,MousecalY;
    MousecalX = (-deviationOriginX + MouseCurrentX);
    MousecalY = (-deviationOriginY + MouseCurrentY);
    imageStartX = MouseCurrentX-MousecalX*zoomratio/lastzoomratio;
    imageStartY = MouseCurrentY-MousecalY*zoomratio/lastzoomratio;
    QPointF afterpoint;
    afterpoint = getridofblack(imageStartX,imageStartY,height* zoomratio,width* zoomratio);
    imageStartX = afterpoint.x();
    imageStartY = afterpoint.y();
    if(imageStartX<1&&imageStartX>-1) imageStartX = 0;
    if(imageStartY<1&&imageStartY>-1) imageStartY = 0;
//    qDebug()<<imageStartX<<imageStartY;
    counterpoint[0].setX(imageStartX);
    counterpoint[0].setY(imageStartY);
    counterpoint[1].setX(counterpoint[0].x()+width* zoomratio);
    counterpoint[1].setY(counterpoint[0].y());
    counterpoint[2].setX(counterpoint[1].x());
    counterpoint[2].setY(counterpoint[1].y()+height* zoomratio);
    counterpoint[3].setX(counterpoint[2].x()-width* zoomratio);
    counterpoint[3].setY(counterpoint[2].y());
    deviationOriginX = imageStartX;
    deviationOriginY = imageStartY;
    DragLastPosX = imageStartX;
    DragLastPosY = imageStartY;
    lastzoomratio = zoomratio;
    QPointF topleft = QPoint(width/2.0,height/2.0);
    for(int i = 0 ; i < 4 ; i++){
    counterpoint[i].setX(counterpoint[i].x() -topleft.x());
    counterpoint[i].setY(-counterpoint[i].y() +topleft.y());
    counterpoint[i].setX(counterpoint[i].x()/(width/2.0));
    counterpoint[i].setY(counterpoint[i].y()/(height/2.0));
    }//坐标系变换，原点变换至屏幕中央
    updateGL();
}

void OpenglWidget::paintGL()
/***************************
 Effect:opengl绘制函数对opengl当前窗口进行绘制，绘制内容主要有：当前图片、左右切换按钮
        是，否按钮，边框线，用户自定义右键拉框线等，是否绘制由各个标志位决定
 parameter:无
 ***************************/
{
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glBindTexture(GL_TEXTURE_2D, texture);
     glEnable(GL_TEXTURE_2D);    //启用2D纹理映射,将纹理图片贴到四边形上，x，y，z轴坐标
     glLoadIdentity();
     glBegin(GL_QUADS);
     glTexCoord2f(0.0f, 0.0f);   //纹理坐标配置，左下角
     glVertex3f(counterpoint[3].x(), counterpoint[3].y(), 0.0f);
     glTexCoord2f(1.0f, 0.0f);   //纹理坐标配置，右下角
     glVertex3f(counterpoint[2].x(), counterpoint[2].y(), 0.0f);
     glTexCoord2f(1.0f, 1.0f);   //纹理坐标配置，右上角
     glVertex3f(counterpoint[1].x(), counterpoint[1].y(), 0.0f);
     glTexCoord2f(0.0f, 1.0f);   //纹理坐标配置，左上角
     glVertex3f(counterpoint[0].x(), counterpoint[0].y(), 0.0f);
     glEnd();
     glDisable(GL_TEXTURE_2D);

     if(blackflag)
     {
         glClearColor(0.0,0.0,0.0,0.0);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     }
     if(grayflag)
     {
         glClearColor(0.5,0.5,0.5,0.5);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     }

     if(drawlines && setsqurae)
     {
         glEnable(GL_LINE_SMOOTH);
         glEnable(GL_LINE_STIPPLE);
         glColor3f(1.0f, 0.0f, 0.0f);
         glLineWidth(3.0f);
         glBegin(GL_LINES);
         glVertex3f(drawstartpos.x(), drawstartpos.y(), 0.0f);
         glVertex3f(  drawendpos.x(), drawstartpos.y(), 0.0f);
         glVertex3f(  drawendpos.x(), drawstartpos.y(), 0.0f);
         glVertex3f(  drawendpos.x(),   drawendpos.y(), 0.0f);
         glVertex3f(  drawendpos.x(),   drawendpos.y(), 0.0f);
         glVertex3f(drawstartpos.x(),   drawendpos.y(), 0.0f);
         glVertex3f(drawstartpos.x(),   drawendpos.y(), 0.0f);
         glVertex3f(drawstartpos.x(), drawstartpos.y(), 0.0f);
         glEnd();
         glDisable(GL_LINE_SMOOTH);
         glDisable(GL_LINE_STIPPLE);
     }
     if(lineflag){
     glEnable(GL_LINE_SMOOTH);
     glEnable(GL_LINE_STIPPLE);
     glColor3f(1.0f, 1.0f, 1.0f);
     glLineWidth(1.2f);
     glBegin(GL_LINES);
     glVertex3f( 0.9999f, 0.9999f, 0.0f);
     glVertex3f(-0.9999f, 0.9999f, 0.0f);
     glVertex3f(-0.9999f, 0.9999f, 0.0f);
     glVertex3f(-0.9999f,-0.9999f, 0.0f);
     glVertex3f(-0.9999f,-0.9999f, 0.0f);
     glVertex3f( 0.9999f,-0.9999f, 0.0f);
     glVertex3f( 0.9999f,-0.9999f, 0.0f);
     glVertex3f( 0.9999f, 0.9999f, 0.0f);
     glEnd();
     glDisable(GL_LINE_SMOOTH);
     glDisable(GL_LINE_STIPPLE);}
     
//     glFlush();
}


void OpenglWidget::resizeGL(int w, int h)
/***************************
 Effect:opengl的resize函数，当opengl窗口大小发生改变时调用，设定为在当前窗口大小改变时
        将全局放大倍数设置为1
 parameter:w当前窗口宽，h当前窗口高
 ***************************/
{
    if(w>0 && h>0)
    {glViewport(0,0,(GLint)w,(GLint)h);
    width = w;
    height = h;
    zoomratio = 1.0;
    lastzoomratio = 1.0;
    deviationOriginX = 0;
    deviationOriginY = 0;
    imageStartX = 0;
    imageStartY = 0;
    DragstartPos = QPointF(0.0f,0.0f);
    counterpoint[0]=QPointF(-1.0f,1.0f);
    counterpoint[1]=QPointF(1.0f,1.0f);
    counterpoint[2]=QPointF(1.0f,-1.0f);
    counterpoint[3]=QPointF(-1.0f,-1.0f);
    DragLastPosX = 0;
    DragLastPosY = 0;
    MoveFlag = 0;
    paintGL();
    }
}
bool OpenglWidget::readImage(QImage image , GLuint tex_id)
/***************************
 Effect:读取图片数据，并将图片贴到2D纹理上
 parameter:image  图片数据， tex_id  2D纹理ID
 ***************************/
{
//    GLint alignment;
    if(image.isNull())
        return false;
    QImage tex = QGLWidget::convertToGLFormat(image);
    if(tex.isNull())
        return false;
    glBindTexture(GL_TEXTURE_2D,tex_id);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, tex.bits() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);   //配置纹理信息

    return true;
}

QPointF OpenglWidget::getridofblack(float PosX , float PosY, float heightz, float widthz)
/***************************
 Effect:去除图片因放大缩小时产生的黑色部分，主要内容为坐标变换
 parameter:PosX 放大后图片左上角位置X坐标，PosY  放大后图片左上角位置Y坐标
           heightz  放大后图片高度，widthz  放大后图片宽度
 ***************************/
{
    int imagex1 = 0;
    int imagey1 = 0;
    int imagex2 = width;
    int imagey2 = height;
    int X,Y;

    if(PosX > imagex1)
    {
        if(PosY > imagey1)
        Y = imagey1;
        else if(PosY + heightz < imagey2)
        Y = imagey2 - heightz;
        else return QPointF(imagex1,PosY);
        return QPointF(imagex1,Y);
    }
    if(PosX + widthz < imagex2)
    {
        if(PosY > imagey1)
        Y = imagey1;
        else if(PosY + heightz < imagey2)
        Y = imagey2 - heightz;
        else return QPointF(imagex2 - widthz,PosY);
        return QPointF(imagex2 - widthz,Y);
    }
    if(PosY > imagey1)
    {
        if(PosX > imagex1)
        X = imagex1;
        else if(PosX + widthz < imagex2)
        X = imagex2 - widthz;
        else return QPointF(PosX,imagey1);
        return QPointF(X,imagey1);
    }
    if(PosY + heightz < imagey2)
    {
        if(PosX > imagex1)
        X = imagex1;
        else if(PosX + widthz < imagex2)
        X = imagex2 - widthz;
        else return QPointF(PosX,imagey2 - heightz);
        return QPointF(X,imagey2 - heightz);
    }
    return QPointF(PosX,PosY);
}
void OpenglWidget::mousePressEvent(QMouseEvent *event)
/***************************
 Effect:鼠标按压事件，当鼠标按压时触发该函数，主要是记录当前鼠标位置共后续使用
 parameter:
 ***************************/
{
    QPoint invalidpos = event->pos();
    if(invalidpos.x()<0 || invalidpos.y()<0 || invalidpos.x()>width || invalidpos.y()>height)
    {
        return;
    }
    if(event->button() == Qt::RightButton)
    {
        drawlines = true;
        origindrawstartpos = QPointF((event->pos().x() - deviationOriginX)/zoomratio/width,
                                     (event->pos().y() - deviationOriginY)/zoomratio/height);
        drawstartpos = changecoordinate(event->pos());
        return;
    }
    float zuobiaox,zuobiaoy;
    zuobiaox = invalidpos.x() - width/2.0;
    zuobiaoy = -invalidpos.y() + height/2.0;
    if(changebtnflag){
    if(zuobiaox/(width/2.0)<40.0/(width/2.0)-(1-45.0/(width/2.0))+5.0/(width/2.0)&&
       zuobiaox/(width/2.0)>-40.0/(width/2.0)-(1-45.0/(width/2.0))+5.0/(width/2.0)&&
       zuobiaoy/(height/2.0)<40.0/(height/2.0)+10.0/(height/2.0)&&
       zuobiaoy/(height/2.0)>-40.0/(height/2.0)+10.0/(height/2.0))
    {
        emit hasclickedpre();
        return;
    }
    if(zuobiaox/(width/2.0)<40.0/(width/2.0)+(1-45.0/(width/2.0))-5.0/(width/2.0)&&
       zuobiaox/(width/2.0)>-40.0/(width/2.0)+(1-45.0/(width/2.0))-5.0/(width/2.0)&&
       zuobiaoy/(height/2.0)<40.0/(height/2.0)+10.0/(height/2.0)&&
       zuobiaoy/(height/2.0)>-40.0/(height/2.0)+10.0/(height/2.0))
    {
        emit hasclickednext();
        return;
    }}
    if(confirmbtnflag){
    if(zuobiaox/(width/2.0)<50.0f/(width/2.0)-(60.0/(width/2.0))&&
       zuobiaox/(width/2.0)>-50.0f/(width/2.0)-(60.0/(width/2.0))&&
       zuobiaoy/(height/2.0)<20.0f/(height/2.0)-(1-30.0/(height/2.0))&&
       zuobiaoy/(height/2.0)>-20.0f/(height/2.0)-(1-30.0/(height/2.0)))
    {
        emit hasclickedyes();
        return;
    }
    if(zuobiaox/(width/2.0)<50.0f/(width/2.0)+(60.0/(width/2.0))&&
       zuobiaox/(width/2.0)>-50.0f/(width/2.0)+(60.0/(width/2.0))&&
       zuobiaoy/(height/2.0)<20.0f/(height/2.0)-(1-30.0/(height/2.0))&&
       zuobiaoy/(height/2.0)>-20.0f/(height/2.0)-(1-30.0/(height/2.0)))
    {
        emit hasclickedno();
        return;
    }}

     pressflag = true;
     DragstartPos = event->pos();

}

void OpenglWidget::mouseMoveEvent(QMouseEvent *event)
/***************************
 Effect:鼠标移动事件，当按压鼠标左键时实现图片的拖动，当按压鼠标右键时实现人工选择框的绘制
 parameter:
 ***************************/
{
    if(event->buttons() == Qt::RightButton)
    {
        drawendpos = changecoordinate(event->pos());
        updateGL();
        return;
    }
    QPoint invalidpos = event->pos();
    float zuobiaox,zuobiaoy;
    zuobiaox = invalidpos.x() - width/2.0;
    zuobiaoy = -invalidpos.y() + height/2.0;
    if(changebtnflag){
    if(zuobiaox/(width/2.0)<40.0/(width/2.0)-(1-45.0/(width/2.0))+5.0/(width/2.0)&&
       zuobiaox/(width/2.0)>-40.0/(width/2.0)-(1-45.0/(width/2.0))+5.0/(width/2.0)&&
       zuobiaoy/(height/2.0)<40.0/(height/2.0)+10.0/(height/2.0)&&
       zuobiaoy/(height/2.0)>-40.0/(height/2.0)+10.0/(height/2.0))
    {
        prebtnflag = true;
        updateGL();
        return;
    }
    if(zuobiaox/(width/2.0)<40.0/(width/2.0)+(1-45.0/(width/2.0))-5.0/(width/2.0)&&
       zuobiaox/(width/2.0)>-40.0/(width/2.0)+(1-45.0/(width/2.0))-5.0/(width/2.0)&&
       zuobiaoy/(height/2.0)<40.0/(height/2.0)+10.0/(height/2.0)&&
       zuobiaoy/(height/2.0)>-40.0/(height/2.0)+10.0/(height/2.0))
    {
        nextbtnflag = true;
        updateGL();
        return;
    }}
    if(confirmbtnflag){
    if(zuobiaox/(width/2.0)<50.0f/(width/2.0)-(60.0/(width/2.0))&&
       zuobiaox/(width/2.0)>-50.0f/(width/2.0)-(60.0/(width/2.0))&&
       zuobiaoy/(height/2.0)<20.0f/(height/2.0)-(1-30.0/(height/2.0))&&
       zuobiaoy/(height/2.0)>-20.0f/(height/2.0)-(1-30.0/(height/2.0)))
    {
        yesbtnflag = true;
        updateGL();
        return;
    }
    if(zuobiaox/(width/2.0)<50.0f/(width/2.0)+(60.0/(width/2.0))&&
       zuobiaox/(width/2.0)>-50.0f/(width/2.0)+(60.0/(width/2.0))&&
       zuobiaoy/(height/2.0)<20.0f/(height/2.0)-(1-30.0/(height/2.0))&&
       zuobiaoy/(height/2.0)>-20.0f/(height/2.0)-(1-30.0/(height/2.0)))
    {
        nobtnflag = true;
        updateGL();
        return;
    }}
    updateGL();
    if(!pressflag) return;

    if(invalidpos.x()<0 || invalidpos.y()<0 || invalidpos.x()>width || invalidpos.y()>height)
    {
        MoveFlag = 0;
        return;
    }
    if ((event->pos() - DragstartPos).manhattanLength() < QApplication::startDragDistance())
    {MoveFlag = 0;return;}
    if(zoomratio <= 1) {MoveFlag = 0;return;}
    if(event->buttons() == Qt::LeftButton){
    int imagex1 = 0;
    int imagey1 = 0;
    int imagex2 = width;
    int imagey2 = height;
    QPointF offset;
    offset =  event->pos() - DragstartPos;
    if(DragLastPosX+offset.x() > imagex1)
    {
       if(DragLastPosY+offset.y() > imagey1)
           DragLastPosY = imagey1;
       else if(DragLastPosY+offset.y() + height* zoomratio < imagey2)
           DragLastPosY = imagey2 - height* zoomratio;
       else
       {
           changezuobiao(imagex1,DragLastPosY+offset.y());
           updateGL();
           MoveFlag = 1;
           return;
       }
       DragLastPosX = imagex1;
       MoveFlag = 0;
       deviationOriginX = DragLastPosX;
       deviationOriginY = DragLastPosY;
       return;
    }
    if(DragLastPosX+offset.x()+width* zoomratio < imagex2)
    {
       if(DragLastPosY+offset.y() > imagey1)
           DragLastPosY = imagey1;
       else if(DragLastPosY+offset.y() + height* zoomratio < imagey2)
           DragLastPosY = imagey2 - height* zoomratio;
       else
       {
           changezuobiao(imagex2-width* zoomratio,DragLastPosY+offset.y());
           updateGL();
           MoveFlag = 2;
           return;
       }
       DragLastPosX = imagex2-width* zoomratio;
       MoveFlag = 0;
       deviationOriginX = DragLastPosX;
       deviationOriginY = DragLastPosY;
       return;
    }
    if(DragLastPosY+offset.y() > imagey1)
    {
        if(DragLastPosX+offset.x() > imagex1)
          DragLastPosX = imagex1;
        else if(DragLastPosX+offset.x() + width* zoomratio <imagex2)
          DragLastPosX = imagex2 - width* zoomratio;
        else
        {
           changezuobiao(DragLastPosX+offset.x(),imagey1);
           updateGL();
           MoveFlag = 3;
           return;
        }
        DragLastPosY = imagey1;
        MoveFlag = 0;
        deviationOriginX = DragLastPosX;
        deviationOriginY = DragLastPosY;
        return;
    }
    if(DragLastPosY+offset.y() + height* zoomratio < imagey2)
    {
        if(DragLastPosX+offset.x() > imagex1)
          DragLastPosX = imagex1;
        else if(DragLastPosX+offset.x() + width* zoomratio < imagex2)
          DragLastPosX = imagex2 - width* zoomratio;
        else
        {
           changezuobiao(DragLastPosX+offset.x(),imagey2 - height* zoomratio);
           updateGL();
           MoveFlag = 4;
           return;
        }
        DragLastPosY = imagey2 - height* zoomratio;
        MoveFlag = 0;
        deviationOriginX = DragLastPosX;
        deviationOriginY = DragLastPosY;
        return;
    }
    changezuobiao(DragLastPosX+offset.x(),DragLastPosY+offset.y());
    updateGL();
    MoveFlag = 5;
    return;}
}

void OpenglWidget::mouseReleaseEvent(QMouseEvent *event)
/***************************
 Effect:鼠标释放事件，记录当前鼠标释放位置，供下次拖动使用
 parameter:
 ***************************/
{
    if(subpicclicked != -1)
    {
        emit hasclickedsubpic(subpicclicked);
    }
    if(event->button() == Qt::LeftButton){
    pressflag = false;
    QPoint invalidpos = event->pos();
    if(invalidpos.x()<0 || invalidpos.y()<0 || invalidpos.x()>width || invalidpos.y()>height)
    {
        MoveFlag = 0;
        return;
    }
    if(MoveFlag == 0) return;
    int imagex1 = 0;
    int imagey1 = 0;
    int imagex2 = width;
    int imagey2 = height;
    QPointF offset;
    offset = event->pos() - DragstartPos ;
    if(MoveFlag == 1){
        DragLastPosX = imagex1;
        DragLastPosY = DragLastPosY+offset.y();}

    if(MoveFlag == 2){
        DragLastPosX = imagex2 - width* zoomratio;
        DragLastPosY = DragLastPosY+offset.y();}

    if(MoveFlag == 3){
        DragLastPosX = DragLastPosX+offset.x();
        DragLastPosY = imagey1;}

    if(MoveFlag == 4){
        DragLastPosX = DragLastPosX+offset.x();
        DragLastPosY = imagey2 - height* zoomratio;}

    if(MoveFlag == 5){
        DragLastPosX = DragLastPosX+offset.x();
        DragLastPosY = DragLastPosY+offset.y();}

    deviationOriginX = DragLastPosX;
    deviationOriginY = DragLastPosY;}
    else if(event->button() == Qt::RightButton)
    {
        origindrawendpos = QPointF((event->pos().x() - deviationOriginX)/zoomratio/width,
                                   (event->pos().y() - deviationOriginY)/zoomratio/height);

        emit squraepos(origindrawstartpos,origindrawendpos);
    }
}


void OpenglWidget::changezuobiao(float X,float Y)
/***************************
 Effect:坐标变换函数，将Qt坐标系变换为opengl坐标系
 parameter:X  变换坐标X， Y变换坐标Y
 ***************************/
{
    counterpoint[0].setX(X);
    counterpoint[0].setY(Y);
    counterpoint[1].setX(counterpoint[0].x()+width* zoomratio);
    counterpoint[1].setY(counterpoint[0].y());
    counterpoint[2].setX(counterpoint[1].x());
    counterpoint[2].setY(counterpoint[1].y()+height* zoomratio);
    counterpoint[3].setX(counterpoint[2].x()-width* zoomratio);
    counterpoint[3].setY(counterpoint[2].y());
    QPointF topleft = QPoint(width/2.0,height/2.0);
    for(int i = 0 ; i < 4 ; i++){
    counterpoint[i].setX(counterpoint[i].x() -topleft.x());
    counterpoint[i].setY(-counterpoint[i].y() +topleft.y());
    counterpoint[i].setX(counterpoint[i].x()/(width/2.0));
    counterpoint[i].setY(counterpoint[i].y()/(height/2.0));
    }//坐标系变换，原点变换至屏幕中央
}

void OpenglWidget::setpictowidget(QImage image)
/***************************
 Effect:读入图片，并实现图片在窗口中的绘制
 parameter:
 ***************************/
{
    initializeGL();
    if(!readImage(image , texture))
    {  blackflag = true;
        return;}
    blackflag = false;
    grayflag = false;
    zoomratio = 1.0;
    lastzoomratio = 1.0;
    deviationOriginX = 0;
    deviationOriginY = 0;
    imageStartX = 0;
    imageStartY = 0;
    DragstartPos = QPointF(0.0f,0.0f);
    counterpoint[0]=QPointF(-1.0f,1.0f);
    counterpoint[1]=QPointF(1.0f,1.0f);
    counterpoint[2]=QPointF(1.0f,-1.0f);
    counterpoint[3]=QPointF(-1.0f,-1.0f);
    DragLastPosX = 0;
    DragLastPosY = 0;
    MoveFlag = 0;
    updateGL();
}

/***************************
 Effect:坐标转换，将Qt坐标系转换为opengl坐标系，主要用于绘制人工选择框
 parameter:
 ***************************/
QPointF OpenglWidget::changecoordinate(QPointF point)
{
    QPointF res;
    QPointF topleft = QPoint(width/2.0,height/2.0);
    res.setX((point.x() -topleft.x())/(width/2.0));
    res.setY((-point.y() +topleft.y())/(height/2.0));
    return res;
}

void OpenglWidget::zoomback()
/***************************
 Effect:将opengl内的窗口图片还原成初始状态
 parameter:
 ***************************/
{
    zoomratio = 1.0;
    lastzoomratio = 1.0;
    deviationOriginX = 0;
    deviationOriginY = 0;
    imageStartX = 0;
    imageStartY = 0;
    DragstartPos = QPointF(0.0f,0.0f);
    counterpoint[0]=QPointF(-1.0f,1.0f);
    counterpoint[1]=QPointF(1.0f,1.0f);
    counterpoint[2]=QPointF(1.0f,-1.0f);
    counterpoint[3]=QPointF(-1.0f,-1.0f);
    DragLastPosX = 0;
    DragLastPosY = 0;
    MoveFlag = 0;
    updateGL();
}

void OpenglWidget::setblackback()
/***************************
 Effect:设置opengl窗口全黑
 parameter:
 ***************************/
{
    blackflag = true;
    updateGL();
}

void OpenglWidget::setgrayback()
/***************************
 Effect:设置opengl窗口全灰
 parameter:
 ***************************/
{
    grayflag = true;
    updateGL();
}
