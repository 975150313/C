#include <QCoreApplication>

#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::vector<cv::Point2f> points;
    for(int x=0; x<=20;x+=2)
    {
        float y = 0.5*pow(x,2)+2*x+10;
        points.emplace_back(x*10,y*10);
    }
    cv::Mat A(points.size(),3,CV_32F);
    cv::Mat b(points.size(),1,CV_32F);
    for(size_t i=0;i<points.size();++i)
    {
        float x =points[i].x;
        A.at<float>(i,0) = x*x;
        A.at<float>(i,1) = x;
        A.at<float>(i,2) = 1;
        b.at<float>(i) = points[i].y;
    }
    cv::Mat coeff;
    cv::solve(A,b,coeff,cv::DECOMP_NORMAL| cv::DECOMP_SVD);
    std::vector<cv::Point> curvePoints;
    for(int x=0;x<200;x+=2)
    {
        float y = coeff.at<float>(0)*x*x+ coeff.at<float>(1)*x+coeff.at<float>(2);
        curvePoints.emplace_back(x, cvRound(y));
    }
    cv::Mat img = cv::Mat::zeros(800,800,CV_8UC3);
    for(const auto& p:points)
    {
        cv::circle(img, p,false,cv::Scalar(0,255,0),cv::FILLED);
    }
    cv::polylines(img,curvePoints,false,cv::Scalar(0,0,255),2);
    cv::imshow("111",img);
    cv::waitKey(0);
    return 0;
}


