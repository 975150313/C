/**
这个文件下有三个demo 
1. 使用fcl库 和圆柱模型对机械臂关节进行碰撞检测 不推荐
2. 使用 assimp 库加载stl模型 使用 fcl库进行碰撞检测
3. 使用 vtk 加载stl模型  使用fcl库进行碰撞检测
碰撞检测的模型是 ur5  和 ur5e
**/
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <thread>
#include "fcl/fcl.h"
#define M_PI       3.14159265358979323846
struct fcl_cylinder
{
    double rad;//  #圆柱半径
    double lz;//  #z轴向高度
    fcl::Vector3f T;// #平移向量,表示位置 3d向量
    fcl::Matrix3f R;//#旋转,矩阵3*3
    fcl::Quaternionf q;//#旋转,四元数
};
struct CollisionInfo
{
    int joint1;
    int joint2;
    double distance;
};
enum RobotType
{
    UR5 = 0,
    UR5e = 1
};
struct RobotParams
{
    int type= RobotType::UR5e;
    double pi=3.14159265358979323846;
    double d[6] ={0};
    double a[6] ={0};
    double alpha[6]={0};
    double d1=0;
    double r1=0;
    double r2=0;
    double r3=0;
    double r4=0;
    double r5=0;
    double r6=0;
    double r7=0;
    double r8=0;
    double r9=0;
    double d2=0;
    double d3=0;
    double d6=0;
    double d7=0;
    double a2=0;
    double a3=0;
    double e3=0;
};

double d2r(double degree) { return (degree / 180.0) * M_PI; }

void set_model(RobotParams* p, int m)
{
    if(p!= nullptr&& m== RobotType::UR5e )
    {
        p->type = RobotType::UR5e ;
        p->d1=162.5;
        p->r1=58;
        p->r2=42.5;
        p->r3=37.5;
        p->r4=37.5;
        p->r5=37.5;
        p->r6=37.5;
        p->r7=58;
        p->r8=37.5;
        p->r9=58;
        p->d2=137.8;
        p->d3=131.2;
        p->d6=51.8;
        p->d7=45.75;
        p->a2=-425;
        p->a3=-392.2;
        p->e3=57;
        p->pi = M_PI;

        double ur5e_d[6] ={162.5,0,0,133.33,99.7,99.6};
        double ur5e_a[6] ={0,-425,-392.2,0,0,0};
        double ur5e_alpha[6]={p->pi/2.0,0,0,p->pi/2.0,-p->pi/2.0,0};
        memcpy(p->d, &ur5e_d[0],sizeof(double)*6);
        memcpy(p->a, &ur5e_a[0],sizeof(double)*6);
        memcpy(p->alpha, &ur5e_alpha[0],sizeof(double)*6);
    }
    else if(p!= nullptr&& m== RobotType::UR5 )
    {
        p->type = RobotType::UR5 ;
        p->d1=89.159;
        p->r1=58;
        p->r2=42.5;
        p->r3=37.5;
        p->r4=37.5;
        p->r5=37.5;
        p->r7=58;
        p->r8=37.5;
        p->r9=58;
        p->d2=135.7;
        p->d3=119.7;
        p->d6=45.7;
        p->d7=34.5;
        p->a2=-425;
        p->a3=-392.25;
        p->e3=60;
        p->pi = M_PI;
        double ur5_d[] = {89.159,0,0,109.15,94.65,82.3};
        double ur5_a[] = {0,-425,-392.25,0,0,0};
        double ur5_alpha[6]={p->pi/2.0,0,0,p->pi/2.0,-p->pi/2.0,0};
        memcpy(p->d, &ur5_d[0],sizeof(double)*6);
        memcpy(p->a, &ur5_a[0],sizeof(double)*6);
        memcpy(p->alpha, &ur5_alpha[0],sizeof(double)*6);
    }
}
Eigen::Matrix4f fk(std::vector<double>& theta,int k,RobotParams* p)
{
    //    #UR5e DH参数
    auto d = p->d;
    auto a = p->a;
    auto alpha = p->alpha;
    Eigen::Matrix4f sum_mat = Eigen::Matrix4f::Identity();
    for (int i = 0; i < k; ++i)
    {
        Eigen::Matrix4f mat = Eigen::Matrix4f::Identity();
        mat(0, 0) = cos(theta[i]);
        mat(0, 1) = -sin(theta[i]) * cos(alpha[i]);
        mat(0, 2) =  sin(theta[i]) * sin(alpha[i]);
        mat(0, 3) = a[i] * cos(theta[i]);
        mat(1, 0) = sin(theta[i]);
        mat(1, 1) =  cos(theta[i]) * cos(alpha[i]);
        mat(1, 2) = -cos(theta[i]) * sin(alpha[i]);
        mat(1, 3) = a[i] * sin(theta[i]);
        mat(2, 0) = 0;
        mat(2, 1) = sin(alpha[i]);
        mat(2, 2) = cos(alpha[i]);
        mat(2, 3) = d[i];
        mat(3, 0) = 0;
        mat(3, 1) = 0;
        mat(3, 2) = 0;
        mat(3, 3) = 1;
        sum_mat = sum_mat * mat;//点乘
    }
    return sum_mat;
}

