#ifndef IBCLASS_H
#define IBCLASS_H
class IBClass                       //抽象基类，对用户可见
{
public:
    IBClass();
    virtual ~IBClass();
    virtual int fun() = 0;      //需要公开的接口，一定要纯虚函数?
};
#endif // IBCLASS_H
