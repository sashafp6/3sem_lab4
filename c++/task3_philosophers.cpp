#include "task3_philosophers.h"
#include "benchmark_utils.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <condition_variable>

using namespace std::chrono_literals;

namespace task3 {

// Самодельный двоичный семафор для C++17
class BinarySemaphore {
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool available;
    
public:
    BinarySemaphore(bool initial = true) : available(initial) {}
    
    void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return available; });
        available = false;
    }
    
    void release() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            available = true;
        }
        cv.notify_one();
    }
};

DiningPhilosophers::DiningPhilosophers(int num_philosophers, Strategy strategy)
    : num_philosophers_(num_philosophers), strategy_(strategy) {}

void DiningPhilosophers::philosopher_mutex(int id, int iterations, bool verbose) {
    static std::vector<std::mutex> forks(num_philosophers_);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> think_dist(50, 200);
    std::uniform_int_distribution<> eat_dist(100, 300);
    
    int left_fork = id;
    int right_fork = (id + 1) % num_philosophers_;
    
    for (int i = 0; i < iterations; ++i) {
        // Размышление
        std::this_thread::sleep_for(std::chrono::milliseconds(think_dist(gen)));
        
        if (verbose && i < 10) {
            std::cout << "Философ " << id << " размышляет (итерация " << i + 1 << ")\n";
        }
        
        // Захват вилок в определенном порядке для избежания deadlock
        if (id % 2 == 0) {
            forks[left_fork].lock();
            forks[right_fork].lock();
        } else {
            forks[right_fork].lock();
            forks[left_fork].lock();
        }
        
        // Еда
        std::this_thread::sleep_for(std::chrono::milliseconds(eat_dist(gen)));
        
        if (verbose && i < 10) {
            std::cout << "Философ " << id << " ест спагетти (итерация " << i + 1 << ")\n";
        }
        
        // Освобождение вилок
        forks[left_fork].unlock();
        forks[right_fork].unlock();
    }
}

void DiningPhilosophers::philosopher_semaphore(int id, int iterations, bool verbose) {
    static std::vector<BinarySemaphore> forks(num_philosophers_);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> think_dist(50, 200);
    std::uniform_int_distribution<> eat_dist(100, 300);
    
    int left_fork = id;
    int right_fork = (id + 1) % num_philosophers_;
    
    for (int i = 0; i < iterations; ++i) {
        // Размышление
        std::this_thread::sleep_for(std::chrono::milliseconds(think_dist(gen)));
        
        if (verbose && i < 10) {
            std::cout << "Философ " << id << " размышляет (итерация " << i + 1 << ")\n";
        }
        
        // Захват вилок
        forks[left_fork].acquire();
        forks[right_fork].acquire();
        
        // Еда
        std::this_thread::sleep_for(std::chrono::milliseconds(eat_dist(gen)));
        
        if (verbose && i < 10) {
            std::cout << "Философ " << id << " ест спагетти (итерация " << i + 1 << ")\n";
        }
        
        // Освобождение вилок
        forks[left_fork].release();
        forks[right_fork].release();
    }
}

void DiningPhilosophers::philosopher_try_lock(int id, int iterations, bool verbose) {
    static std::vector<std::mutex> forks(num_philosophers_);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> think_dist(50, 200);
    std::uniform_int_distribution<> eat_dist(100, 300);
    std::uniform_int_distribution<> retry_dist(10, 50);
    
    int left_fork = id;
    int right_fork = (id + 1) % num_philosophers_;
    
    for (int i = 0; i < iterations; ++i) {
        // Размышление
        std::this_thread::sleep_for(std::chrono::milliseconds(think_dist(gen)));
        
        if (verbose && i < 10) {
            std::cout << "Философ " << id << " размышляет (итерация " << i + 1 << ")\n";
        }
        
        // Попытка захвата вилок с повторными попытками
        bool has_forks = false;
        while (!has_forks) {
            if (forks[left_fork].try_lock()) {
                if (forks[right_fork].try_lock()) {
                    has_forks = true;
                } else {
                    forks[left_fork].unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(retry_dist(gen)));
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(retry_dist(gen)));
            }
        }
        
        // Еда
        std::this_thread::sleep_for(std::chrono::milliseconds(eat_dist(gen)));
        
        if (verbose && i < 10) {
            std::cout << "Философ " << id << " ест спагетти (итерация " << i + 1 << ")\n";
        }
        
        // Освобождение вилок
        forks[left_fork].unlock();
        forks[right_fork].unlock();
    }
}

