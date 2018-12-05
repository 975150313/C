#include "bclass.h"
#include <QDebug>
//#include "vld.h"
BClass::BClass():m(10)
{

}
int BClass::fun()
{
    qDebug()<<"BClass::fun"<<m;
    return m;
}
