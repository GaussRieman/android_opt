//
// Created by frank on 18-12-7.
//

#ifndef CPU_TUNE_H
#define CPU_TUNE_H

#ifdef __ANDROID__
#include <vector>
#include <thread>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <android/log.h>

#define  LOG_TAG    "CPU_TUNE"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static int g_cpucount = std::thread::hardware_concurrency();

#ifdef USE_POWERSDK
#include "power.hh"
static bool qspower_init_flag = qspower_init();;
#endif // END USE_POWERSDK

#ifdef DEBUG_CPU_TUNE
static bool debug_print = true;
#else
static bool debug_print = false;
#endif

#endif // END ANDROID


enum cpuCores{
    ALL     = 0, // all cores activated
    LITTLE  = 1, // excuted on LITTLE cores only
    BIG     = 2  // excuted on BIG cores only
};

enum cpuMode{
    NORMAL      = 0,
    SAVER       = 1,
    EFFICIENT   = 2,
    PERFORMANCE = 3,
};



#ifdef __ANDROID__
static int get_max_freq_khz(int cpuid)
{
    // first try, for all possible cpu
    char path[256];
    sprintf(path, "/sys/devices/system/cpu/cpufreq/stats/cpu%d/time_in_state", cpuid);

    FILE* fp = fopen(path, "rb");

    if (!fp)
    {
        // second try, for online cpu
        sprintf(path, "/sys/devices/system/cpu/cpu%d/cpufreq/stats/time_in_state", cpuid);
        fp = fopen(path, "rb");

        if (!fp)
        {
            // third try, for online cpu
            sprintf(path, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cpuid);
            fp = fopen(path, "rb");

            if (!fp)
                return -1;

            int max_freq_khz = -1;
            fscanf(fp, "%d", &max_freq_khz);

            fclose(fp);

            return max_freq_khz;
        }
    }

    int max_freq_khz = 0;
    while (!feof(fp))
    {
        int freq_khz = 0;
        int nscan = fscanf(fp, "%d %*d", &freq_khz);
        if (nscan != 1)
            break;

        if (freq_khz > max_freq_khz)
            max_freq_khz = freq_khz;
    }

    fclose(fp);

    return max_freq_khz;
}

static int set_sched_affinity(const std::vector<int>& cpuids)
{
    // cpu_set_t definition
    // ref http://stackoverflow.com/questions/16319725/android-set-thread-affinity
#define CPU_SETSIZE 1024
#define __NCPUBITS  (8 * sizeof (unsigned long))
    typedef struct
    {
        unsigned long __bits[CPU_SETSIZE / __NCPUBITS];
    } cpu_set_t;

#define CPU_SET(cpu, cpusetp) \
  ((cpusetp)->__bits[(cpu)/__NCPUBITS] |= (1UL << ((cpu) % __NCPUBITS)))

#define CPU_ZERO(cpusetp) \
  memset((cpusetp), 0, sizeof(cpu_set_t))

    // set affinity for thread
#ifdef __GLIBC__
    pid_t pid = syscall(SYS_gettid);
#else
#ifdef PI3
    pid_t pid = getpid();
#else
    pid_t pid = gettid();
#endif
#endif
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i=0; i<(int)cpuids.size(); i++)
    {
        CPU_SET(cpuids[i], &mask);
    }

    int syscallret = syscall(__NR_sched_setaffinity, pid, sizeof(mask), &mask);
    if (syscallret)
    {
        if (debug_print)
            LOGE("CPU syscall error %d", syscallret);
        
        return -1;
    }

    return 0;
}

static int sort_cpuid_by_max_frequency(std::vector<int>& cpuids, int* little_cluster_offset)
{
    const int cpu_count = cpuids.size();

    *little_cluster_offset = 0;

    if (cpu_count == 0)
        return 0;

    std::vector<int> cpu_max_freq_khz;
    cpu_max_freq_khz.resize(cpu_count);

    for (int i=0; i<cpu_count; i++)
    {
        int max_freq_khz = get_max_freq_khz(i);

//         printf("%d max freq = %d khz\n", i, max_freq_khz);

        cpuids[i] = i;
        cpu_max_freq_khz[i] = max_freq_khz;
    }

    // sort cpuid as big core first
    // simple bubble sort
    for (int i=0; i<cpu_count; i++)
    {
        for (int j=i+1; j<cpu_count; j++)
        {
            if (cpu_max_freq_khz[i] < cpu_max_freq_khz[j])
            {
                // swap
                int tmp = cpuids[i];
                cpuids[i] = cpuids[j];
                cpuids[j] = tmp;

                tmp = cpu_max_freq_khz[i];
                cpu_max_freq_khz[i] = cpu_max_freq_khz[j];
                cpu_max_freq_khz[j] = tmp;
            }
        }
    }

    // SMP
    int mid_max_freq_khz = (cpu_max_freq_khz.front() + cpu_max_freq_khz.back()) / 2;
    if (mid_max_freq_khz == cpu_max_freq_khz.back())
        return 0;

    for (int i=0; i<cpu_count; i++)
    {
        if (cpu_max_freq_khz[i] < mid_max_freq_khz)
        {
            *little_cluster_offset = i;
            break;
        }
    }

    return 0;
}