void DiningPhilosophers::philosopher_arbitrator(int id, int iterations, bool verbose) {
    static std::mutex table_mutex;
    static std::vector<bool> forks_available(num_philosophers_, true);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> think_dist(50, 200);
    std::uniform_int_distribution<> eat_dist(100, 300);
    
    int left_fork = id;
    int right_fork = (id + 1) % num_philosophers_;
    
    for (int i = 0; i < iterations; ++i) {
        // Размышление
        std::this_thread::sleep_for(std::chrono::milliseconds(think_dist(gen)));
        
        if (verbose && i < 10) {
            std::cout << "Философ " << id << " размышляет (итерация " << i + 1 << ")\n";
        }
        
        // Запрос разрешения у арбитра (мьютекс стола)
        bool has_permission = false;
        while (!has_permission) {
            std::unique_lock<std::mutex> lock(table_mutex);
            
            if (forks_available[left_fork] && forks_available[right_fork]) {
                forks_available[left_fork] = false;
                forks_available[right_fork] = false;
                has_permission = true;
            }
            
            if (!has_permission) {
                lock.unlock();
                std::this_thread::sleep_for(10ms);
            }
        }
        
        // Еда
        std::this_thread::sleep_for(std::chrono::milliseconds(eat_dist(gen)));
        
        if (verbose && i < 10) {
            std::cout << "Философ " << id << " ест спагетти (итерация " << i + 1 << ")\n";
        }
        
        // Возврат вилок
        {
            std::lock_guard<std::mutex> lock(table_mutex);
            forks_available[left_fork] = true;
            forks_available[right_fork] = true;
        }
    }
}

void DiningPhilosophers::philosopher_resource_hierarchy(int id, int iterations, bool verbose) {
    static std::vector<std::mutex> forks(num_philosophers_);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> think_dist(50, 200);
    std::uniform_int_distribution<> eat_dist(100, 300);
    
    // Присваиваем порядковые номера вилкам
    int first_fork, second_fork;
    
    if (id == num_philosophers_ - 1) {
        // Последний философ берет вилки в обратном порядке
        first_fork = 0;
        second_fork = id;
    } else {
        first_fork = id;
        second_fork = (id + 1) % num_philosophers_;
    }
    
    // Убедимся, что first_fork имеет меньший номер
    if (first_fork > second_fork) {
        std::swap(first_fork, second_fork);
    }
    
    for (int i = 0; i < iterations; ++i) {
        // Размышление
        std::this_thread::sleep_for(std::chrono::milliseconds(think_dist(gen)));
        
        if (verbose && i < 10) {
            std::cout << "Философ " << id << " размышляет (итерация " << i + 1 << ")\n";
        }
        
        // Захват вилок в порядке возрастания номеров
        forks[first_fork].lock();
        forks[second_fork].lock();
        
        // Еда
        std::this_thread::sleep_for(std::chrono::milliseconds(eat_dist(gen)));
        
        if (verbose && i < 10) {
            std::cout << "Философ " << id << " ест спагетти (итерация " << i + 1 << ")\n";
        }
        
        // Освобождение вилок
        forks[second_fork].unlock();
        forks[first_fork].unlock();
    }
}

void DiningPhilosophers::run_simulation(int iterations, bool verbose) {
    std::vector<std::thread> philosophers;
    
    std::string strategy_name;
    switch (strategy_) {
        case Strategy::MUTEX: strategy_name = "Мьютексы"; break;
        case Strategy::SEMAPHORE: strategy_name = "Семафоры"; break;
        case Strategy::TRY_LOCK: strategy_name = "Попытка захвата"; break;
        case Strategy::ARBITRATOR: strategy_name = "Арбитр"; break;
        case Strategy::RESOURCE_HIERARCHY: strategy_name = "Иерархия ресурсов"; break;
    }
    
    std::cout << "\n=== Задача обедающих философов ===\n";
    std::cout << "Философов: " << num_philosophers_ << "\n";
    std::cout << "Стратегия: " << strategy_name << "\n";
    std::cout << "Итераций: " << iterations << "\n";
    
    if (verbose && iterations > 10) {
        std::cout << "(Вывод ограничен первыми 10 итерациями)\n";
    }
    
    // Запуск философов
    for (int i = 0; i < num_philosophers_; ++i) {
        switch (strategy_) {
            case Strategy::MUTEX:
                philosophers.emplace_back(&DiningPhilosophers::philosopher_mutex, 
                                         this, i, iterations, verbose);
                break;
            case Strategy::SEMAPHORE:
                philosophers.emplace_back(&DiningPhilosophers::philosopher_semaphore, 
                                         this, i, iterations, verbose);
                break;
            case Strategy::TRY_LOCK:
                philosophers.emplace_back(&DiningPhilosophers::philosopher_try_lock, 
                                         this, i, iterations, verbose);
                break;
            case Strategy::ARBITRATOR:
                philosophers.emplace_back(&DiningPhilosophers::philosopher_arbitrator, 
                                         this, i, iterations, verbose);
                break;
            case Strategy::RESOURCE_HIERARCHY:
                philosophers.emplace_back(&DiningPhilosophers::philosopher_resource_hierarchy, 
                                         this, i, iterations, verbose);
                break;
        }
    }
    
    // Ожидание завершения
    for (auto& p : philosophers) {
        p.join();
    }
    
    std::cout << "\nСимуляция завершена успешно!\n";
}

