//

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <atomic>
#include <boost/thread.hpp>

class BankAccount {
public:
    int id;
    std::string name;
    double balance;
    std::mutex mtx;

    BankAccount(int _id, const std::string& _name, double _balance)
        : id(_id), name(_name), balance(_balance) {}
    
    BankAccount(const BankAccount&) = delete;
    BankAccount& operator=(const BankAccount&) = delete;
    
    BankAccount(BankAccount&& other) noexcept
        : id(other.id), name(std::move(other.name)), balance(other.balance) {}
    
    BankAccount& operator=(BankAccount&& other) noexcept {
        if (this != &other) {
            id = other.id;
            name = std::move(other.name);
            balance = other.balance;
        }
        return *this;
    }
    
    double getBalance() {
        std::lock_guard<std::mutex> lock(mtx);
        return balance;
    }
    
    //true если снятие успешно, false если мало деняк :(
    bool withdraw(double amount) {
        std::lock_guard<std::mutex> lock(mtx);
        if (amount <= balance && amount > 0) {
            balance -= amount;
            return true;
        }
        return false;
    }
    
    void deposit(double amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance += amount;
    }
};

std::vector<BankAccount> accounts;
std::atomic<int> transactionsCompleted(0);
std::mutex printMutex;
std::mutex logMutex;
std::mutex randMutex;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> accountDist(0, 9);
std::uniform_real_distribution<> amountDist(100, 5000);

//структура для результата перевода
struct TransferResult {
    bool success;
    std::string reason;
};

void logTransaction(int fromId, int toId, double amount, const TransferResult& result) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    //логирование в файл (доп)
    {
        std::lock_guard<std::mutex> lock(logMutex);
        std::ofstream logFile("transactions.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
                    << " | FROM: " << fromId << " (" << accounts[fromId].name << ")"
                    << " TO: " << toId << " (" << accounts[toId].name << ")"
                    << " | AMOUNT: " << std::fixed << std::setprecision(2) << amount
                    << " | STATUS: " << (result.success ? "SUCCESS" : "FAILED");
            if (!result.success) {
                logFile << " | REASON: " << result.reason;
            }
            logFile << std::endl;
        }
    }
    
    //вывод в терминал
    {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << std::put_time(std::localtime(&time), "%H:%M:%S")
                  << " | " << accounts[fromId].name << " -> " << accounts[toId].name
                  << " | " << std::fixed << std::setprecision(2) << amount << " руб."
                  << " | " << (result.success ? "УСПЕХ" : "ОТКАЗ");
        
        if (!result.success) {
            std::cout << " | " << result.reason;
        }
        
        if (result.success) {
            std::cout << " | Остаток у " << accounts[fromId].name << ": " 
                      << accounts[fromId].getBalance() << " руб.";
        }
        std::cout << std::endl;
    }
}

TransferResult transferMoney(int fromId, int toId, double amount) {
    TransferResult result;
    
    if (fromId == toId) {
        result.success = false;
        result.reason = "Перевод самому себе";
        return result;
    }
    
    BankAccount& from = accounts[fromId];
    BankAccount& to = accounts[toId];
    
    if (fromId < toId) {
        std::lock(from.mtx, to.mtx);
        std::lock_guard<std::mutex> lock1(from.mtx, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(to.mtx, std::adopt_lock);
    } else {
        std::lock(to.mtx, from.mtx);
        std::lock_guard<std::mutex> lock1(to.mtx, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(from.mtx, std::adopt_lock);
    }
    
    //если мало деняк :(
    {
        std::lock_guard<std::mutex> lock(from.mtx);
        if (amount > from.balance) {
            result.success = false;
            result.reason = "Недостаточно средств (доступно: " + 
                           std::to_string(from.balance) + " руб.)";
            return result;
        }
    }
    
    if (from.withdraw(amount)) {
        to.deposit(amount);
        result.success = true;
        return result;
    }
    
    result.success = false;
    result.reason = "Недостаточно средств";
    return result;
}

void processTransactions(int threadId, int totalTransactions) {
    while (transactionsCompleted.load() < totalTransactions) {
        int fromId, toId;
        double amount;
        
        {
            std::lock_guard<std::mutex> lock(randMutex);
            do {
                fromId = accountDist(gen);
                toId = accountDist(gen);
            } while (fromId == toId);
            amount = amountDist(gen);
        }
        
        TransferResult result = transferMoney(fromId, toId, amount);
        logTransaction(fromId, toId, amount, result);
        transactionsCompleted++;
    }
}

void printBalances(const std::string& title) {
    std::cout << "\n=== " << title << " ===\n";
    double total = 0;
    for (auto& acc : accounts) {
        double balance = acc.getBalance();
        std::cout << "Клиент " << std::setw(2) << acc.id 
                  << " (" << acc.name << "): " 
                  << std::fixed << std::setprecision(2) << balance << " руб.\n";
        total += balance;
    }
    std::cout << "--------------------------------\n";
    std::cout << "ОБЩИЙ БАЛАНС: " << total << " руб.\n";
    std::cout << "================================\n";
}

int main() {
    const int TOTAL_TRANSACTIONS = 100;
    const int NUM_THREADS = 5;
    
    std::cout << "=== БАНКОВСКОЕ ПРИЛОЖЕНИЕ ===\n";
    std::cout << "Клиентов: 10 | Потоков: " << NUM_THREADS << " | Транзакций: " << TOTAL_TRANSACTIONS << "\n\n";
    
    accounts.reserve(10);
    accounts.emplace_back(0, "Иванов", 10000);
    accounts.emplace_back(1, "Петров", 15000);
    accounts.emplace_back(2, "Сидоров", 12000);
    accounts.emplace_back(3, "Кузнецов", 8000);
    accounts.emplace_back(4, "Смирнов", 20000);
    accounts.emplace_back(5, "Васильев", 5000);
    accounts.emplace_back(6, "Попов", 18000);
    accounts.emplace_back(7, "Соколов", 22000);
    accounts.emplace_back(8, "Михайлов", 7000);
    accounts.emplace_back(9, "Федоров", 25000);
    
    printBalances("НАЧАЛЬНЫЕ БАЛАНСЫ");
    
    std::ofstream("transactions.log", std::ios::trunc).close();
    
    std::cout << "\n=== ТРАНЗАКЦИИ ===\n";
    
    auto start = std::chrono::steady_clock::now();
    
    std::vector<boost::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(processTransactions, i, TOTAL_TRANSACTIONS);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    printBalances("ИТОГОВЫЕ БАЛАНСЫ");
    
    std::cout << "\n=== СТАТИСТИКА ===\n";
    std::cout << "Выполнено: " << transactionsCompleted.load() << " транзакций\n";
    std::cout << "Время: " << duration << " мс\n";
    std::cout << "Лог: transactions.log\n";
    
    return 0;
}