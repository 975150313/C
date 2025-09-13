#include <iostream>
#include <unsupported/Eigen/CXX11/Tensor>

int main()
{
    Eigen::Tensor<double, 2> A(4, 4);

    // 填一些 0~1 的随机值做演示
    A.setRandom();
     std::cout << A.constant(1.0) <<std::endl;

    A = (A + A.constant(1.0)) * A.constant(0.5);

    std::cout << "Start A:\n" << A << "\n\n";

    // 1. 构造 mask：大于 0.5 的位置为 true
    Eigen::Tensor<bool, 2> mask = (A > A.constant(0.8));

    // 2. select(mask, on_true, on_false)
    Eigen::Tensor<double, 2> one = A.constant(1.0);
    A = mask.select(one, A);

    std::cout << "End A:\n" << A << "\n";
    return 0;
}