void DiningPhilosophers::run_benchmark(int max_philosophers, int iterations) {
    std::cout << "\n=== Бенчмарк задачи обедающих философов ===\n";
    std::cout << "Тестируем разные стратегии и количество философов\n\n";
    
    std::vector<std::pair<std::string, double>> benchmark_results;
    
    std::vector<Strategy> strategies = {
        Strategy::MUTEX,
        Strategy::SEMAPHORE,
        Strategy::TRY_LOCK,
        Strategy::ARBITRATOR,
        Strategy::RESOURCE_HIERARCHY
    };
    
    std::vector<std::string> strategy_names = {
        "Мьютексы",
        "Семафоры",
        "Попытка захвата",
        "Арбитр",
        "Иерархия ресурсов"
    };
    
    std::vector<int> philosopher_counts = {5, 10, 20};
    
    for (int count : philosopher_counts) {
        if (count > max_philosophers) continue;
        
        for (size_t s = 0; s < strategies.size(); ++s) {
            std::string test_name = std::to_string(count) + "_философов_" + strategy_names[s];
            
            std::cout << "Тестируем: " << test_name << "... ";
            
            DiningPhilosophers dp(count, strategies[s]);
            
            Benchmark b(test_name, false);
            dp.run_simulation(iterations, false);
            
            double time = b.elapsed_microseconds();
            benchmark_results.emplace_back(test_name, time);
            
            std::cout << time << " мкс\n";
        }
    }
    
    Benchmark::save_to_csv(benchmark_results, "philosophers_benchmark.csv");
    std::cout << "\nБенчмарк завершен. Результаты сохранены в philosophers_benchmark.csv\n";
}

void run_philosophers() {
    std::cout << "\n=== Задание 3: Обедающие философы ===\n";
    std::cout << "Классическая задача синхронизации\n\n";
    
    int choice;
    std::cout << "Выберите режим:\n";
    std::cout << "1. Стандартная симуляция\n";
    std::cout << "2. Расширенный бенчмарк\n";
    std::cout << "Ваш выбор: ";
    std::cin >> choice;
    
    switch (choice) {
        case 1: {
            int num_philosophers, iterations, strategy_choice;
            
            std::cout << "\nВведите количество философов (2-20): ";
            std::cin >> num_philosophers;
            
            std::cout << "Введите количество итераций на философа (1-100): ";
            std::cin >> iterations;
            
            std::cout << "\nВыберите стратегию синхронизации:\n";
            std::cout << "1. Мьютексы (стандартная)\n";
            std::cout << "2. Семафоры\n";
            std::cout << "3. Попытка захвата (try_lock)\n";
            std::cout << "4. Арбитр (официант)\n";
            std::cout << "5. Иерархия ресурсов\n";
            std::cout << "Ваш выбор: ";
            std::cin >> strategy_choice;
            
            if (num_philosophers < 2) num_philosophers = 2;
            if (num_philosophers > 20) num_philosophers = 20;
            if (iterations < 1) iterations = 1;
            if (iterations > 100) iterations = 100;
            
            DiningPhilosophers::Strategy strategy;
            switch (strategy_choice) {
                case 1: strategy = DiningPhilosophers::Strategy::MUTEX; break;
                case 2: strategy = DiningPhilosophers::Strategy::SEMAPHORE; break;
                case 3: strategy = DiningPhilosophers::Strategy::TRY_LOCK; break;
                case 4: strategy = DiningPhilosophers::Strategy::ARBITRATOR; break;
                case 5: strategy = DiningPhilosophers::Strategy::RESOURCE_HIERARCHY; break;
                default: strategy = DiningPhilosophers::Strategy::MUTEX;
            }
            
            DiningPhilosophers dp(num_philosophers, strategy);
            
            Benchmark b("Симуляция обедающих философов");
            dp.run_simulation(iterations, true);
            break;
        }
        case 2: {
            int iterations;
            std::cout << "\nВведите количество итераций на философа (10-100): ";
            std::cin >> iterations;
            
            if (iterations < 10) iterations = 10;
            if (iterations > 100) iterations = 100;
            
            run_philosophers_benchmark();
            break;
        }
        default:
            std::cout << "Неверный выбор! Запускаю стандартную симуляцию...\n";
            DiningPhilosophers dp(5, DiningPhilosophers::Strategy::MUTEX);
            dp.run_simulation(10, true);
    }
}

void run_philosophers_benchmark() {
    std::cout << "\n=== Расширенный бенчмарк обедающих философов ===\n";
    
    int iterations;
    std::cout << "Введите количество итераций на философа (10-100): ";
    std::cin >> iterations;
    
    if (iterations < 10) iterations = 10;
    if (iterations > 100) iterations = 100;
    
    std::cout << "\nТестируем все стратегии...\n";
    
    DiningPhilosophers dp(5, DiningPhilosophers::Strategy::MUTEX);
    dp.run_benchmark(20, iterations);
}

} // namespace task3
