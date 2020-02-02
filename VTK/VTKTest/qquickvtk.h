#ifndef QQUICKVTK_H
#define QQUICKVTK_H

#include <QQueue>
#include <QQuickFramebufferObject>
#include "QQuickVtkRenderer.h"
class QQuickVtk : public QQuickFramebufferObject
{
    Q_OBJECT
public:
    QQuickVtk(QQuickItem* parent = nullptr);

    Renderer *createRenderer() const override;
    static QSurfaceFormat defaultFormat();

signals:
    void qevent(QEvent* event) ;
public slots:
protected:
    bool event(QEvent *e) override;
private:
};

#endif // QQUICKVTK_H
