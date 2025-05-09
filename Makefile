# ===== 컴파일러 설정 =====
CC      := gcc
CFLAGS  := -Wall -Wextra -Iinclude
LDLIBS  := -pthread

# 디버그 전용 플래그
DEBUG_FLAGS := -g -O0

# ===== 디렉토리 =====
SRC_DIR   := src
TEST_DIR  := test
BIN_DIR   := bin

# ===== 소스 파일 =====
SRC_SRCS    := $(wildcard $(SRC_DIR)/*.c)
FRAME_SRCS  := $(SRC_DIR)/frame.c $(SRC_DIR)/frame_pool.c

# ===== 실행 파일 =====
TARGET       := $(BIN_DIR)/tinyBlackBox
TEST_TARGET  := $(BIN_DIR)/test_frame

# ===== 기본/테스트/클린/디버그 타겟 =====
.PHONY: all test clean debug

all: $(TARGET)

$(TARGET): $(SRC_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(TEST_TARGET): $(TEST_DIR)/test_frame.c $(FRAME_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -lcheck -lm -lrt -lsubunit -pthread

test: $(TEST_TARGET)
	@echo "=== Running frame module tests ==="
	./$(TEST_TARGET)

clean:
	rm -rf $(BIN_DIR)

# ─── 디버그 전용 빌드 ─────────────────────────────────
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all
