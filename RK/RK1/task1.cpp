// Рубежный контроль №1
// студента группы ИУ1-41Б
// Величко Фёдора Тимофеевича
//============================
// ВАРИАНТ №4, задание №1

#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/atomic.hpp>
#include <iostream>

boost::mutex mtx;
boost::condition_variable cv;

boost::atomic<int> stage(0); 
// 0 - ничего
// 1 - сборка завершена
// 2 - покраска завершена

//собираем
void assembly() {
    boost::this_thread::sleep_for(boost::chrono::seconds(1));
    
    boost::unique_lock<boost::mutex> lock(mtx);
    std::cout << "Сборка завершена\n";
    
    stage.store(1);
    cv.notify_all();
}

//красим
void painting() {
    boost::unique_lock<boost::mutex> lock(mtx);
    
    while (stage.load() < 1)    //ждём сборку
        cv.wait(lock);

    lock.unlock();

    boost::this_thread::sleep_for(boost::chrono::seconds(1));

    lock.lock();
    std::cout << "Покраска завершена\n";
    
    stage.store(2);
    cv.notify_all();
}

//пакуем
void packaging() {
    boost::unique_lock<boost::mutex> lock(mtx);
    
    while (stage.load() < 2)    //ждём сборку и покраску
        cv.wait(lock);

    lock.unlock();

    boost::this_thread::sleep_for(boost::chrono::seconds(1));

    std::cout << "Упаковка завершена\n";
}

int main() {
    boost::thread t1(assembly);
    boost::thread t2(painting);
    boost::thread t3(packaging);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}