#include <iostream>
#include <boost/thread.hpp>
void thread_function() {
 std::cout << "Привет из потока!" << std::endl;
}
int main() {
 boost::thread t(thread_function);
 t.join();
 return 0;
}