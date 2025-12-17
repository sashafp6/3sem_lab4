package main

import (
	"bufio"
	"fmt"
	"os"
)

func printHeader() {
	fmt.Println("         Вариант 26")
}

func printMenu() {
	fmt.Println("\n=== ГЛАВНОЕ МЕНЮ ===")
	fmt.Println("1. Задание 1: Сравнение примитивов синхронизации")
	fmt.Println("2. Задание 2: Анализ сотрудников (вариант 26)")
	fmt.Println("3. Задание 3: Обедающие философы")
	fmt.Println("4. Запустить все тесты производительности")
	fmt.Println("5. Экспорт всех результатов бенчмарка")
	fmt.Println("0. Выход")
	fmt.Println("=============================================")
}

func runAllBenchmarks() {
	fmt.Println("\n=== ЗАПУСК ВСЕХ ТЕСТОВ ПРОИЗВОДИТЕЛЬНОСТИ ===")
	fmt.Println("Это может занять несколько минут...\n")

	// Тест 1: Примитивы синхронизации
	fmt.Println("\n[1/3] Тестирование примитивов синхронизации...")
	RunExtendedBenchmark()

	// Тест 2: Анализ сотрудников
	fmt.Println("\n[2/3] Бенчмарк анализа сотрудников...")
	RunEmployeesBenchmark()

	// Тест 3: Обедающие философы
	fmt.Println("\n[3/3] Бенчмарк обедающих философов...")
	RunPhilosophersBenchmark()

	fmt.Println("\n=== ВСЕ ТЕСТЫ ЗАВЕРШЕНЫ ===")
	fmt.Println("Созданные файлы:")
	fmt.Println("1. primitives_benchmark.csv")
	fmt.Println("2. extended_benchmark.csv")
	fmt.Println("3. employees_benchmark.csv")
	fmt.Println("4. philosophers_benchmark.csv")
	fmt.Println("5. all_benchmark_results.csv")
}

func exportAllResults() {
	fmt.Println("\n=== ЭКСПОРТ РЕЗУЛЬТАТОВ БЕНЧМАРКА ===")

	sampleData := []BenchmarkResult{
		{Name: "Mutex_4t_1000i", TimeMicroseconds: 1250.5},
		{Name: "Semaphore_4t_1000i", TimeMicroseconds: 1450.2},
		{Name: "Barrier_4t_500i", TimeMicroseconds: 2100.8},
		{Name: "SpinLock_4t_1000i", TimeMicroseconds: 980.3},
		{Name: "Однопоточная_10000", TimeMicroseconds: 4550.7},
		{Name: "Многопоточная_10000_4п", TimeMicroseconds: 1250.9},
		{Name: "Философы_5_Мьютексы", TimeMicroseconds: 3250.1},
		{Name: "Философы_10_Семафоры", TimeMicroseconds: 6250.4},
	}

	SaveToCSV(sampleData, "all_benchmark_results.csv")

	fmt.Println("\nПример команд для анализа CSV файлов:")
	fmt.Println("1. Откройте файлы в Excel или Google Sheets")
	fmt.Println("2. Используйте фильтрацию и сортировку")
	fmt.Println("3. Постройте графики для визуализации")
	fmt.Println("4. Сравните производительность разных подходов")
}

func waitForEnter() {
	fmt.Print("\nНажмите Enter для продолжения...")
	bufio.NewReader(os.Stdin).ReadBytes('\n')
}

func main() {
	printHeader()

	scanner := bufio.NewScanner(os.Stdin)

	for {
		printMenu()
		fmt.Print("Ваш выбор: ")

		if !scanner.Scan() {
			break
		}

		choice := scanner.Text()

		switch choice {
		case "1":
			RunRace()
		case "2":
			RunEmployees()
		case "3":
			RunPhilosophers()
		case "4":
			runAllBenchmarks()
		case "5":
			exportAllResults()
		case "0":
			fmt.Println("\nВыход из программы...")
			os.Exit(0)
		default:
			fmt.Println("\nНеверный выбор! Пожалуйста, введите число от 0 до 5.")
		}

		if choice != "0" {
			waitForEnter()
		}
	}

	fmt.Println("\n=============================================")
	fmt.Println("   Лабораторная работа №4 завершена!")
	fmt.Println("   Результаты сохранены в CSV файлах")
	fmt.Println("=============================================\n")
}
