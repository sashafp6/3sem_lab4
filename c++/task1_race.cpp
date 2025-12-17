#include "task1_race.h"
#include "benchmark_utils.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <random>
#include <condition_variable>
#include <sstream>

using namespace std::chrono_literals;

namespace task1 {

// Класс SpinLock для тестирования
class SpinLock {
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    
public:
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // Активное ожидание
        }
    }
    
    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

// Класс SpinWait для тестирования
class SpinWait {
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    
public:
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield(); // Уступаем процессорное время
        }
    }
    
    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

// Класс Monitor для тестирования
class Monitor {
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool available = true;
    
public:
    void enter() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return available; });
        available = false;
    }
    
    void exit() {
        std::lock_guard<std::mutex> lock(mtx);
        available = true;
        cv.notify_one();
    }
};

// Самодельный семафор для C++17
class CustomSemaphore {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
    
public:
    CustomSemaphore(int initial = 1) : count(initial) {}
    
    void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return count > 0; });
        count--;
    }
    
    void release() {
        std::lock_guard<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }
};

// Самодельный барьер для C++17
class CustomBarrier {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
    int total;
    int generation = 0;
    
public:
    CustomBarrier(int n) : count(n), total(n) {}
    
    void arrive_and_wait() {
        std::unique_lock<std::mutex> lock(mtx);
        int gen = generation;
        
        if (--count == 0) {
            generation++;
            count = total;
            cv.notify_all();
        } else {
            cv.wait(lock, [this, gen]() { return gen != generation; });
        }
    }
};

