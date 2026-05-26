#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "Semaphore.hpp"
#include "Employee.hpp"

//  Величко Фёдор Тимофеевич, ИУ1-41Б
//  ВАРИАНТ №2 
//  Лабораторная работа №2, задача №1

std::priority_queue<Employee> queue_employees;
Semaphore turnstiles(5); // 5 входов
std::mutex mtx;

void employee_worker() {
    while (true) {
        mtx.lock();
        if (queue_employees.empty()) {
            mtx.unlock();
            return;
        }

        Employee emp = queue_employees.top();
        queue_employees.pop();
        mtx.unlock();

        turnstiles.acquire();

        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Employee " << emp.id
                      << (emp.high_priority ? " [VIP]" : "")
                      << " entered\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        turnstiles.release();
    }
}

int main() {
    // создаём сотрудников
    for (int i = 1; i <= 20; i++) {
        queue_employees.push({i, i % 5 == 0}); // VIP: 5,10,15,20
    }

    // адаптация если очередь большая
    if (queue_employees.size() > 10) {
        turnstiles.release(2); // добавили 2 турникета
        std::cout << "Extra turnstiles opened!\n";
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < 5; i++) {
        threads.emplace_back(employee_worker);
    }

    for (auto& t : threads)
        t.join();

    return 0;
}