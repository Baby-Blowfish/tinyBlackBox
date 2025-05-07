# ===== 컴파일러 설정 =====
CC      := gcc
CFLAGS  := -Wall -Wextra -Iinclude
LDLIBS  := -pthread

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

# ===== 기본/테스트/클린 타겟 =====
.PHONY: all test clean

all: $(TARGET)

# ─── 메인 바이너리 빌드 (전체 src/*.c) ─────────────────
$(TARGET): $(SRC_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# ─── 테스트 바이너리 빌드 (Check) ─────────────────────
# test_frame.c + frame.c + frame_pool.c -> bin/test_frame
$(TEST_TARGET): $(TEST_DIR)/test_frame.c $(FRAME_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -lcheck -lm -lrt -lsubunit -pthread

# ─── 테스트 실행 ───────────────────────────────────────
test: $(TEST_TARGET)
	@echo "=== Running frame module tests ==="
	./$(TEST_TARGET)

# ─── 클린 업 ───────────────────────────────────────────
clean:
	rm -rf $(BIN_DIR)
