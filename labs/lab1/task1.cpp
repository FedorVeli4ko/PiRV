#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <mutex>
#include <boost/thread.hpp>
#include <thread>

//  Величко Фёдор Тимофеевич, ИУ1-41Б
//  ВАРИАНТ №2 
//  Лабораторная работа №1, задача №1

bool is_prime(int n) {      //проверка числа простое/нет
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

int main() {
    const int N = 10'000'000;

    std::cout << "========== Задача 1: Поиск простых чисел ==========\n";

    //однопоточный вариант
    auto start = std::chrono::steady_clock::now();
    task1_single_thread(N);
    auto end = std::chrono::steady_clock::now();
    auto ms_single = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Однопоточный режим: " << ms_single << " ms\n\n";

    //многопоточный вариант (2, 4, 8, 16 и максимум)
    for (int threads : {2, 4, 8, 16, 22}) {
        start = std::chrono::steady_clock::now();
        task1_boost_threads(N, threads);
        end = std::chrono::steady_clock::now();
        auto ms_multi = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Многопоточный режим (" << threads << " потоков): " << ms_multi << " ms\n";
        std::cout << "Ускорение: " << (double)ms_single / ms_multi << "\n\n";
    }

    //  |
    //  |
    //  |     сколько всего потоков одновременно возможно на моём компе
    //  V           
    //std::cout << std::thread::hardware_concurrency();

    return 0;
}