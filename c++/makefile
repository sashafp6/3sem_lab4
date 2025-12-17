CXX = g++
CXXFLAGS = -std=c++17 -pthread -O2 -I. -Wall -Wextra
TARGET = lab4_variant26

SOURCES = main.cpp \
          task1_race.cpp \
          task2_employees.cpp \
          task3_philosophers.cpp \
          benchmark_utils.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

rebuild: clean all

benchmark: $(TARGET)
	@echo "Запуск тестов производительности..."
	@echo -e "1\n4\n1000\n2\nИнженер\n1\n10000\n4\n0\n" | ./$(TARGET)

.PHONY: all clean run rebuild benchmark
