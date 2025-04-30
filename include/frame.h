#ifndef FRAME_H
#define FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief   범용 Frame 구조체.
 * @note    depth만큼의 바이트를 pixel data로 사용합니다.
 */
typedef struct Frame {
  size_t width;  ///< 가로 해상도 (px)
  size_t height; ///< 세로 해상도 (px)
  size_t seq;    ///< 시퀀스 번호
  size_t depth;  ///< 픽셀당 바이트 수 (ex: 1=GRAY, 3=RGB)
  void *data;    ///< 픽셀 데이터 (width * height * depth 바이트)
} Frame;

/**
 * @brief   새 Frame 객체를 동적 할당하고 초기화합니다. (Owning)
 * @param   width   [in] 가로 해상도 (px, >0)
 * @param   height  [in] 세로 해상도 (px, >0)
 * @param   depth   [in] 픽셀당 바이트 수 (>0)
 * @return  성공 시 Frame* (NULL on failure; errno 설정)
 */
Frame *frame_create(size_t width, size_t height, size_t depth);

/**
 * @brief   동적 할당된 Frame 객체와 내부 버퍼를 모두 해제합니다.
 * @param   frame   [in] 해제할 Frame 포인터 (NULL safe)
 */
void frame_free(Frame *frame);

/**
 * @brief   Frame 내부 데이터를 초기화하고 버퍼를 할당합니다.
 * @param   frame   [out] 초기화할 Frame 포인터 (NULL 불가)
 * @param   width   [in]  가로 해상도 (px, >0)
 * @param   height  [in]  세로 해상도 (px, >0)
 * @param   depth   [in]  픽셀당 바이트 수 (>0)
 * @return  0: 성공, -1: 실패 (errno 설정)
 * @note    errno = EINVAL/EOVERFLOW/ENOMEM
 */
int frame_init(Frame *frame, size_t width, size_t height, size_t depth);

/**
 * @brief   frame_init()으로 할당된 내부 버퍼를 해제합니다.
 * @param   frame   [in] 초기화된 Frame 포인터 (NULL safe)
 */
void frame_destroy(Frame *frame);

/**
 * @brief   Frame의 픽셀 데이터 버퍼를 반환합니다. (쓰기 가능)
 * @param   frame   [in] Frame 포인터
 * @return  void* (NULL 허용)
 */
void *frame_get_data(Frame *frame);

/**
 * @brief   Frame의 픽셀 데이터 버퍼를 반환합니다. (읽기 전용)
 * @param   frame   [in] Frame 포인터
 * @return  const void* (NULL 허용)
 */
const void *frame_get_data_const(const Frame *frame);

/**
 * @brief   Frame의 가로 해상도(px)를 반환합니다.
 * @param   frame   [in] Frame 포인터
 * @return  가로 해상도 (frame이 NULL이면 0)
 */
size_t frame_get_width(const Frame *frame);

/**
 * @brief   Frame의 세로 해상도(px)를 반환합니다.
 * @param   frame   [in] Frame 포인터
 * @return  세로 해상도 (frame이 NULL이면 0)
 */
size_t frame_get_height(const Frame *frame);

/**
 * @brief   Frame의 픽셀당 바이트 수(depth) 반환합니다.
 */
size_t frame_get_depth(const Frame *frame);

/**
 * @brief   Frame의 시퀸스 번호를 반환합니다.
 * @param   frame   [in] Frame 포인터
 * @return  시퀸스 번호 (frame이 NULL이면 0)
 */
size_t frame_get_seq(const Frame *frame);

/**
 * @brief   Frame 구조체 정보를 출력합니다.
 * @param   frame   [in] Frame 포인터 (NULL 허용)
 * @param   out     [in] 출력 스트림 (NULL이면 stdout 사용)
 */
void frame_dump_info(const Frame *frame, FILE *out);

/**
 * @brief Frame의 픽셀 데이터만 파일 디스크립터에 저장합니다.
 * @param fd      [in] 파일 디스크립터 (쓰기 가능)
 * @param frame   [in] Frame 포인터 (width, height, data 필요)
 * @return >=0: 실제 쓴 바이트 수, -1: 실패 (errno 설정)
 */
ssize_t frame_write_data(int fd, const Frame *frame);

/**
 * @brief 파일 디스크립터에서 픽셀 데이터만 읽어 Frame에 채웁니다.
 * @param fd      [in] 파일 디스크립터 (읽기 가능)
 * @param frame   [out] Frame 포인터 (이미 width, height, data 초기화됨)
 * @return >=0: 실제 읽은 바이트 수, -1: 실패 (errno 설정)
 * @note    반드시 frame_gray_init() 또는 frame_gray_create() 이후 호출해야 함
 */
ssize_t frame_read_data(int fd, Frame *frame);

/**
 * @brief   Frame의 픽셀 데이터를 파일 디스크립터에서 읽습니다. (EOF 시 자동으로
 * 처음부터 다시 읽기)
 * @param   fd      [in] 파일 디스크립터 (읽기 가능)
 * @param   frame   [out] 이미 width, height가 초기화된 Frame 포인터
 * @return  >=0: 실제 읽은 바이트 수, -1: 실패 (errno 설정)
 * @note    EOF에 도달하면 lseek으로 다시 처음부터 읽기를 시도합니다.
 */
ssize_t frame_read_loop(int fd, Frame *frame);

#ifdef __cplusplus
}
#endif
#endif // FRAME_H
