
#define _POSIX_C_SOURCE 200112L

#include "log.h"
#include <stdio.h>
#include <time.h>
#include <string.h>


// 로깅 함수
void log_event(LogLevel level, const char* action, int fd, int count, const char* details) {
    time_t now;
    time(&now);
    char timestamp[26];
    ctime_r(&now, timestamp);
    timestamp[24] = '\0'; // 줄바꿈 문자 제거

    // 콘솔 출력
    const char* level_str;
    switch (level) {
        case LOG_DEBUG:   level_str = "DEBUG"; break;
        case LOG_INFO:    level_str = "INFO"; break;
        case LOG_WARNING: level_str = "WARNING"; break;
        case LOG_ERROR:   level_str = "ERROR"; break;
        default:          level_str = "UNKNOWN"; break;
    }


    FILE* log_file = fopen("logs/ball_operations.log", "a");
    if (log_file) {
        fprintf(log_file, "[%s] [%s] Client FD: %d, Action: %s, Count: %d, Details: %s\n",
                timestamp, level_str, fd, action, count, details);
        fclose(log_file);
    }
}
