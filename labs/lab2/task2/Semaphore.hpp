#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

class Semaphore {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;

public:
    Semaphore(int count) : count(count) {}

    void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return count > 0; });
        count--;
    }

    void release(int n = 1) {
        std::lock_guard<std::mutex> lock(mtx);
        count += n;
        for (int i = 0; i < n; ++i)
            cv.notify_one();
    }
};