double collision(fcl_cylinder c1, fcl_cylinder c2)
{
    double rad1 = c1.rad; // 圆柱1的半径
    double lz1 = c1.lz;  // 圆柱1的高度
    double rad2 = c2.rad; // 圆柱2的半径
    double lz2 = c2.lz;  // 圆柱2的高度

    // 定义两个位姿（旋转和平移矩阵）
    fcl::Matrix3f R1 = c1.R; // 单位矩阵,表示没有旋转
    fcl::Vector3f T1 = c1.T; // 圆柱1的位置
    fcl::Matrix3f R2 = c2.R; // 单位矩阵,表示没有旋转
    fcl::Vector3f T2 = c2.T;  // 圆柱2的位置

    // 通过形状和位姿生成碰撞对象
    auto cylinder1 = std::make_shared<fcl::Cylinderf>(rad1, lz1); // 圆柱1
    auto cylinder2 = std::make_shared<fcl::Cylinderf>(rad2, lz2); // 圆柱2

    fcl::CollisionObjectf cylinder1_obj(cylinder1, R1, T1);
    fcl::CollisionObjectf cylinder2_obj(cylinder2, R2, T2); // 圆柱2的碰撞对象

    // 距离检测
    fcl::DistanceRequestf distance_request;
    fcl::DistanceResultf distance_result;
    return fcl::distance(&cylinder1_obj, &cylinder2_obj, distance_request, distance_result);
}

