#include <iostream>
#include "task1_race.h"
#include "task2_employees.h"
#include "task3_philosophers.h"
#include "benchmark_utils.h"  

void print_header() {
    std::cout << "         Вариант 26 \n";
}

void print_menu() {
    std::cout << "\n=== ГЛАВНОЕ МЕНЮ ===\n";
    std::cout << "1. Задание 1: Сравнение примитивов синхронизации\n";
    std::cout << "2. Задание 2: Анализ сотрудников (вариант 26)\n";
    std::cout << "3. Задание 3: Обедающие философы\n";
    std::cout << "4. Запустить все тесты производительности\n";
    std::cout << "5. Экспорт всех результатов бенчмарка\n";
    std::cout << "0. Выход\n";
    std::cout << "=============================================\n";
}

void run_all_benchmarks() {
    std::cout << "\n=== ЗАПУСК ВСЕХ ТЕСТОВ ПРОИЗВОДИТЕЛЬНОСТИ ===\n";
    std::cout << "Это может занять несколько минут...\n\n";
    
    // Тест 1: Примитивы синхронизации
    {
        std::cout << "\n[1/3] Тестирование примитивов синхронизации...\n";
        task1::run_extended_benchmark();
    }
    
    // Тест 2: Анализ сотрудников
    {
        std::cout << "\n[2/3] Бенчмарк анализа сотрудников...\n";
        task2::run_employees_benchmark();
    }
    
    // Тест 3: Обедающие философы
    {
        std::cout << "\n[3/3] Бенчмарк обедающих философов...\n";
        task3::run_philosophers_benchmark();
    }
    
    std::cout << "\n=== ВСЕ ТЕСТЫ ЗАВЕРШЕНЫ ===\n";
    std::cout << "Созданные файлы:\n";
    std::cout << "1. primitives_benchmark.csv\n";
    std::cout << "2. extended_benchmark.csv\n";
    std::cout << "3. employees_benchmark.csv\n";
    std::cout << "4. philosophers_benchmark.csv\n\n";
}

void export_all_results() {
    std::cout << "\n=== ЭКСПОРТ РЕЗУЛЬТАТОВ БЕНЧМАРКА ===\n";
    std::cout << "Генерация тестовых данных...\n\n";
    
    std::vector<std::pair<std::string, double>> sample_data = {
        {"Mutex_4t_1000i", 1250.5},
        {"Semaphore_4t_1000i", 1450.2},
        {"Barrier_4t_500i", 2100.8},
        {"SpinLock_4t_1000i", 980.3},
        {"Однопоточная_10000", 4550.7},
        {"Многопоточная_10000_4п", 1250.9},
        {"Философы_5", 3250.1},
        {"Философы_10", 6250.4}
    };
    
    Benchmark::save_to_csv(sample_data, "all_benchmark_results.csv");
    
    std::cout << "\nПример команд для анализа CSV файлов:\n";
    std::cout << "1. Откройте файлы в Excel или Google Sheets\n";
    std::cout << "2. Используйте фильтрацию и сортировку\n";
    std::cout << "3. Постройте графики для визуализации\n";
    std::cout << "4. Сравните производительность разных подходов\n\n";
}

int main() {
    print_header();
    
    int choice;
    
    do {
        print_menu();
        std::cout << "Ваш выбор: ";
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                task1::run_race();
                break;
            case 2:
                task2::run_employees();
                break;
            case 3:
                task3::run_philosophers();
                break;
            case 4:
                run_all_benchmarks();
                break;
            case 5:
                export_all_results();
                break;
            case 0:
                std::cout << "\nВыход из программы...\n";
                break;
            default:
                std::cout << "\nНеверный выбор! Пожалуйста, введите число от 0 до 5.\n";
        }
        
        if (choice != 0) {
            std::cout << "\nНажмите Enter для продолжения...";
            std::cin.ignore();
            std::cin.get();
        }
        
    } while (choice != 0);
    
    std::cout << "   Результаты сохранены в CSV файлах\n";
    
    return 0;
}
