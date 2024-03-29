#ifndef THREAD_H
#define THREAD_H

#include <android/log.h>
#define JNI_TAG "CCamera"
#define LOGE(format, ...) __android_log_print(ANDROID_LOG_ERROR,   JNI_TAG, format, ##__VA_ARGS__)

#include <Mutex.h>
#include <Condition.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <stdio.h>

typedef enum {
    Priority_Default = -1,
    Priority_Low = 0,
    Priority_Normal = 1,
    Priority_High = 2
} ThreadPriority;

class Runnable {
public:
    virtual ~Runnable(){}

    virtual void run() = 0;
};

/**
 * Thread can use a custom Runnable, but must delete Runnable constructor yourself
 */
class Thread : public Runnable {
public:

    Thread();

    Thread(Runnable *runnable);

    virtual ~Thread();

    void start();

    void join();

    void detach();

    pthread_t getId() const;

    bool isActive() const;

protected:
    static void *threadEntry(void *arg);

    int schedPriority(ThreadPriority priority);

    virtual void run();

    void lock();

    void unlock();
protected:
    Mutex mMutex;
    Condition mCondition;
    Runnable *mRunnable;
    ThreadPriority mPriority; // thread priority
    pthread_t mId;  // thread id
    bool mRunning;  // thread running
    bool mNeedJoin; // if call detach function, then do not call join function
};

inline Thread::Thread() {
    mNeedJoin = true;
    mRunning = false;
    mId = -1;
    mRunnable = NULL;
    mPriority = Priority_High;
}

inline Thread::Thread(Runnable *runnable) {
    mNeedJoin = false;
    mRunning = false;
    mId = -1;
    mRunnable = runnable;
    mPriority = Priority_High;
}

inline Thread::~Thread() {
    join();
    mRunnable = NULL;
}

inline void Thread::start() {
    if(mRunning) {
        LOGE("Thread start fail,  is running now.");
        return ;
    }

    pthread_attr_t attr;
    int ret = pthread_attr_init(&attr);

    ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    LOGE("pthread_attr_setschedpolicy  ret: %d", ret);

    struct sched_param sp;
    sp.sched_priority = 50;
    ret = pthread_attr_setschedparam(&attr, &sp);
    LOGE("pthread_attr_setschedparam  ret: %d", ret);

    pthread_create(&mId, &attr, threadEntry, this);
    mNeedJoin = true;

    // wait thread to run
    mMutex.lock();
    while (!isActive()) {
        mCondition.wait(mMutex);
    }
    mMutex.unlock();
}

inline void Thread::join() {
    LOGE("Thread  join === ");
    if (mId == -1 || !isActive()) {
        mId = -1;
        mNeedJoin = false;
        return;
    }

    LOGE("Thread  join   mId: %d, mNeedJoin: %d", mId, mNeedJoin);

    if (mId != -1 && mNeedJoin) {
        pthread_join(mId, NULL);
    }
    mNeedJoin = false;
    mId = -1;
    mMutex.lock();
    while (isActive()) {
        mCondition.wait(mMutex);
    }
    mMutex.unlock();
}

inline  void Thread::detach() {
    Mutex::Autolock lock(mMutex);
    LOGE("Thread  detach   mId: %ld", mId);
    if (mId >= 0) {
        pthread_detach(mId);
        mNeedJoin = false;
    }
}

inline pthread_t Thread::getId() const {
    return mId;
}

inline bool Thread::isActive() const {
    return mRunning;
}

inline void* Thread::threadEntry(void *arg) {
    Thread *thread = (Thread *) arg;

    if (thread != NULL) {
        thread->mRunning = true;
        thread->mCondition.signal();

        thread->schedPriority(thread->mPriority);

        // 绑定CPU0
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(0, &mask);
        if (sched_setaffinity(0, sizeof(mask), &mask) < 0) {
            LOGE("sched_setaffinity fail");
        }

        if(CPU_ISSET(0, &mask)) {
            LOGE("cpu set success, cpu: 0");
        }

        // when runnable is exit，run runnable else run()
        if (thread->mRunnable) {
            thread->mRunnable->run();
        } else {
            thread->run();
        }

        thread->mRunning = false;
        thread->mCondition.signal();
    }

    pthread_exit(NULL);

    return NULL;
}

inline int Thread::schedPriority(ThreadPriority priority) {
    if (priority == Priority_Default) {
        return 0;
    }

    struct sched_param sched;
    int policy;
    pthread_t thread = pthread_self();
    if (pthread_getschedparam(thread, &policy, &sched) < 0) {
        return -1;
    }

    if (priority == Priority_Low) {
        sched.sched_priority = sched_get_priority_min(policy);
    } else if (priority == Priority_High) {
        sched.sched_priority = sched_get_priority_max(policy);
    } else {
        int min_priority = sched_get_priority_min(policy);
        int max_priority = sched_get_priority_max(policy);
        sched.sched_priority = (min_priority + (max_priority - min_priority) / 2);
    }
    sched.sched_priority = 51;

    if (pthread_setschedparam(thread, policy, &sched) < 0) {
        return -1;
    }
    return 0;
}

inline void Thread::run() {
    // do nothing
}

inline void Thread::lock() {
    mMutex.lock();
}

inline void Thread::unlock() {
    mMutex.unlock();
}

#endif //THREAD_H
