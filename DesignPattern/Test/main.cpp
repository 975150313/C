#include <QCoreApplication>
#include "bfactory.h"
#include <QDebug>
//#include "vld.h"
void test1()
{
//    IBClass* p = BFactory::Instance().produce("BClass");
//    p->fun();
   std::shared_ptr<IBClass>  p1 = BFactory::Instance().produce_shared("BClass");
    p1.get()->fun();


}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    test1();
    return a.exec();
}
