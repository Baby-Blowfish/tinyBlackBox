// ui_thread.h
#ifndef UI_H
#define UI_H

#include <pthread.h>
#include <termios.h>

// 프로그램 상태
typedef enum
{
  STATE_STOPPED,
  STATE_RUNNING
} State;

// UI Thread용 컨텍스트 구조체
typedef struct
{
  State *state;                // 프로그램 상태 포인터
  pthread_mutex_t *mutex;        // 상태 동기화용 mutex
  pthread_cond_t *cond;        // 상태 동기화용 condvar
  int *menu_sel;               // 메뉴 선택 인덱스 포인터 (테스트용)
  void (*reset_capture)(void); // 재시작 시 호출할 리셋 콜백
  struct termios orig_tio;     // 터미널 원복용 저장
} UiArgs;

// UI 쓰레드 시작: 성공 시 0 반환
int ui_run(UiArgs *arg, pthread_t *tid);
// UI 종료 시 터미널 설정 원복
void ui_thread_cleanup(UiArgs *ctx);

#endif // UI_H