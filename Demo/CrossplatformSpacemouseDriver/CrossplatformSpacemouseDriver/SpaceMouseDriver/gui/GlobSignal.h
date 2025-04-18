#pragma once

#include <QObject>
struct SpaceMouseStatus
{
    short type   = 0;
    short btn    = 0;
    short arr[6] = {0};
};
class GlobSignal:public QObject
{
    Q_OBJECT
public:
signals:
    void trage_event(SpaceMouseStatus status);
};
