#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <mutex>
#include <atomic>
#include <boost/thread.hpp>

// ------------------------------------------------------------
// Задача 1: Поиск простых чисел
// ------------------------------------------------------------

bool is_prime(int n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (int i = 3; i <= std::sqrt(n); i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

void count_primes_in_range(int start, int end, long long& result, std::mutex& mtx) {
    long long local_count = 0;
    for (int i = start; i <= end; ++i) {
        if (is_prime(i)) local_count++;
    }
    std::lock_guard<std::mutex> lock(mtx);
    result += local_count;
}

void task1_boost_threads(int N, int num_threads) {
    std::vector<boost::thread> threads;
    std::mutex mtx;
    long long total_primes = 0;

    int range_size = N / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * range_size + 1;
        int end = (i == num_threads - 1) ? N : (i + 1) * range_size;
        threads.emplace_back(count_primes_in_range, start, end, std::ref(total_primes), std::ref(mtx));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Найдено простых чисел: " << total_primes << std::endl;
}

void task1_single_thread(int N) {
    long long total_primes = 0;
    for (int i = 1; i <= N; ++i) {
        if (is_prime(i)) total_primes++;
    }
    std::cout << "Найдено простых чисел: " << total_primes << std::endl;
}

// ------------------------------------------------------------
// Задача 2: Сумма массива
// ------------------------------------------------------------

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

        std::cout << "Без синхронизации: " << ms << " ms, результат = " << result << " (может быть неверным)\n";
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
    const int N = 10'000'000; // Для задачи 1
    const int ARRAY_SIZE = 10'000'000; // Для задачи 2

    std::cout << "========== Задача 1: Поиск простых чисел ==========\n";

    // Однопоточный вариант
    auto start = std::chrono::steady_clock::now();
    task1_single_thread(N);
    auto end = std::chrono::steady_clock::now();
    auto ms_single = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Однопоточный режим: " << ms_single << " ms\n\n";

    // Многопоточный вариант (2, 4, 8 потоков)
    for (int threads : {2, 4, 8}) {
        start = std::chrono::steady_clock::now();
        task1_boost_threads(N, threads);
        end = std::chrono::steady_clock::now();
        auto ms_multi = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Многопоточный режим (" << threads << " потоков): " << ms_multi << " ms\n";
        std::cout << "Ускорение: " << (double)ms_single / ms_multi << "\n\n";
    }

    std::cout << "\n========== Задача 2: Сумма массива ==========\n";
    std::vector<int> arr(ARRAY_SIZE, 1); // все элементы = 1, сумма = ARRAY_SIZE

    for (int threads : {2, 4, 8}) {
        run_task2(threads, arr);
    }

    return 0;
}