int set_cpuCores(cpuCores cores){
    static std::vector<int> sorted_cpuids;
    static int little_cluster_offset = 0;

    if (sorted_cpuids.empty()) {
        // 0 ~ g_cpucount
        sorted_cpuids.resize(g_cpucount);
        for (int i = 0; i < g_cpucount; i++) {
            sorted_cpuids[i] = i;
        }

        // descent sort by max frequency
        sort_cpuid_by_max_frequency(sorted_cpuids, &little_cluster_offset);
    }

    if (little_cluster_offset == 0 && cores != ALL) {
        cores = ALL;
        if (debug_print)
            LOGE("CPU cores not supported");
    }

    // prepare affinity cpuid
    std::vector<int> cpuids;
    switch (cores)
    {
        case BIG:
            cpuids = std::vector<int>(sorted_cpuids.begin(), sorted_cpuids.begin() + little_cluster_offset);
            break;
        case LITTLE:
            cpuids = std::vector<int>(sorted_cpuids.begin() + little_cluster_offset, sorted_cpuids.end());
            break;
        case ALL:
            cpuids = sorted_cpuids;
            break;
        default:
            return -1;
    }

#ifdef _OPENMP
    // set affinity for each thread
    int num_threads = cpuids.size();
    omp_set_num_threads(num_threads);
    std::vector<int> ssarets(num_threads, 0);
#pragma omp parallel for
    for (int i=0; i<num_threads; i++)
    {
        ssarets[i] = set_sched_affinity(cpuids);
    }
    for (int i=0; i<num_threads; i++)
    {
        if (ssarets[i] != 0)
        {
            return -1;
        }
    }
#else
    int ssaret = set_sched_affinity(cpuids);
    if (ssaret != 0)
    {
        return -1;
    }
#endif
}

#ifdef USE_POWERSDK

void terminateQspower(){
    if (qspower_init_flag)
    {
        qspower::request_mode(qspower::mode::normal, qspower::device_set{ qspower::device_type::cpu});
        qspower::terminate();
    }
}

int set_cpuMode(cpuMode mode){
    if (! qspower_init_flag)
        return -1;

    switch (mode){
        case PERFORMANCE:
            qspower::request_mode(qspower::mode::perf_burst, qspower::device_set{ qspower::device_type::cpu});
            break;
        case SAVER:
            qspower::request_mode(qspower::mode::saver, qspower::device_set{ qspower::device_type::cpu});
            break;
        case EFFICIENT:
            qspower::request_mode(qspower::mode::efficient, qspower::device_set{ qspower::device_type::cpu});
            break;
        case NORMAL:
            qspower::request_mode(qspower::mode::normal, qspower::device_set{ qspower::device_type::cpu});
            break;
        default:
            return -1;
    }
    return 1;
}
#endif

#endif // __ANDROID__

int set_cpuStatus(cpuCores cores, cpuMode mode) {
#ifdef __ANDROID__
    int ret = 0;
#ifdef USE_POWERSDK
    ret = set_cpuCores(cores);
    if (-1 == ret)
    {
        if (debug_print)
            LOGE("CPU set cores fail");
        return -1;
    }

    ret = set_cpuMode(mode);
    if (-1 == ret)
    {
        if (debug_print)
            LOGE("CPU set cores done, but set mode fail");
        return -2;
    }
#else
    ret = set_cpuCores(cores);
    if (-1 == ret)
    {
        if (debug_print)
            LOGE("CPU set mode fail");
        return -1;
    }
#endif // END USE_POWERSDK

    return 1;
#elif __IOS__
    // thread affinity not supported on ios
    return -1;
#else
    // TODO
    (void) cores;  // Avoid unused parameter warning.
    return -1;
#endif
}


#endif //CPU_TUNE_H
