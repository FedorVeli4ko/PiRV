#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include "Semaphore.hpp"
#include "Order.hpp"

//  Величко Фёдор Тимофеевич, ИУ1-41Б
//  ВАРИАНТ №2 
//  Лабораторная работа №2, задача №2

std::priority_queue<Order> orders;
std::mutex mtx;
Semaphore machines(4); // 4 станка

std::mt19937 gen(std::random_device{}());
std::uniform_int_distribution<> time_dist(500, 1500);

void worker() {
    while (true) {
        mtx.lock();
        if (orders.empty()) {
            mtx.unlock();
            return;
        }

        Order ord = orders.top();
        orders.pop();
        mtx.unlock();

        machines.acquire();

        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Processing order " << ord.id
                      << " (priority " << ord.priority << ")\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(time_dist(gen)));

        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Order " << ord.id << " completed\n";
        }

        machines.release();
    }
}

int main() {
    // создаем заказы
    for (int i = 1; i <= 15; i++) {
        int priority = rand() % 3 + 1; // 1..3
        orders.push({i, priority});
    }

    // имитация поломки
    bool broken = rand() % 2;
    if (broken) {
        machines.acquire(); // уменьшаем число станков
        std::cout << "One machine is broken! Capacity reduced.\n";
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < 4; i++) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads)
        t.join();

    return 0;
}