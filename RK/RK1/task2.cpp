// Рубежный контроль №1
// студента группы ИУ1-41Б
// Величко Фёдора Тимофеевича
//============================
// ВАРИАНТ №4, задание №2

#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/chrono.hpp> //для наглядности сделал
#include <iostream>
#include <vector>
#include <sstream> //чтобы не путались строки с разных потоков в выводе 

const int ORDERS = 5;
const int DELIVERY_LIMIT = 2;

boost::interprocess::interprocess_semaphore processed(0);
boost::interprocess::interprocess_semaphore delivery_slots(DELIVERY_LIMIT);

boost::mutex cout_mutex;

void process_order(int id) {    //обработка
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));

    std::stringstream ss;
    ss << "Заказ " << id << " обработан\n";

    {
        boost::lock_guard<boost::mutex> lock(cout_mutex);
        std::cout << ss.str();
    }

    processed.post();
}

void deliver_order(int id) {    //доставка
    processed.wait();
    delivery_slots.wait();

    {
        std::stringstream ss;
        ss << "Доставка заказа " << id << " начата\n";

        boost::lock_guard<boost::mutex> lock(cout_mutex);
        std::cout << ss.str();
    }

    boost::this_thread::sleep_for(boost::chrono::seconds(1));

    {
        std::stringstream ss;
        ss << "Заказ " << id << " доставлен\n";

        boost::lock_guard<boost::mutex> lock(cout_mutex);
        std::cout << ss.str();
    }

    delivery_slots.post();
}

int main() {
    std::vector<boost::thread> workers;

    //обработка
    for (int i = 0; i < ORDERS; i++) {
        workers.emplace_back(process_order, i);
    }

    //доставка
    for (int i = 0; i < ORDERS; i++) {
        workers.emplace_back(deliver_order, i);
    }

    for (auto& t : workers)
        t.join();

    return 0;
}