void model_cylinder(std::vector<double>& tmp_theta, RobotParams* params,std::vector<fcl_cylinder> &modelCylinder)
{
    double pi = M_PI;
    std::vector<double> theta;
    for(int i=0;i<tmp_theta.size();++i)
    {
        theta.emplace_back( d2r(tmp_theta[i]) );//弧度不需要这个
    }
    auto r1 = params->r1;
    auto r2 = params->r2;
    auto r3 = params->r3;
    auto r4 = params->r4;
    auto r5 = params->r5;
    auto r6 = params->r6;
    auto r7 = params->r7;
    auto r8 = params->r8;
    auto r9 = params->r9;

    auto d1 = params->d1;
    auto d2 = params->d2;
    auto d3 = params->d3;
    auto d6 = params->d6;
    auto d7 = params->d7;

    auto a2 = params->a2;
    auto a3 = params->a3;
    auto e3 = params->e3;
    //    std::vector<fcl::Vector3f> T;
    //    std::vector<fcl::Matrix3f> R;
    double rad[]={r1,r2,r3,r4,r5,r6};
    double lz[]={d1+r9,-a2,-a3,d6+e3,d6+e3,d7};

    std::vector<Eigen::Matrix4f> T0i;
    std::vector<Eigen::Matrix4f> T0i_rotation;
    std::vector<Eigen::Matrix4f> T0i_translation;

    {
        Eigen::Matrix4f rota1;
        rota1<<1,0,0,0,
                0,cos(-pi/2.0),-sin(-pi/2.0),0,
                0,sin(-pi/2.0),cos(-pi/2.0),0,
                0,0,0,1;////##旋转成z轴和模型圆柱体轴线重合 绕x旋转-90度
        T0i_rotation.emplace_back(rota1);
        Eigen::Matrix4f trans1;
        trans1<<1,0,0,0,
                0,1,0,0,
                0,0,1,lz[0]/2.0-d1,
                0,0,0,1;
        T0i_translation.emplace_back(trans1);

        Eigen::Matrix4f rota2;
        rota2<<cos(-pi/2.0),0,sin(-pi/2.0),0,
                0, 1,0,0,
                -sin(-pi/2.0),0,cos(-pi/2.0),0,
                0,0,0,1; //##旋转成z轴和模型圆柱体轴线重合 绕y旋转-90度
        T0i_rotation.emplace_back(rota2);
        Eigen::Matrix4f trans2;
        trans2<<1, 0, 0,  d2,
                0, 1, 0, 0,
                0, 0, 1, -lz[1]/2.0,
                0, 0, 0, 1;
        T0i_translation.emplace_back(trans2);

        Eigen::Matrix4f rota3;
        rota3<<cos(-pi/2.0),0,sin(-pi/2.0),0,
                0, 1,0,0,
                -sin(-pi/2.0),0,cos(-pi/2.0),0,
                0,0,0,1;//##旋转成z轴和模型圆柱体轴线重合 绕y旋转-90度

        T0i_rotation.emplace_back(rota3);

        Eigen::Matrix4f trans3;
        trans3<<1, 0, 0, d2-d3,
                0, 1, 0, 0,
                0, 0, 1, -lz[2]/2.0,
                0, 0,0,1;
        T0i_translation.emplace_back(trans3);

        Eigen::Matrix4f rota4=Eigen::Matrix4f::Identity();
        T0i_rotation.emplace_back(rota4);
        Eigen::Matrix4f trans4;
        trans4<<1,0,0,0,
                0, 1,0,0,
                0,0,1,lz[3]/2.0-e3,
                0,0,0,1;
        T0i_translation.emplace_back(trans4);

        Eigen::Matrix4f rota5=Eigen::Matrix4f::Identity();
        T0i_rotation.emplace_back(rota5);
        Eigen::Matrix4f trans5;
        trans5<<1,0,0,0,
                0,1,0,0,
                0,0,1,lz[4]/2.0-e3,
                0,0,0,1;
        T0i_translation.emplace_back(trans5);

        Eigen::Matrix4f rota6=Eigen::Matrix4f::Identity();
        T0i_rotation.emplace_back(rota6);
        Eigen::Matrix4f trans6;
        trans6<<1,0,0,0,
                0,1,0,0,
                0,0,1,-lz[5]/2.0,
                0,0,0,1;
        T0i_translation.emplace_back(trans6);
    }

    for(int i=0; i<6;++i)
    {
        Eigen::Matrix4f T_fk = fk(theta,i+1, params);
        T0i.emplace_back(T_fk);
        Eigen::Matrix4f T0i_cyl=T0i[i]*T0i_rotation[i]*T0i_translation[i];
        //        T.emplace_back(T0i_cyl.block<3,1>(0, 3));//这里不确定是不是这样 T[i]=T0i_cyl[0:3,3:4]
        //        R.emplace_back(T0i_cyl.block<3,3>(0, 0));//这里不确定是不是这样 R[i]=T0i_cyl[0:3,0:3]

        fcl_cylinder c;
        c.rad = rad[i];
        c.lz = lz[i];
        c.T = T0i_cyl.block<3,1>(0, 3);
        c.R = T0i_cyl.block<3,3>(0, 0);
        modelCylinder.emplace_back(c);
    }
}

