#include <cxut/Parallel.h>
#include <cassert>

#if defined(WIN32)
#include <Windows.h>
#else
#include <sched.h>
#include <pthread.h>
#endif

namespace cxut {
    void getJobRange(size_t numCores, size_t tIndex, size_t numJobs, size_t& startIndex, size_t& endIndex) {
        assert(numJobs >= numCores);
        size_t jobsPerCore = numJobs / numCores;
        startIndex = jobsPerCore * tIndex;
        endIndex = jobsPerCore * (tIndex + 1) - 1;
        if (tIndex == numCores - 1)
            endIndex = numJobs - 1;
    }

#if defined(WIN32)
    void parallelFor(THREAD_FUNC_RETURN_TYPE (*threadFunc)(void *params), void *parent, void *data, size_t numCores, size_t numJobs) {
        SyncInfo syncInfo(numCores);
        HANDLE *tHandles = new HANDLE[numCores];
        ThreadArg *args = new ThreadArg[numCores];

        for (int p = 0; p < numCores; p++) {
            args[p].parent = parent;
            args[p].tIndex = p;
            args[p].syncInfo = &syncInfo;
            args[p].numCores = numCores;
            args[p].numJobs = numJobs;
            args[p].data = data;
            DWORD dwThreadID;
            tHandles[p] = (HANDLE)_beginthreadex(nullptr, 0, threadFunc, &args[p], 0, (unsigned *)&dwThreadID);
        }

        WaitForMultipleObjects(numCores, tHandles, TRUE, INFINITE);
        delete[] tHandles;
        delete[] args;
    }
#else
    void parallelFor(THREAD_FUNC_RETURN_TYPE (*threadFunc)(void *params), void *parent, void *data, size_t numCores, size_t numJobs) {
        SyncInfo syncInfo(numCores);

        ThreadArg *args = new ThreadArg[numCores];
        pthread_t *threads = new pthread_t[numCores];

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        for (int p = 0; p < numCores; p++) {
            args[p].parent = parent;
            args[p].tIndex = p;
            args[p].syncInfo = &syncInfo;
            args[p].numCores = numCores;
            args[p].numJobs = numJobs;
            args[p].data = data;
            pthread_create(&threads[p], nullptr, threadFunc, &args[p]);

            CPU_SET(p, &cpuset);
            pthread_setaffinity_np(threads[p], sizeof(cpu_set_t), &cpuset);
        }

        for (int p = 0; p < numCores; p++) {
            void *retval;
            int err = pthread_join(threads[p], &retval);
        }

        delete[] threads;
        delete[] args;
    }
#endif
}