// 1. Тест с использованием Mutex
void test_mutex(int num_threads, int iterations) {
    std::mutex mtx;
    std::vector<std::thread> threads;
    std::atomic<int> counter{0};
    std::atomic<int> progress{0};
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(33, 126); // Печатные ASCII символы
            
            for (int j = 0; j < iterations; ++j) {
                std::lock_guard<std::mutex> lock(mtx);
                char c = static_cast<char>(dis(gen));
                int value = static_cast<int>(c) * (j % 256);
                counter += value % 256;
                progress++;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    if (num_threads * iterations < 1000) {
        std::cout << "  [Mutex] Завершено операций: " << progress.load() 
                  << ", итоговое значение: " << counter.load() << std::endl;
    }
}

// 2. Тест с использованием Semaphore
void test_semaphore(int num_threads, int iterations) {
    CustomSemaphore semaphore(1);
    std::vector<std::thread> threads;
    std::atomic<int> counter{0};
    std::atomic<int> progress{0};
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(33, 126);
            
            for (int j = 0; j < iterations; ++j) {
                semaphore.acquire();
                char c = static_cast<char>(dis(gen));
                int value = static_cast<int>(c) * (j % 256);
                counter += value % 256;
                progress++;
                semaphore.release();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    if (num_threads * iterations < 1000) {
        std::cout << "  [Semaphore] Завершено операций: " << progress.load() 
                  << ", итоговое значение: " << counter.load() << std::endl;
    }
}

// 3. Тест с использованием Barrier
void test_barrier(int num_threads, int iterations) {
    CustomBarrier sync_point(num_threads);
    std::vector<std::thread> threads;
    std::atomic<int> counter{0};
    std::atomic<int> progress{0};
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(33, 126);
            
            for (int j = 0; j < iterations; ++j) {
                char c = static_cast<char>(dis(gen));
                int value = static_cast<int>(c) * (j % 256);
                counter += value % 256;
                progress++;
                
                // Синхронизация в барьере
                sync_point.arrive_and_wait();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    if (num_threads * iterations < 1000) {
        std::cout << "  [Barrier] Завершено операций: " << progress.load() 
                  << ", итоговое значение: " << counter.load() << std::endl;
    }
}

// 4. Тест с использованием SpinLock
void test_spinlock(int num_threads, int iterations) {
    SpinLock spinlock;
    std::vector<std::thread> threads;
    std::atomic<int> counter{0};
    std::atomic<int> progress{0};
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(33, 126);
            
            for (int j = 0; j < iterations; ++j) {
                std::lock_guard<SpinLock> lock(spinlock);
                char c = static_cast<char>(dis(gen));
                int value = static_cast<int>(c) * (j % 256);
                counter += value % 256;
                progress++;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    if (num_threads * iterations < 1000) {
        std::cout << "  [SpinLock] Завершено операций: " << progress.load() 
                  << ", итоговое значение: " << counter.load() << std::endl;
    }
}

// 5. Тест с использованием SpinWait
void test_spinwait(int num_threads, int iterations) {
    SpinWait spinwait;
    std::vector<std::thread> threads;
    std::atomic<int> counter{0};
    std::atomic<int> progress{0};
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(33, 126);
            
            for (int j = 0; j < iterations; ++j) {
                std::lock_guard<SpinWait> lock(spinwait);
                char c = static_cast<char>(dis(gen));
                int value = static_cast<int>(c) * (j % 256);
                counter += value % 256;
                progress++;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    if (num_threads * iterations < 1000) {
        std::cout << "  [SpinWait] Завершено операций: " << progress.load() 
                  << ", итоговое значение: " << counter.load() << std::endl;
    }
}

// 6. Тест с использованием Monitor
void test_monitor(int num_threads, int iterations) {
    Monitor monitor;
    std::vector<std::thread> threads;
    std::atomic<int> counter{0};
    std::atomic<int> progress{0};
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(33, 126);
            
            for (int j = 0; j < iterations; ++j) {
                monitor.enter();
                char c = static_cast<char>(dis(gen));
                int value = static_cast<int>(c) * (j % 256);
                counter += value % 256;
                progress++;
                monitor.exit();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    if (num_threads * iterations < 1000) {
        std::cout << "  [Monitor] Завершено операций: " << progress.load() 
                  << ", итоговое значение: " << counter.load() << std::endl;
    }
}

void benchmark_all_primitives(int num_threads, int iterations) {
    std::cout << "\n=== Тестирование примитивов синхронизации ===\n";
    std::cout << "Параметры: " << num_threads << " потоков, " 
              << iterations << " итераций на поток\n";
    std::cout << "Общее количество операций: " << num_threads * iterations << "\n\n";
    
    std::vector<std::pair<std::string, double>> results;
    
    {
        Benchmark b("Mutex тест", false);
        test_mutex(num_threads, iterations);
        results.emplace_back("Mutex", b.elapsed_microseconds());
    }
    
    {
        Benchmark b("Semaphore тест", false);
        test_semaphore(num_threads, iterations);
        results.emplace_back("Semaphore", b.elapsed_microseconds());
    }
    
    {
        Benchmark b("Barrier тест", false);
        test_barrier(num_threads, iterations);
        results.emplace_back("Barrier", b.elapsed_microseconds());
    }
    
    {
        Benchmark b("SpinLock тест", false);
        test_spinlock(num_threads, iterations);
        results.emplace_back("SpinLock", b.elapsed_microseconds());
    }
    
    {
        Benchmark b("SpinWait тест", false);
        test_spinwait(num_threads, iterations);
        results.emplace_back("SpinWait", b.elapsed_microseconds());
    }
    
    {
        Benchmark b("Monitor тест", false);
        test_monitor(num_threads, iterations);
        results.emplace_back("Monitor", b.elapsed_microseconds());
    }
    
    Benchmark::print_results(results, "Сравнение примитивов синхронизации");
    Benchmark::save_to_csv(results, "primitives_benchmark.csv");
    Benchmark::print_statistics(results);
}

void run_scalability_test() {
    std::cout << "\n=== Тест масштабируемости ===\n";
    std::cout << "Изучаем производительность при разном количестве потоков\n\n";
    
    std::vector<int> thread_counts = {1, 2, 4, 8};
    const int iterations = 1000;
    
    std::cout << "Фиксированное количество итераций на поток: " << iterations << "\n";
    std::cout << "Тестируем примитив: Mutex (как пример)\n\n";
    
    std::vector<std::pair<std::string, double>> scalability_results;
    
    for (int threads : thread_counts) {
        Benchmark b("Масштабируемость: " + std::to_string(threads) + " потоков", false);
        test_mutex(threads, iterations);
        double time = b.elapsed_microseconds();
        scalability_results.emplace_back(std::to_string(threads) + " потоков", time);
    }
    
    std::cout << "\nРезультаты масштабируемости:\n";
    std::cout << std::setw(15) << std::left << "Потоки"
              << std::setw(15) << "Время (мкс)"
              << std::setw(15) << "Ускорение" << "\n";
    std::cout << std::string(45, '-') << std::endl;
    
    double base_time = scalability_results[0].second;
    for (const auto& result : scalability_results) {
        double speedup = base_time / result.second;
        std::cout << std::setw(15) << std::left << result.first
                  << std::setw(15) << std::fixed << std::setprecision(2) << result.second
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup << "x\n";
    }
    std::cout << std::string(45, '-') << std::endl;
}

void run_extended_benchmark() {
    std::cout << "\n=== Расширенный бенчмарк примитивов синхронизации ===\n";
    std::cout << "Выполняем тесты с разными параметрами\n\n";
    
    std::vector<int> thread_options = {2, 4, 8};
    std::vector<int> iteration_options = {100, 500, 1000};
    
    std::vector<std::pair<std::string, double>> all_results;
    
    for (int threads : thread_options) {
        for (int iterations : iteration_options) {
            std::cout << "\n--- Конфигурация: " << threads << " потоков, " 
                      << iterations << " итераций ---\n";
            
            {
                Benchmark b("Mutex", false);
                test_mutex(threads, iterations);
                all_results.emplace_back(
                    "Mutex_" + std::to_string(threads) + "t_" + std::to_string(iterations) + "i",
                    b.elapsed_microseconds()
                );
            }
            
            {
                Benchmark b("Semaphore", false);
                test_semaphore(threads, iterations);
                all_results.emplace_back(
                    "Semaphore_" + std::to_string(threads) + "t_" + std::to_string(iterations) + "i",
                    b.elapsed_microseconds()
                );
            }
            
            // Для ускорения тестирования, остальные примитивы можно тестировать
            // только при определенных конфигурациях
            if (threads == 4 && iterations == 500) {
                {
                    Benchmark b("Barrier", false);
                    test_barrier(threads, iterations);
                    all_results.emplace_back(
                        "Barrier_" + std::to_string(threads) + "t_" + std::to_string(iterations) + "i",
                        b.elapsed_microseconds()
                    );
                }
                
                {
                    Benchmark b("SpinLock", false);
                    test_spinlock(threads, iterations);
                    all_results.emplace_back(
                        "SpinLock_" + std::to_string(threads) + "t_" + std::to_string(iterations) + "i",
                        b.elapsed_microseconds()
                    );
                }
            }
        }
    }
    
    Benchmark::save_to_csv(all_results, "extended_benchmark.csv");
    std::cout << "\nРасширенный бенчмарк завершен. Результаты сохранены в extended_benchmark.csv\n";
}

void run_race() {
    std::cout << "\n=== Задание 1: Параллельная гонка с ASCII символами ===\n";
    std::cout << "Сравнение 6 примитивов синхронизации:\n";
    std::cout << "1. Mutex (взаимное исключение)\n";
    std::cout << "2. Semaphore (семафор)\n";
    std::cout << "3. Barrier (барьер)\n";
    std::cout << "4. SpinLock (спин-блокировка)\n";
    std::cout << "5. SpinWait (ожидание с уступкой)\n";
    std::cout << "6. Monitor (монитор)\n\n";
    
    int choice;
    std::cout << "Выберите режим тестирования:\n";
    std::cout << "1. Стандартный тест (все примитивы с заданными параметрами)\n";
    std::cout << "2. Тест масштабируемости\n";
    std::cout << "3. Расширенный бенчмарк\n";
    std::cout << "Ваш выбор: ";
    std::cin >> choice;
    
    switch (choice) {
        case 1: {
            int num_threads, iterations;
            
            std::cout << "\nВведите количество потоков (1-16): ";
            std::cin >> num_threads;
            
            std::cout << "Введите количество итераций на поток (100-10000): ";
            std::cin >> iterations;
            
            if (num_threads < 1 || num_threads > 16 || iterations < 100 || iterations > 10000) {
                std::cout << "Некорректные параметры! Использую значения по умолчанию.\n";
                num_threads = 4;
                iterations = 1000;
            }
            
            benchmark_all_primitives(num_threads, iterations);
            break;
        }
        case 2:
            run_scalability_test();
            break;
        case 3:
            run_extended_benchmark();
            break;
        default:
            std::cout << "Неверный выбор! Запускаю стандартный тест...\n";
            benchmark_all_primitives(4, 1000);
    }
}

} // namespace task1