void test(std::vector<double>& theta, RobotParams* params, std::vector<CollisionInfo>& c_info)
{
    std::vector<fcl_cylinder> cs;
    model_cylinder(theta, params, cs);
    c_info.clear();
    for(int i=0; i<3;++i)
    {
        for(int j=i+2; j<6;++j)
        {
            double distance = collision(cs[i], cs[j]);
            CollisionInfo info;
            info.joint1 = i+1;
            info.joint2 = j+1;
            info.distance = distance;
            c_info.emplace_back(info);
        }
    }
}
std::vector<std::vector<double>> list1={
    {-60.378,-108.252,-90.935,-62.798,95.596,-2.039},
    {-58.665,-118.968,-86.106,-57.484,93.113,0.923},
    {-54.340,-117.945,-87.782,-57.173,89.118,-14.712},
    {-57.056,-107.559,-91.679,-63.756,93.663,-20.266},
    {-69.755,-91.930,-132.619,-4.310,109.765,-12.919},
    {-94.076,-116.247,-107.371,-58.324,154.831,-108.509},
    {-75.267,-139.524,-42.705,-125.881,116.778,-175.560},
    {-62.568,-109.075,-89.210,-64.251,96.085,13.560 },
    {-61.758,-118.957,-86.036,-56.911,95.422,14.498 },
    {-37.723,-107.953,-94.355,-69.716,72.655,-44.986},
    {-36.852,-117.137,-95.150,-57.172,68.391,-40.444},
    {-40.395,-95.387,-110.308,-53.335,79.226,-65.733},
    {-29.207,-131.820,-78.783,-78.611,56.971,-28.472},
    {-59.459,-89.614,-113.673,-52.413,105.625,-107.028},
    {-73.274,-102.776,-98.125,-71.420,120.296,-133.050},
    {-63.988,-97.076,-109.386,-47.329,108.055,-71.284},
    {-43.835,-142.542,-48.840,-98.094,74.978,-17.061},
    {-48.150,-109.250,-89.210,-70.240,84.097,-36.808},
    {-46.260,-117.693,-89.553,-59.200,79.703,-31.006},
    {-59.751,-130.644,-56.027,-97.060,94.790,-1.908 },
    {-59.943,-105.150,-124.247,-5.878,95.227,-0.827 },
    {-52.574,-133.325,-52.192,-102.406,86.896,-13.765 },
    {-62.556,-106.583,-118.589,-15.788,102.919,-31.812},
    {-55.097,-119.020,-68.405,-102.332,83.107,141.789 },
    {-33.623,-94.611,-111.940,-63.406,53.089,104.692},
    {-43.202,-86.234,-125.841,-32.458,65.216,60.606 },
    {-64.872,-132.526,-53.235,-98.107,99.886,9.230  },
    {-56.370,-103.994,-119.553,-16.526,86.191,25.717},
    {-60.502,-102.538,-91.474,-74.308,82.482,95.430 },
    {-72.497,-117.097,-77.199,-76.816,101.674,48.124},
    {-73.118,-126.561,-73.517,-68.668,104.296,42.114},
    {-67.009,-105.867,-89.455,-68.789,90.773,67.783 },
    {-77.336,-147.185,-41.654,-95.438,112.492,30.224},
    {-74.017,-101.402,-103.998,-58.936,122.810,-106.302},
    {-63.396,-104.293,-88.854,-78.742,86.430,105.176 },
    {-34.166,-128.665,-68.061,-96.597,66.509,-27.659 },
    {-47.075,-104.644,-113.061,-33.638,85.418,-62.383},
    {-76.918,-137.611,-48.037,-100.410,109.603,31.568},
    {-67.255,-113.150,-93.374,-51.821,93.623,58.358  },
    {-71.650,-135.253,-50.105,-99.175,105.615,23.283 },
    {-60.762,-108.366,-103.280,-40.388,87.796,48.973 },
    {-66.873,-112.536,-83.775,-69.986,97.706,36.170  },
    {-66.998,-120.875,-82.486,-60.182,98.843,34.112  },
    {-52.947,-95.934,-104.666,-57.538,75.377,75.061  },
    {-72.284,-158.466,-19.616,-112.429,109.218,18.603},
    {-57.618,-94.875,-110.432,-43.086,85.440,38.019  },
    {-68.110,-145.618,-40.387,-97.110,103.635,16.054 },
    {-62.414,-94.318,-115.821,-32.995,98.397,-3.074  },
    {-59.010,-145.181,-39.474,-99.193,93.606,0.408   },
    {-65.984,-104.656,-88.854,-79.328,89.937,109.387 },
    {-43.561,-133.057,-55.461,-102.815,76.802,-22.213},
    {-55.960,-106.162,-111.803,-30.709,95.778,-50.970},
    {-67.049,-96.374,-114.744,-35.264,109.356,-38.915},
    {-52.171,-143.610,-43.339,-98.933,85.440,-9.212  },
    {-84.053,-114.081,-91.618,-78.592,136.002,-126.864},
    {-50.667,-90.118,-114.712,-48.412,93.979,-86.323  },
    {-29.382,-152.221,-47.310,-107.636,56.221,-16.258 },
    {-60.785,-100.967,-99.016,-57.869,86.681,56.414   },
    {-78.451,-99.762,-117.133,-32.120,127.952,-63.673 },
    {-51.091,-84.638,-138.130,-4.971,78.887,27.526    },
    {-55.202,-99.481,-103.800,-53.804,94.840,-52.667  },
    {-43.807,-129.966,-69.564,-80.189,75.488,-22.738  },
    {-61.841,-98.762,-107.559,-45.033,101.189,-29.275 },
    {-52.622,-130.132,-66.466,-79.434,86.301,-11.589  },
    {-61.338,-98.732,-107.286,-43.860,96.750,-1.372   },
    {-58.795,-131.019,-64.204,-79.184,93.296,0.753    },
    {-60.805,-99.135,-105.195,-46.708,92.942,17.478   },
    {-63.292,-132.839,-61.266,-80.334,97.842,10.983   },
    {-61.192,-101.041,-101.043,-52.455,90.935,32.805  },
    {-64.860,-120.977,-82.308,-60.470,97.744,25.432   },
    {-87.325,-150.512,-106.628,49.307,153.560,-39.511 },
    {-85.145,-131.073,-111.925,8.286,143.887,-74.103  },
    {-29.549,-127.698,-136.609,42.813,44.170,59.559   },
    {-54.281,-113.558,-88.355,-64.597,89.356,-19.480  },
    {-45.937,-114.885,-86.959,-68.947,80.000,-32.506  },
    {-36.716,-113.431,-92.934,-66.834,69.769,-40.643 },
    {-62.434,-113.863,-88.078,-61.852,96.108,16.198  },
    {-68.389,-119.701,-78.220,-70.850,100.429,33.789 },
    {-74.074,-124.464,-71.329,-77.207,105.035,42.224 },
    {-62.620,-94.428,-115.780,-32.917,98.880,-4.723  },
    {-58.805,-153.038,-25.150,-109.511,93.363,-0.232 }
};

