#include <jni.h>
#include <string>
#include <iostream>
#include <random>
#include <math.h>
#include <android/log.h>
//#include <qspower/power.hh>
#include <sys/time.h>
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h> // syscall
#include <future>
#include  <omp.h>
#include "Eigen/Core"
#include <Eigen/Dense>
#include <vector>
#include <thread>
#include <pthread.h>
#include <assert.h>
//#include "cpu.h"
#include "cpu_tune.h"

#define gettid() syscall(__NR_gettid)
//#define gettidv2() syscall(SYS_gettid)


using namespace Eigen;
using namespace std;

#define  LOG_TAG    "nativeprint"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


extern "C" JNIEXPORT jstring

JNICALL
Java_com_example_frank_myapplication_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

/*
bool static_test(){
    LOGI("frank static test");

    //powerSDK相关
    struct timeval t1, t2, t3, t4;
    double time = 0.0;
    gettimeofday(&t1, NULL);

    const size_t N = 10000000;
    std::vector<float> a(N), b(N), c(N);
    std::random_device random_device;
    std::mt19937 generator(random_device());
    const float min_value = -1024.0, max_value = 1024.0;
    std::uniform_real_distribution<float> dist(min_value, max_value);

    const float alpha = 0.2f; // multiplicative factor

    qspower::init();

    bool isSupported = false;
    isSupported = qspower::is_supported();

    LOGD("frank isSupported : %d\n", isSupported);

    // request window mode of 40% to 50% frequency on cpu big cores for an indefinite period of time
    qspower::request_mode(qspower::mode::perf_burst, qspower::device_set{ qspower::device_type::cpu_big});

    // Initialize the source arrays with random numbers
//    for (size_t i = 0; i < N; i++) {
//        a[i] = dist(generator);
//        b[i] = dist(generator);
//    }
    volatile double d = 0.0;
    // computation, vector addition
    for (size_t i = 0; i < N; i++) {
//        c[i] = alpha * a[i] + b[i];
        d += cos(sin(tan(cosh(1.234))));
        d += sin(sin(tan(cosh(1.234))));
        d += tan(sin(tan(cosh(1.234))));
    }

    // return the system to the normal power state
    qspower::request_mode(qspower::mode::normal);

    qspower::terminate();

    gettimeofday(&t2, NULL);
    time = 1000*(t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec)/1000;
    LOGD("frank time = %lf,  d= %lf\n", time, d);

    if (isSupported){
        return true;
    } else{
        return false;
    }
}

bool dynamic_test(){
    const size_t N            = 1000000;                       // Number of elements in the vector
    const size_t iterations   = 10;                             // Number of times, entire vector operation will be performed
    const float  min_value    = -1024.0, max_value = 1024.0;    // Min and Max value for random number generator
    const float  alpha        = 0.2f;                           // multiplicative factor
    const int    desired_goal = 7800;                           // desired number of elements to be processed per millisecond

    std::vector<float>               a(N), b(N), c(N);
    std::random_device               random_device;
    std::mt19937                     generator(random_device());
    std::uniform_real_distribution<> dist(min_value, max_value);
    auto                             gen_dist = std::bind(dist, std::ref(generator));

    std::chrono::time_point<std::chrono::system_clock> begin;
    using float_millisecs = std::chrono::duration<float, std::chrono::milliseconds::period>;

    qspower::init();

    bool isSupported = qspower::is_supported();

    // Set the desired goal of how many elements are to be processed
    // with tolerance of 100 elements.
    int tolerance =

    qspower::set_goal(desired_goal, 100, qspower::cpu_big);

    for (size_t j = 0; j < iterations; j++)
    {
        begin = std::chrono::system_clock::now();

        // Initialize the source arrays with random numbers
        std::generate(a.begin(), a.end(), gen_dist);
        std::generate(b.begin(), b.end(), gen_dist);

        // add both arrays
        for (size_t i = 0; i < N; i++)
        {
            c[i] = sin(alpha * a[i] + b[i]);
        }

        // measure the time
        auto measured = float_millisecs(std::chrono::system_clock::now() - begin);

        // feed the SDK runtime with how many elements are processed per millisecond
        // so it can regulate the cores and frequencies
        qspower::regulate(N / measured.count());
    }

    // return the system to the normal power state
    qspower::clear_goal();

//    qspower::terminate();

    if (isSupported){
        return true;
    } else{
        return false;
    }
}
*/

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_frank_myapplication_MainActivity_test(JNIEnv *env, jobject instance) {

//#pragma omp parallel for num_threads(6)
//    for (int i = 0; i < 12; i++)
//    {
//        LOGI("frank num = %d", omp_get_thread_num());
//    }

    int row = 3000;
    int col = 3000;

    Eigen::MatrixXf a = Eigen::MatrixXf::Random(row, col);
    Eigen::MatrixXf b = Eigen::MatrixXf::Random(col, row);
    MatrixXf c = Eigen::MatrixXf::Random(row, col);
    MatrixXf d = Eigen::MatrixXf::Random(row, col);

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            a(i,j) = i+j*2;
        }
    }
    for (int i = 0; i < col; i++) {
        for (int j = 0; j < row; j++) {
            b(i,j) = i*2+j;
        }
    }

    cpuCores cores = BIG;
    cpuMode mode = SAVER;
    set_cpuStatus(cores, mode);
    struct timeval t1, t2, t3, t4;
    double time = 0.0;
    gettimeofday(&t1, NULL);

//    c = a * b;
//    d = a * b;

    gettimeofday(&t2, NULL);
    time = 1000*(t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec)/1000;
    LOGI("frank time1 =  %lf\n", time);

    terminateQspower();
    std::string hello = "tune test\n";
    return env->NewStringUTF(hello.c_str());
}