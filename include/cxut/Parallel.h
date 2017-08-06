#pragma once

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define THREAD_FUNC_RETURN_TYPE unsigned int
#define THREAD_FUNC_RETURN_STATEMENT return 0
#else
#include <semaphore.h>
#define THREAD_FUNC_RETURN_TYPE void *
#define THREAD_FUNC_RETURN_STATEMENT
#endif

#define NUM_USE_CPU_THREADS 8

#define DECLARE_PARALLEL_MANUAL(className, funcName)\
    static THREAD_FUNC_RETURN_TYPE funcName##Thread(void *param) {\
        ThreadArg *arg = reinterpret_cast<ThreadArg *>(param);\
        className *p = reinterpret_cast<className *>(arg->parent);\
        p->funcName(arg->tIndex);\
        RandezvousOthers(*arg->syncInfo, arg->tIndex);\
        THREAD_FUNC_RETURN_STATEMENT;\
    }

#define DECLARE_PARALLEL_AUTO(className, funcName)\
    void funcName(size_t tIndex, size_t numCores, size_t numJobs, className::funcName##Data *pData) {\
        size_t startIndex;\
        size_t endIndex;\
        getJobRange(numCores, tIndex, numJobs, startIndex, endIndex);\
        for (size_t i = startIndex; i < endIndex; ++i)\
        funcName##Body(i, pData);\
    }\
static THREAD_FUNC_RETURN_TYPE funcName##Thread(void *param) {\
    ThreadArg *arg = reinterpret_cast<ThreadArg *>(param); \
    className *p = reinterpret_cast<className *>(arg->parent); \
    className::funcName##Data *pData = reinterpret_cast<className::funcName##Data *>(arg->data);\
    p->funcName(arg->tIndex, arg->numCores, arg->numJobs, pData); \
    RandezvousOthers(*arg->syncInfo, arg->tIndex); \
    return 0; \
}\
void funcName##Body(size_t index, className::funcName##Data *data)

#define PARALLEL_DATA(funcName) funcName##Data
#define DECLARE_PARALLEL_DATA(funcName)\
    struct funcName##Data

namespace cxut {
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
    struct SyncInfo {
        SyncInfo(int threadsCount) : Awaiting(threadsCount), ThreadsCount(threadsCount), Semaphore(::CreateSemaphore(0, 0, 1024, 0)) {};
        ~SyncInfo() { ::CloseHandle(this->Semaphore); }
        volatile unsigned int Awaiting;
        const int ThreadsCount;
        const HANDLE Semaphore;
    };

    static void RandezvousOthers(SyncInfo &sync, int ordinal) {
        if (0 == ::InterlockedDecrement(&(sync.Awaiting))) {
            sync.Awaiting = sync.ThreadsCount;
            ::ReleaseSemaphore(sync.Semaphore, sync.ThreadsCount - 1, 0);
        }
        else {
            ::WaitForSingleObject(sync.Semaphore, INFINITE);
        }
    }
#else
    struct SyncInfo {
        SyncInfo(int threadsCount) : Awaiting(threadsCount), ThreadsCount(threadsCount) {
            sem_init(&Semaphore, 0, threadsCount);
        };

        ~SyncInfo() {
            sem_destroy(&Semaphore);
        }

        volatile unsigned int Awaiting;
        const int ThreadsCount;
        sem_t Semaphore;
    };

    static void RandezvousOthers(SyncInfo &sync, int ordinal) {
        if (1 == __sync_fetch_and_sub(&sync.Awaiting, 1)) {
            sync.Awaiting = sync.ThreadsCount;
            sem_post(&sync.Semaphore);
        }
        else {
            sem_wait(&sync.Semaphore);
        }
    }
#endif
    struct ThreadArg {
        void *parent;
        void *data;
        size_t tIndex;
        size_t numCores;
        size_t numJobs;
        SyncInfo *syncInfo;
    };

    void getJobRange(size_t numCores, size_t tIndex, size_t numJobs, size_t& startIndex, size_t& endIndex);
    void parallelFor(THREAD_FUNC_RETURN_TYPE (*threadFunc)(void *params), void *parent, void *data, size_t numCores, size_t numJobs);
}