std::vector<std::vector<double>> list2={
    {0,170.46,-89.32,79.72,134.69,-9.88},
    {0,170.41,-89.33,79.70,132.68,-9.88},
    {-60.541,-123.270,-105.739,45.344,182.555,-1.912},
};
std::vector<std::vector<double>> list3={
    {-60.541,-123.270,-105.739,8, 200,-1.912},
    {-60.541,-123.270,-105.739,8, 160,-1.912},
    {-60.541,-123.270,-105.739,136, 100,-1.912},
};
std::vector<std::vector<double>> list4={
//    {-60.541,-90.270,-161,90, 145,-1.912},
//    {-60.541,-90.270,-163,  40, 145,-1.912},
//    {-60.541,-123.270,-100,90,130, 0},
//    {-60.541,-90,-165.5, 81, 0,-1.912},
    {-60.541,-123.270,-105.739,45.344,182.555,-1.912},
};

int test1() {

    RobotParams params;
    set_model(&params, RobotType::UR5e);//设置模型参数

    double threshold_distance = 20;

    std::cout<< "关节1-3\t关节1-4\t关节1-5\t关节2-4\t关节2-5\t关节3-5\t关节角 "<<std::endl;
    for(auto q :list4)//测试数据
    {
        std::string msg;
        std::vector<CollisionInfo> c_info;//关节距离
        auto t1 = std::chrono::high_resolution_clock::now();
        test(q, &params, c_info);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        //        std::cout<<"    duration  "<<duration<<std::endl<<std::endl;

        for(auto c:c_info)
        {
            msg.append(std::to_string(c.distance));
            msg.append("\t");
            if(threshold_distance >c.distance)
            {
                std::cout<<"小于阈值  碰撞";
                std::cout <<std::endl<<std::endl;
                break;
            }
        }
        for(int i =0;i<q.size();++i)
        {
            msg.append(std::to_string(q[i]));
            msg.append(", ");
        }

        std::cout<< msg<<std::endl;
    }
    return 0;
}

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fcl/geometry/collision_geometry.h>
int test2()
{
    /**
     * 计算两个模型的距离
     * @brief params
     */
    RobotParams params;
    set_model(&params, RobotType::UR5e);//设置模型参数
    std::vector<std::string> files;
    //    files.emplace_back("D:/Test/UR5e/base.STL");
    files.emplace_back("D:/Test/UR5e/link_1.STL");
    files.emplace_back("D:/Test/UR5e/link_2.STL");
    files.emplace_back("D:/Test/UR5e/link_3.STL");
    files.emplace_back("D:/Test/UR5e/link_4.STL");
    files.emplace_back("D:/Test/UR5e/link_5.STL");
    files.emplace_back("D:/Test/UR5e/link_6.STL");

    std::vector<std::shared_ptr<fcl::CollisionGeometryf>> objs;//需要检测碰撞的物体集合

    //加载 STL模型
    for (const auto &filename : files)
    {
        auto model = std::make_shared<fcl::BVHModel<fcl::OBBRSSf>>();

        Assimp::Importer importer;
        auto scene = importer.ReadFile( filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices); //导入单关节的碰撞体
        //        auto scene = importer.ReadFile( filename, aiProcess_Triangulate | aiProcess_FlipWindingOrder); //导入单关节的碰撞体
        if (!scene || !scene->HasMeshes()) {
            std::cerr << "Failed to load STL file: " << filename << std::endl;
            return 0;
        }

        for(size_t n=0;n<scene->mNumMeshes;++n)
        {
            auto mesh = scene->mMeshes[n];// 获取第n个网格（STL 文件应该只有一个网格）
            std::vector<fcl::Vector3f> vertices;// 准备存储顶点和三角形
            std::vector<fcl::Triangle> triangles;
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
                auto vertex = mesh->mVertices[i];
                vertices.push_back(fcl::Vector3f(vertex.x, vertex.y, vertex.z));
            }
            for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
                auto face = mesh->mFaces[i];
                if (face.mNumIndices == 3) {
                    triangles.push_back( fcl::Triangle(face.mIndices[0], face.mIndices[1], face.mIndices[2]));
                }
            }
            std::cout <<filename<< "  " << vertices.size() << " vertices  " << triangles.size() << " triangles" << std::endl;
            model->beginModel(vertices.size(), triangles.size());
            model->addSubModel(vertices, triangles); // 将顶点和三角形添加到 BVH 模型
            model->endModel();
        }
        objs.emplace_back(model);
    }

    //多组测试数据
    std::cout<< "关节1-3\t关节1-4\t关节1-5\t关节1-6\t关节2-4\t关节2-5\t关节2-6\t关节3-5\t关节3-6\t关节角 "<<std::endl;

    for(auto q :list4)//测试关节数据
    {
        std::string msg;
        std::vector<double> theta;
        for(int i=0;i<q.size();++i)
        {
            theta.emplace_back( d2r(q[i]) );//弧度不需要这个
        }
        std::vector<Eigen::Matrix4f> mats;
        for(int i=0;i<6;++i)
        {
            mats.emplace_back(fk(theta,i+1,&params));//计算每个模型的变换矩阵
        }

        //计算模型之间的距离
        for(int i=0; i<6;++i)
        {
            for(int j=i+2; j<6;++j)
            {
                if(i==3 && j==5)
                {
                    continue;
                }
                fcl::CollisionObjectf joint1(objs[i], mats[i].block<3,3>(0, 0), mats[i].block<3,1>(0, 3));//使用变换矩阵作用于模型
                fcl::CollisionObjectf joint2(objs[j], mats[j].block<3,3>(0, 0), mats[j].block<3,1>(0, 3));
                fcl::DistanceRequestf requestd;
                fcl::DistanceResultf resultd;
                auto dis = fcl::distance(&joint1, &joint2, requestd, resultd);//计算模型距离
                //                std::cout << "关节 "<<i+1<<" "<<j+1<<" 距离: " << dis<<std::endl;
                msg.append(std::to_string(dis));
                msg.append("\t");
            }
        }
        for(int i =0;i<q.size();++i)
        {
            msg.append(std::to_string(q[i]));
            msg.append(", ");
        }

        std::cout<< msg<<std::endl;
        //        std::cout  <<std::endl;
    }
    return 0;
}

