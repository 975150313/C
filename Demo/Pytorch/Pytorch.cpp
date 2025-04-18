#include "Pytorch.h"
#define  COMPILER_MSVC 1
#include <Eigen/Core>
#include <Eigen/Dense>
#include <cuda_runtime_api.h>

#include <torch/torch.h>
#include <torch/script.h>
#include <c10/cuda/CUDAStream.h>
#include <ATen/cuda/CUDAEvent.h>

#include <string>
#include <opencv2/core/eigen.hpp>
#include <opencv2/opencv.hpp>

#include <QMap>
#include <QCoreApplication>
#include <QException>
#include <QDateTime>
#include <QDebug>
#include <time.h>
#include "Helpers/thpn.h"
#include "Helpers/FileHelper.h"
#include "Helpers/JsonHelper.h"

Pytorch::Pytorch()
{
    using namespace std;
    modelPath = QString("%1/%2")
            .arg(QCoreApplication::applicationDirPath())
            .arg(thpn::PATH_MODEL).toStdString();

    h1 = LoadLibraryA("ATen_cuda.dll");
    h2 = LoadLibraryA("c10_cuda.dll");
    h3 = LoadLibraryA("torch_cuda.dll");
    h4 = LoadLibraryA("torchvision.dll");

}

Pytorch::~Pytorch()
{
    qDebug()<<"======closeSession=======";
    closeSession();
    FreeLibrary(h1);
    FreeLibrary(h2);
    FreeLibrary(h3);
    FreeLibrary(h4);
}

void Pytorch::closeSession()
{

}

bool Pytorch::TestModl()
{
    qDebug()<<"Test SUCCESS";
    cudaDeviceReset();
    return true;
}


int Pytorch::LoadModel(const char* path)
{
    torch::Device deviceGPU(torch::kCUDA);
    m_session = std::make_shared<torch::jit::Module>(torch::jit::load(path));
    m_session->to(deviceGPU);
    return 0;
}

void Pytorch::Clean()
{
    this->isStop = true;
}
bool Pytorch::hasPrefix(const std::string& str, const std::string& prefix)
{
    return str.compare(0, prefix.length(), prefix) == 0;
}
bool Pytorch::infer(thpnDICOMReader* vtk)
{
    auto dims = vtk->GetDimensions();
    auto imageData = vtk->GetImageDataPointer();
    auto maskDatas = vtk->GetMaskData();
    auto numClass = vtk->GetNumClass();
    auto threshold = vtk->getThreshold();
    std::string input  = vtk->GetInput();
    std::string output = vtk->GetOutput();
    std::string pbname = vtk->GetModelName();
    int dropout = vtk->GetDropOut();
    auto imageSize = dims[0] * dims[1];
    this->isStop = false;
    clock_t t1 =clock();
    torch::Device deviceGPU(torch::kCUDA);
    torch::Device deviceCPU(torch::kCPU);

    c10::cuda::CUDACachingAllocator::emptyCache();

    int W = dims[0];
    int H = dims[1];
    std::cout<<"start" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for(int slice_number = 0; slice_number < dims[2] && false == isStop && false==isExit;slice_number++)
    {
        //网络输入参数1
        float* pdata = imageData + (dims[0] * dims[1] * slice_number);

        //Skin  Bronchial  TotalPulmonaryVessels
//        if(hasPrefix(pbname,"Heart")|| hasPrefix(pbname,"Bone") ||hasPrefix(pbname,"PulmonaryVessels"))
        if(hasPrefix(pbname,"Skin")|| hasPrefix(pbname,"Bronchial") ||hasPrefix(pbname,"TotalPulmonaryVessels"))
        {

        }else {
            for(int i=0;i<W*H;++i)
            {
                float val = pdata[i];
                if(val>255.0)
                {
                    val=255.0;
                }
                if(val<0.0)
                {
                    val=0.0;
                }
                pdata[i] =val;
            }
        }
#define SHOW 0
#if SHOW
        cv::Mat src(dims[0],dims[1], CV_32FC1, pdata);
        //        float maxValue = *std::max_element(src.begin<float>(), src.end<float>());
        //        float minValue = *std::min_element(src.begin<float>(), src.end<float>());
        //        qDebug() << "maxValue " << maxValue<<minValue;
        cv::Mat copy ;
        src.convertTo(copy,CV_8UC1);
        cv::imshow("src",src);
        cv::imshow("copy",copy);
        cv::waitKey(40);
#endif
        try
        {
            torch::Tensor input_tensor = torch::from_blob(pdata, {1, 1, dims[0], dims[1]}, torch::kFloat).to(deviceGPU, torch::kFloat32);
            torch::Tensor output_tensor = m_session->forward({ input_tensor }).toTensor().sigmoid().round().to(deviceCPU, torch::kFloat32);;
            //            std::vector<float> v(output_tensor.data_ptr<float>(),output_tensor.data_ptr<float>()+ output_tensor.numel());//将tensor转换为vector
            for(int n_class = 0; n_class<numClass; ++n_class)
            {
                auto maskdata = maskDatas[n_class];//获取第 i 个类
                float* pMaskdata = static_cast<float*>(maskdata->GetScalarPointer());
                float* buf = pMaskdata + H * W * slice_number;//mask 当前位置
                int offset = 1 * n_class * H * W;//tensor 偏移量
                memcpy(buf, output_tensor.data_ptr<float>() + offset, H * W * sizeof(float));
#if SHOW
                cv::Mat src1(512, 512, CV_32FC1, buf);
                cv::imshow("dst1", src1);
                cv::waitKey(1);
#endif
            }

        }  catch (const std::exception& e)
        {
            std::cerr << "Error "<<e.what();
            return -1;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto const total = std::chrono::duration<float, std::milli>(end - start).count();
    std::cout<<"total time is "<< total << " ms" << std::endl;

    clock_t t2 = clock() - t1;
    qDebug() << "times: " << t2/1000<<" s";
    //    m_session->Close();
    //    cudaDeviceReset();
    c10::cuda::CUDACachingAllocator::emptyCache();
    return EXIT_SUCCESS;
}


bool Pytorch::build()
{
    return 1;
}

void Pytorch::SetStop(bool)
{

}
