#ifndef BCLASS_H
#define BCLASS_H
#include "bfactory.h"
class BClass : public IBClass
{
public:
    BClass();
    int fun();
private:
    int m;
};
REGISTER_Class(BClass, "BClass");
#endif // BCLASS_H
