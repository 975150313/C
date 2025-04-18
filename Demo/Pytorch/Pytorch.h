#ifndef PYTORCH_H
#define PYTORCH_H

#include <atomic>
#include <memory>
#include <string>
#include "Plugins/Segment/DeepLearning/thpndicomreader.h"
#include "Plugins/Segment/DeepLearning/DeepLearning.h"
#include "Windows.h"
namespace torch {
namespace jit {
struct Module;
} // namespace jit
} // namespace torch
class Pytorch: public DeepLearning
{
public:
    Pytorch();
    ~Pytorch();

    int LoadModel(const char* path) override;
    bool build() override;
    void SetStop(bool) override;
    bool infer(thpnDICOMReader* vtk) override;

    bool TestModl();
    void closeSession();
    void Clean();
    bool hasPrefix(const std::string& str, const std::string& prefix);
private:
    std::shared_ptr<torch::jit::Module> m_session;
    bool isStop = false;
    bool isRuning = false;
    bool isExit = false;
    std::string modelPath;
//    typedef void* HMODULE;
    HMODULE h1;
    HMODULE h2;
    HMODULE h3;
    HMODULE h4;
};

#endif // PYTORCH_H
