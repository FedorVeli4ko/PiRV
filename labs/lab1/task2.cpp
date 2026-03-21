#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <atomic>
#include <boost/thread.hpp>

//  Величко Фёдор Тимофеевич, ИУ1-41Б
//  ВАРИАНТ №2 
//  Лабораторная работа №1, задача №1

void sum_array_unsafe(const std::vector<int>& arr, long long& result, int start, int end) {
    for (int i = start; i < end; ++i) {
        result += arr[i];
    }
}

void sum_array_atomic(const std::vector<int>& arr, std::atomic<long long>& result, int start, int end) {
    long long local_sum = 0;
    for (int i = start; i < end; ++i) {
        local_sum += arr[i];
    }
    result += local_sum;
}

void sum_array_mutex(const std::vector<int>& arr, long long& result, std::mutex& mtx, int start, int end) {
    long long local_sum = 0;
    for (int i = start; i < end; ++i) {
        local_sum += arr[i];
    }
    std::lock_guard<std::mutex> lock(mtx);
    result += local_sum;
}

void run_task2(int num_threads, const std::vector<int>& arr) {
    std::cout << "\n--- Задача 2: Сумма массива (" << num_threads << " потоков) ---\n";

    //без синхронизации
    {
        long long result = 0;
        std::vector<boost::thread> threads;
        int chunk = arr.size() / num_threads;

        auto start_time = std::chrono::steady_clock::now();

        for (int i = 0; i < num_threads; ++i) {
            int start = i * chunk;
            int end = (i == num_threads - 1) ? arr.size() : (i + 1) * chunk;
            threads.emplace_back(sum_array_unsafe, std::ref(arr), std::ref(result), start, end);
        }

        for (auto& t : threads) t.join();

        auto end_time = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::cout << "Без синхронизации: " << ms << " ms, результат = " << result << "\n";
    }

    //с atomic
    {
        std::atomic<long long> result(0);
        std::vector<boost::thread> threads;
        int chunk = arr.size() / num_threads;

        auto start_time = std::chrono::steady_clock::now();

        for (int i = 0; i < num_threads; ++i) {
            int start = i * chunk;
            int end = (i == num_threads - 1) ? arr.size() : (i + 1) * chunk;
            threads.emplace_back(sum_array_atomic, std::ref(arr), std::ref(result), start, end);
        }

        for (auto& t : threads) t.join();

        auto end_time = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::cout << "С std::atomic: " << ms << " ms, результат = " << result.load() << "\n";
    }

    //с mutex
    {
        long long result = 0;
        std::mutex mtx;
        std::vector<boost::thread> threads;
        int chunk = arr.size() / num_threads;

        auto start_time = std::chrono::steady_clock::now();

        for (int i = 0; i < num_threads; ++i) {
            int start = i * chunk;
            int end = (i == num_threads - 1) ? arr.size() : (i + 1) * chunk;
            threads.emplace_back(sum_array_mutex, std::ref(arr), std::ref(result), std::ref(mtx), start, end);
        }

        for (auto& t : threads) t.join();

        auto end_time = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        std::cout << "С std::mutex: " << ms << " ms, результат = " << result << "\n";
    }
}

int main() {
    const int ARRAY_SIZE = 10'000'000;

    std::cout << "\n========== Задача 2: Сумма массива ==========\n";
    std::vector<int> arr(ARRAY_SIZE, 1);

    for (int threads : {2, 4, 8}) {
        run_task2(threads, arr);
    }

    return 0;
}