#include <vtkSTLReader.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkTriangleFilter.h>

int test3()
{

    RobotParams params;
    set_model(&params, RobotType::UR5e);//设置模型参数
    std::vector<std::string> files;
//    files.emplace_back("D:/Test/UR5e/base.STL");
    files.emplace_back("./link_1.STL");
    files.emplace_back("./link_2.STL");
    files.emplace_back("./link_3.STL");
    files.emplace_back("./link_4.STL");
    files.emplace_back("./link_5.STL");
    files.emplace_back("./link_6.STL");

    std::vector<std::shared_ptr<fcl::CollisionGeometryf>> objs;//需要检测碰撞的物体集合

    //加载 STL模型
    std::cout<<"load stl start"<<std::endl;

    for (const auto &filename : files)
    {
        std::cout<< "load "<<filename<<std::endl;
        auto model = std::make_shared<fcl::BVHModel<fcl::OBBRSSf>>();

        auto reader = vtkSmartPointer<vtkSTLReader>::New();
        reader->SetFileName(filename.c_str());
        reader->Update();
        auto triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
        triangleFilter->SetInputData(reader->GetOutput());
        triangleFilter->Update();
        auto polyData = triangleFilter->GetOutput();

        {
            std::vector<fcl::Vector3f> vertices;// 顶点
            std::vector<fcl::Triangle> triangles;// 三角形
             // 获取 顶点
            auto points = polyData->GetPoints();
            for (auto i = 0; i < points->GetNumberOfPoints(); ++i) {
                auto point = points->GetPoint(i);
                vertices.push_back(fcl::Vector3f(point[0], point[1], point[2]));
            }
            // 获取三角形
            auto polys = polyData->GetPolys();
            polys->InitTraversal();
            auto ptIds = vtkSmartPointer<vtkIdList>::New();
            while (polys->GetNextCell(ptIds))
            {
                if (ptIds->GetNumberOfIds() == 3)
                {
                    triangles.push_back(fcl::Triangle(ptIds->GetId(0), ptIds->GetId(1), ptIds->GetId(2)));
                }
            }
            std::cout<<vertices.size()<<" ,  "<<triangles.size()<<std::endl;
            model->beginModel(vertices.size(), triangles.size());
            model->addSubModel(vertices, triangles); // 将顶点和三角形添加到 BVH 模型
            model->endModel();
        }
        objs.emplace_back(model);
    }
    std::cout<<"load stl  end"<<std::endl;
    //多组测试数据
    std::cout<< "关节1-3\t关节1-4\t关节1-5\t关节1-6\t关节2-4\t关节2-5\t关节2-6\t关节3-5\t关节3-6\t关节角 "<<std::endl;
    for(auto q :list4)//测试关节数据
    {
        std::string msg;
        std::vector<double> theta;
        for(int i=0;i<q.size();++i)
        {
            theta.emplace_back( d2r(q[i]) );//弧度不需要这个
        }
        std::vector<Eigen::Matrix4f> mats;
        for(int i=0;i<6;++i)
        {
            mats.emplace_back(fk(theta,i+1,&params));//计算每个模型的变换矩阵
        }

        //计算模型之间的距离
        auto t1 = std::chrono::high_resolution_clock::now();

        std::cout<<"start  fcl::distance "<<std::endl;
        for(int i=0; i<3;++i)
        {
            for(int j=i+2; j<6;++j)
            {
                if(i==3 && j==5)
                {
                    continue;
                }
                fcl::CollisionObjectf joint1(objs[i], mats[i].block<3,3>(0, 0), mats[i].block<3,1>(0, 3));//使用变换矩阵作用于模型
                fcl::CollisionObjectf joint2(objs[j], mats[j].block<3,3>(0, 0), mats[j].block<3,1>(0, 3));
                fcl::DistanceRequestf requestd;
                fcl::DistanceResultf resultd;
                auto dis = fcl::distance(&joint1, &joint2, requestd, resultd);//计算模型距离
                //                std::cout << "关节 "<<i+1<<" "<<j+1<<" 距离: " << dis<<std::endl;
                msg.append(std::to_string(dis));
                msg.append("\t");
            }
        }
        std::cout<<"end  fcl::distance "<<std::endl;
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        std::cout<<"time: "<<duration<<std::endl;
        for(int i =0;i<q.size();++i)
        {
            msg.append(std::to_string(q[i]));
            msg.append(", ");
        }

        std::cout<< msg<<std::endl;
        //        std::cout  <<std::endl;
    }
    return 0;
}


int main(int argc, char *argv[])
{
    test3();
    system("pause");
    return 0;
}
