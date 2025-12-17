#ifndef BENCHMARK_UTILS_H
#define BENCHMARK_UTILS_H

#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <cmath>

class Benchmark {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::string benchmark_name;
    bool verbose;
    
public:
    Benchmark(const std::string& name, bool verbose_mode = true) 
        : benchmark_name(name), verbose(verbose_mode) {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    ~Benchmark() {
        if (verbose) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            std::cout << "[" << benchmark_name << "] Время выполнения: " 
                      << duration.count() << " мкс (" 
                      << duration.count() / 1000.0 << " мс)" << std::endl;
        }
    }
    
    double elapsed_microseconds() const {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time);
        return static_cast<double>(duration.count());
    }
    
    double elapsed_milliseconds() const {
        return elapsed_microseconds() / 1000.0;
    }
    
    double elapsed_seconds() const {
        return elapsed_microseconds() / 1000000.0;
    }
    
    static void print_results(const std::vector<std::pair<std::string, double>>& results, 
                             const std::string& title = "Результаты бенчмарка") {
        std::cout << "\n=== " << title << " ===\n";
        std::cout << std::setw(20) << std::left << "Тест" 
                  << std::setw(15) << "Время (мкс)"
                  << std::setw(15) << "Время (мс)" << "\n";
        std::cout << std::string(50, '-') << std::endl;
        
        for (const auto& result : results) {
            std::cout << std::setw(20) << std::left << result.first 
                      << std::setw(15) << std::fixed << std::setprecision(2) << result.second
                      << std::setw(15) << std::fixed << std::setprecision(4) << result.second / 1000.0 
                      << std::endl;
        }
        std::cout << std::string(50, '=') << "\n" << std::endl;
    }
    
    static void print_comparison(const std::string& test1_name, double time1,
                                const std::string& test2_name, double time2) {
        std::cout << "\n=== Сравнение производительности ===\n";
        std::cout << std::setw(25) << std::left << "Метод"
                  << std::setw(15) << "Время (мс)"
                  << std::setw(15) << "Ускорение" << "\n";
        std::cout << std::string(55, '-') << std::endl;
        
        double speedup = time1 / time2;
        std::cout << std::setw(25) << std::left << test1_name
                  << std::setw(15) << std::fixed << std::setprecision(3) << time1 / 1000.0
                  << std::setw(15) << "1.00x" << std::endl;
        
        std::cout << std::setw(25) << std::left << test2_name
                  << std::setw(15) << std::fixed << std::setprecision(3) << time2 / 1000.0
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
        std::cout << std::string(55, '-') << std::endl;
    }
    
    static void save_to_csv(const std::vector<std::pair<std::string, double>>& results,
                           const std::string& filename = "benchmark_results.csv") {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Ошибка: не удалось создать файл " << filename << std::endl;
            return;
        }
        
        file << "Тест,Время(микросекунды),Время(миллисекунды),Время(секунды)\n";
        
        for (const auto& result : results) {
            file << result.first << ","
                 << result.second << ","
                 << result.second / 1000.0 << ","
                 << result.second / 1000000.0 << "\n";
        }
        
        file.close();
        std::cout << "Результаты сохранены в файл: " << filename << std::endl;
    }
    
    static void print_statistics(const std::vector<std::pair<std::string, double>>& results) {
        if (results.empty()) return;
        
        double min_time = results[0].second;
        double max_time = results[0].second;
        double sum = 0;
        std::string fastest = results[0].first;
        std::string slowest = results[0].first;
        
        for (const auto& result : results) {
            double time = result.second;
            sum += time;
            
            if (time < min_time) {
                min_time = time;
                fastest = result.first;
            }
            
            if (time > max_time) {
                max_time = time;
                slowest = result.first;
            }
        }
        
        double avg = sum / results.size();
        
        // Вычисляем стандартное отклонение
        double variance = 0;
        for (const auto& result : results) {
            variance += std::pow(result.second - avg, 2);
        }
        variance /= results.size();
        double stddev = std::sqrt(variance);
        
        std::cout << "\n=== Статистика бенчмарка ===\n";
        std::cout << "Количество тестов: " << results.size() << "\n";
        std::cout << "Среднее время: " << avg << " мкс (" << avg/1000.0 << " мс)\n";
        std::cout << "Минимальное время: " << min_time << " мкс (" << fastest << ")\n";
        std::cout << "Максимальное время: " << max_time << " мкс (" << slowest << ")\n";
        std::cout << "Стандартное отклонение: " << stddev << " мкс\n";
        std::cout << "Разброс: " << (max_time - min_time) << " мкс ("
                  << std::fixed << std::setprecision(1) 
                  << ((max_time - min_time) / min_time * 100) << "%)\n";
    }
};

#endif // BENCHMARK_UTILS_H