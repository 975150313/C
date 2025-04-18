#include "MainWidget.h"
#include <QMouseEvent>
#include <math.h>
#include "ServiceSpaceMouse.h"
#include "GeometryEngine.h"
#include "BaseSingleton.h"

MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    texture(nullptr)
{
    qRegisterMetaType<SpaceMouseStatus>("SpaceMouseStatus");
    space_mouse = std::make_shared<ServiceSpaceMouse>();
    space_mouse->Start();
    connect(Singleton(GlobSignal), &GlobSignal::trage_event,this, &MainWidget::trage_event);
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete texture;
    doneCurrent();
}

void MainWidget::trage_event(SpaceMouseStatus status)
{
    if(0x01 == status.type)
    {
        double t_scale = 35.0;
        double r_scale = 3.50;
        short vec[6] = {0};
        short max_index = 0;
        short max_val = 0;
        for(size_t i=0; i<6;++i)
        {
            int val = abs(status.arr[i]);
            if(val> max_val)
            {
                max_index = i;
                max_val = val;
            }
        }
        vec[max_index] = 1;
        trans = QVector3D(status.arr[0]/t_scale*vec[0], status.arr[1]/t_scale*vec[1],status.arr[2]/t_scale*vec[2]);
        rotationAxis = QVector3D((status.arr[3]/r_scale*vec[3]), status.arr[4]/r_scale*vec[4], status.arr[5]/r_scale*vec[5]).normalized();
        qDebug()<<"原始数据: "<<status.arr[0]<<" "<<status.arr[1]<<" "<<status.arr[2]<<" "<<status.arr[3]<<" "<<status.arr[4]<<" "<<status.arr[5];
        qDebug()<<"转换数据: "<<trans[0]<<" "<<trans[1]<<" "<<trans[2]<<" "<<rotationAxis[0]<<" "<<rotationAxis[1]<<" "<<rotationAxis[2];
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;
    }
    else if (0x03 == status.type) {
        int btn = status.btn;
         qDebug()<<"btn "<<btn;
    }
    update();
}

void MainWidget::timerEvent(QTimerEvent *)
{

}

void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    geometries = new GeometryEngine;

    // Use QBasicTimer because its faster than QTimer
//    timer.start(50, this);
}

void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}

void MainWidget::initTextures()
{
    // Load cube.png image
    texture = new QOpenGLTexture(QImage(":/cube.png").mirrored());

    // Set nearest filtering mode for texture minification
    texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture->setWrapMode(QOpenGLTexture::Repeat);
}

void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 3.0, zFar = 7.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}

void MainWidget::paintGL()
{
    // Clear color and depth buffer
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //    texture->bind();

    //    QMatrix4x4 matrix;
    //    matrix.translate(0.0, 0.0, -5.0);
    //    matrix.rotate(rotation);

    //    // Set modelview-projection matrix
    //    program.setUniformValue("mvp_matrix", projection * matrix);

    //    // Use texture unit 0 which contains cube.png
    //    program.setUniformValue("texture", 0);

    //    // Draw cube geometry
    //    geometries->drawCubeGeometry(&program);



    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texture->bind();

    // Calculate model view transformation

    QVector3D pos(0.0f, 0.0f, -10.0f); // 相机位置
    QVector3D target(0.0f, 0.0f, 10.0f); // 相机目标点
    QVector3D up(0.0f,   -1.0f,  0.0f); // 上方向
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(pos, target, up);
    viewMatrix.translate(trans);
    viewMatrix.rotate(rotation);
    // 使用视图矩阵
    QMatrix4x4 modelMatrix; // 模型矩阵，根据需要设置
    modelMatrix.setToIdentity();

    QMatrix4x4 projectionMatrix; // 投影矩阵，根据需要设置，例如透视投影或正交投影
    projectionMatrix.perspective(50.0f, width() / static_cast<float>(height()), 0.1f, 100.0f); // 示例透视投影设置

    QMatrix4x4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix; // 计算MVP矩阵

    program.setUniformValue("mvp_matrix", mvpMatrix);// Set modelview-projection matrix
    program.setUniformValue("texture", 0);// Use texture unit 0 which contains cube.png
    geometries->drawCubeGeometry(&program);// Draw cube geometry
}
