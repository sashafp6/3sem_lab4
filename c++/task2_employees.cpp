#include "task2_employees.h"
#include "benchmark_utils.h"
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <algorithm>
#include <mutex>
#include <iomanip>
#include <cmath>
#include <sstream>

namespace task2 {

std::vector<Employee> generate_employees(int count, const std::string& target_position) {
    std::vector<Employee> employees;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Списки для генерации данных
    std::vector<std::string> first_names = {"Иван", "Петр", "Сергей", "Алексей", "Дмитрий", 
                                           "Мария", "Ольга", "Елена", "Анна", "Наталья"};
    std::vector<std::string> last_names = {"Иванов", "Петров", "Сидоров", "Смирнов", "Кузнецов",
                                          "Попов", "Васильев", "Павлов", "Семенов", "Федоров"};
    std::vector<std::string> middle_names = {"Иванович", "Петрович", "Сергеевич", "Алексеевич", 
                                            "Дмитриевич", "Ивановна", "Петровна", "Сергеевна", 
                                            "Алексеевна", "Дмитриевна"};
    
    std::vector<std::string> positions = {"Менеджер", "Разработчик", "Аналитик", "Тестировщик", 
                                         "Дизайнер", "Администратор", "Бухгалтер", target_position};
    
    std::uniform_int_distribution<> age_dist(20, 65);
    std::uniform_real_distribution<> salary_dist(30000, 300000);
    std::uniform_int_distribution<> position_dist(0, positions.size() - 1);
    
    for (int i = 0; i < count; ++i) {
        // Генерация ФИО
        std::ostringstream name;
        name << last_names[gen() % last_names.size()] << " "
             << first_names[gen() % first_names.size()] << " "
             << middle_names[gen() % middle_names.size()];
        
        // Генерация должности
        std::string position = positions[position_dist(gen)];
        
        // Генерация возраста
        int age = age_dist(gen);
        
        // Генерация зарплаты
        double salary = salary_dist(gen);
        
        employees.emplace_back(name.str(), position, age, salary);
    }
    
    // Убедимся, что есть сотрудники с целевой должностью
    bool has_target_position = false;
    for (const auto& emp : employees) {
        if (emp.position == target_position) {
            has_target_position = true;
            break;
        }
    }
    
    if (!has_target_position && !employees.empty()) {
        employees[0].position = target_position;
    }
    
    return employees;
}

double calculate_average_age(const std::vector<Employee>& employees, const std::string& target_position) {
    double total_age = 0.0;
    int count = 0;
    
    for (const auto& emp : employees) {
        if (emp.position == target_position) {
            total_age += emp.age;
            count++;
        }
    }
    
    return count > 0 ? total_age / count : 0.0;
}

double find_max_salary_near_average(const std::vector<Employee>& employees, 
                                   const std::string& target_position, 
                                   double average_age, 
                                   int age_range) {
    double max_salary = 0.0;
    
    for (const auto& emp : employees) {
        if (emp.position == target_position && 
            std::abs(emp.age - average_age) <= age_range) {
            if (emp.salary > max_salary) {
                max_salary = emp.salary;
            }
        }
    }
    
    return max_salary;
}

void process_single_thread(const std::vector<Employee>& employees, 
                          const std::string& target_position) {
    // Расчет среднего возраста
    double average_age = calculate_average_age(employees, target_position);
    
    // Поиск максимальной зарплаты
    double max_salary = find_max_salary_near_average(employees, target_position, average_age);
    
    // Подсчет количества сотрудников с целевой должностью
    int target_count = 0;
    for (const auto& emp : employees) {
        if (emp.position == target_position) {
            target_count++;
        }
    }
    
    std::cout << "\n=== Результаты обработки (однопоточная) ===\n";
    std::cout << "Всего сотрудников: " << employees.size() << "\n";
    std::cout << "Сотрудников с должностью '" << target_position << "': " << target_count << "\n\n";
    
    if (target_count > 0) {
        std::cout << "Средний возраст: " << std::fixed << std::setprecision(2) << average_age << " лет\n";
        std::cout << "Максимальная зарплата среди сотрудников\n";
        std::cout << "с возрастом ±2 года от среднего: " 
                  << std::fixed << std::setprecision(2) << max_salary << " руб.\n";
    } else {
        std::cout << "Нет сотрудников с должностью '" << target_position << "'\n";
    }
}

void process_multi_thread(const std::vector<Employee>& employees, 
                         const std::string& target_position, 
                         int num_threads) {
    if (employees.empty()) {
        std::cout << "Нет данных для обработки\n";
        return;
    }
    
    std::vector<std::thread> threads;
    std::vector<double> thread_ages(num_threads, 0.0);
    std::vector<int> thread_counts(num_threads, 0);
    std::vector<double> thread_max_salaries(num_threads, 0.0);
    
    int chunk_size = employees.size() / num_threads;
    std::mutex cout_mutex;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            int start = i * chunk_size;
            int end = (i == num_threads - 1) ? employees.size() : start + chunk_size;
            
            for (int j = start; j < end; ++j) {
                const auto& emp = employees[j];
                if (emp.position == target_position) {
                    thread_ages[i] += emp.age;
                    thread_counts[i]++;
                    
                    // Пока не знаем средний возраст, сохраняем максимальную зарплату
                    if (emp.salary > thread_max_salaries[i]) {
                        thread_max_salaries[i] = emp.salary;
                    }
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Агрегируем результаты
    double total_age = 0.0;
    int total_count = 0;
    
    for (int i = 0; i < num_threads; ++i) {
        total_age += thread_ages[i];
        total_count += thread_counts[i];
    }
    
    double average_age = total_count > 0 ? total_age / total_count : 0.0;
    
    // Вторая фаза: поиск максимальной зарплаты с учетом среднего возраста
    threads.clear();
    std::vector<double> thread_phase2_max(num_threads, 0.0);
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i, average_age]() {
            int start = i * chunk_size;
            int end = (i == num_threads - 1) ? employees.size() : start + chunk_size;
            
            for (int j = start; j < end; ++j) {
                const auto& emp = employees[j];
                if (emp.position == target_position && 
                    std::abs(emp.age - average_age) <= 2) {
                    if (emp.salary > thread_phase2_max[i]) {
                        thread_phase2_max[i] = emp.salary;
                    }
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    double max_salary = 0.0;
    for (int i = 0; i < num_threads; ++i) {
        if (thread_phase2_max[i] > max_salary) {
            max_salary = thread_phase2_max[i];
        }
    }
    
    std::cout << "\n=== Результаты обработки (многопоточная) ===\n";
    std::cout << "Использовано потоков: " << num_threads << "\n";
    std::cout << "Всего сотрудников: " << employees.size() << "\n";
    std::cout << "Сотрудников с должностью '" << target_position << "': " << total_count << "\n\n";
    
    if (total_count > 0) {
        std::cout << "Средний возраст: " << std::fixed << std::setprecision(2) << average_age << " лет\n";
        std::cout << "Максимальная зарплата среди сотрудников\n";
        std::cout << "с возрастом ±2 года от среднего: " 
                  << std::fixed << std::setprecision(2) << max_salary << " руб.\n";
    } else {
        std::cout << "Нет сотрудников с должностью '" << target_position << "'\n";
    }
}

void analyze_performance(int min_size, int max_size, int step, 
                        const std::string& target_position) {
    std::cout << "\n=== Анализ производительности ===\n";
    std::cout << "Тестируем обработку разных объемов данных\n";
    std::cout << "Целевая должность: '" << target_position << "'\n\n";
    
    std::vector<std::pair<std::string, double>> single_thread_results;
    std::vector<std::pair<std::string, double>> multi_thread_results;
    
    for (int size = min_size; size <= max_size; size += step) {
        std::cout << "Тест с " << size << " сотрудниками...\n";
        
        auto employees = generate_employees(size, target_position);
        
        double single_time, multi_time;
        
        {
            Benchmark b("Однопоточная", false);
            process_single_thread(employees, target_position);
            single_time = b.elapsed_microseconds();
        }
        
        {
            Benchmark b("Многопоточная (4 потока)", false);
            process_multi_thread(employees, target_position, 4);
            multi_time = b.elapsed_microseconds();
        }
        
        single_thread_results.emplace_back(std::to_string(size), single_time);
        multi_thread_results.emplace_back(std::to_string(size), multi_time);
        
        double speedup = single_time / multi_time;
        std::cout << "  Ускорение: " << std::fixed << std::setprecision(2) << speedup << "x\n\n";
    }
    
    std::cout << "\n=== Итоги анализа производительности ===\n";
    std::cout << std::setw(10) << "Размер" 
              << std::setw(20) << "Однопоточная (мс)"
              << std::setw(20) << "Многопоточная (мс)"
              << std::setw(15) << "Ускорение" << "\n";
    std::cout << std::string(65, '-') << std::endl;
    
    for (size_t i = 0; i < single_thread_results.size(); ++i) {
        double single_ms = single_thread_results[i].second / 1000.0;
        double multi_ms = multi_thread_results[i].second / 1000.0;
        double speedup = single_thread_results[i].second / multi_thread_results[i].second;
        
        std::cout << std::setw(10) << single_thread_results[i].first
                  << std::setw(20) << std::fixed << std::setprecision(2) << single_ms
                  << std::setw(20) << std::fixed << std::setprecision(2) << multi_ms
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup << "x\n";
    }
    std::cout << std::string(65, '-') << std::endl;
}

void run_employees_benchmark() {
    std::cout << "\n=== Бенчмарк анализа сотрудников (вариант 26) ===\n";
    
    std::string target_position = "Инженер";
    std::vector<int> test_sizes = {1000, 5000, 10000, 50000, 100000};
    std::vector<int> thread_counts = {1, 2, 4, 8};
    
    std::vector<std::pair<std::string, double>> benchmark_results;
    
    for (int size : test_sizes) {
        std::cout << "\nГенерация " << size << " сотрудников...\n";
        auto employees = generate_employees(size, target_position);
        
        for (int threads : thread_counts) {
            std::string test_name = std::to_string(size) + "_сотр_" + std::to_string(threads) + "_потоков";
            
            Benchmark b(test_name, false);
            if (threads == 1) {
                process_single_thread(employees, target_position);
            } else {
                process_multi_thread(employees, target_position, threads);
            }
            
            benchmark_results.emplace_back(test_name, b.elapsed_microseconds());
        }
    }
    
    Benchmark::save_to_csv(benchmark_results, "employees_benchmark.csv");
    std::cout << "\nБенчмарк завершен. Результаты сохранены в employees_benchmark.csv\n";
}

void run_employees() {
    std::cout << "\n=== Задание 2: Анализ сотрудников (вариант 26) ===\n";
    std::cout << "Найти средний возраст для должности Д\n";
    std::cout << "и наибольшую зарплату среди сотрудников должности Д,\n";
    std::cout << "чья возраст отличается от среднего не более чем на 2 года.\n\n";
    
    int choice;
    std::cout << "Выберите режим:\n";
    std::cout << "1. Стандартный анализ\n";
    std::cout << "2. Анализ производительности\n";
    std::cout << "3. Полный бенчмарк\n";
    std::cout << "Ваш выбор: ";
    std::cin >> choice;
    
    std::string target_position;
    std::cout << "\nВведите целевую должность (Д): ";
    std::cin.ignore();
    std::getline(std::cin, target_position);
    
    if (target_position.empty()) {
        target_position = "Инженер";
    }
    
    switch (choice) {
        case 1: {
            int num_employees, num_threads;
            
            std::cout << "\nВведите количество сотрудников (100-100000): ";
            std::cin >> num_employees;
            
            if (num_employees < 100) num_employees = 100;
            if (num_employees > 100000) num_employees = 100000;
            
            std::cout << "Генерация " << num_employees << " сотрудников...\n";
            auto employees = generate_employees(num_employees, target_position);
            
            double single_time, multi_time;
            
            {
                Benchmark b("Однопоточная обработка");
                process_single_thread(employees, target_position);
                single_time = b.elapsed_microseconds();
            }
            
            std::cout << "\nВведите количество потоков для многопоточной обработки (2-16): ";
            std::cin >> num_threads;
            
            if (num_threads < 2) num_threads = 2;
            if (num_threads > 16) num_threads = 16;
            
            {
                Benchmark b("Многопоточная обработка");
                process_multi_thread(employees, target_position, num_threads);
                multi_time = b.elapsed_microseconds();
            }
            
            Benchmark::print_comparison("Однопоточная", single_time, 
                                       "Многопоточная (" + std::to_string(num_threads) + " потоков)", 
                                       multi_time);
            break;
        }
        case 2:
            analyze_performance(1000, 10000, 2000, target_position);
            break;
        case 3:
            run_employees_benchmark();
            break;
        default:
            std::cout << "Неверный выбор! Запускаю стандартный анализ...\n";
            auto employees = generate_employees(5000, target_position);
            process_single_thread(employees, target_position);
            process_multi_thread(employees, target_position, 4);
    }
}

} // namespace task2
