# ===== 공통 설정 =====
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDLIBS = -pthread

CXX = g++
GTEST_DIR = /usr/src/gtest
GTEST_INC = $(GTEST_DIR)/include
GTEST_BUILD_DIR = $(GTEST_DIR)/build
GTEST_LIB = $(GTEST_BUILD_DIR)/lib/libgtest.a $(GTEST_BUILD_DIR)/lib/libgtest_main.a
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -I$(GTEST_INC)

SRC_DIR = src
TEST_DIR = test
BIN_DIR = bin

# ===== 소스 및 테스트 파일 =====
SOURCES = $(wildcard $(SRC_DIR)/*.c)
SOURCES_TEST = $(filter-out $(SRC_DIR)/main.c, $(SOURCES))
TEST_CPP = $(wildcard $(TEST_DIR)/*.cpp)


# 테스트 타겟은 반드시 실행용 main.c를 제외
TARGET = $(BIN_DIR)/tinyBlackBox
TEST_TARGET = $(BIN_DIR)/tinyBlackBox_test

# ===== 빌드 타겟 =====
all: $(BIN_DIR) $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

test: $(BIN_DIR) $(TEST_CPP) $(SOURCES_TEST)
	$(CXX) $(CXXFLAGS) -pthread -o $(TEST_TARGET) $(TEST_CPP) $(SOURCES_TEST) $(GTEST_LIB)

run: $(TARGET)
	./$(TARGET)

debug:
	$(CC) $(CFLAGS) -g -o $(TARGET) $(SOURCES) $(LDLIBS)

lint:
	cppcheck --enable=all --inconclusive --std=c99 --language=c \
	-Iinclude -I/usr/include -I/usr/include/x86_64-linux-gnu \
	src include


clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: all test run debug clean

