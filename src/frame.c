#include "frame.h"

Frame *frame_create(size_t width, size_t height, DEPTH depth)
{
  Frame *f = malloc(sizeof(*f));
  if (!f)
  {
    errno = ENOMEM;
    return NULL;
  }
  // 내부 초기화
  if (frame_init(f, width, height, depth) != 0)
  {
    free(f);
    return NULL;
  }
  return f;
}

void frame_destroy(Frame *frame)
{
  if (!frame)
    return;
  frame_free(frame);
  free(frame);
}

int frame_init(Frame *frame, size_t width, size_t height, DEPTH depth)
{
  if (!frame || width == 0 || height == 0 || depth == 0)
  {
    errno = EINVAL;
    return -1;
  }

  size_t total;
  if (__builtin_mul_overflow(width, height, &total) || __builtin_mul_overflow(total, depth, &total))
  {
    errno = EOVERFLOW;
    return -1;
  }

  unsigned char *buf = calloc(1, total);
  if (!buf)
  {
    errno = ENOMEM;
    return -1;
  }

  frame->width = width;
  frame->height = height;
  frame->depth = depth;
  frame->seq = 0;
  frame->data = buf;

  return 0;
}

void frame_free(Frame *frame)
{
  if (!frame || !frame->data)
    return;
  free(frame->data);
  frame->data = NULL;
}

/**
 * @brief   Frame의 내부 데이터 버퍼 포인터를 반환합니다. (쓰기 가능)
 * @param   frame [in] 유효한 Frame 포인터
 * @return  버퍼 포인터 (NULL 허용)
 */
void *frame_get_data(Frame *frame)
{
  return frame ? frame->data : NULL;
}

/**
 * @brief   Frame의 내부 데이터 버퍼 포인터를 반환합니다. (읽기 전용)
 * @param   frame [in] 유효한 Frame 포인터
 * @return  const 버퍼 포인터 (NULL 허용)
 */
const void *frame_get_data_const(const Frame *frame)
{
  return frame ? frame->data : NULL;
}

size_t frame_get_width(const Frame *frame)
{
  return frame ? frame->width : 0;
}

size_t frame_get_height(const Frame *frame)
{
  return frame ? frame->height : 0;
}

DEPTH frame_get_depth(const Frame *frame)
{
  return frame ? frame->depth : 0;
}

size_t frame_get_seq(const Frame *frame)
{
  return frame ? frame->seq : 0;
}

/**
 * @brief   Frame 구조체 정보를 출력합니다.
 * @param   frame   [in] 출력할 Frame 포인터 (NULL 허용)
 * @param   out     [in] 출력 대상 스트림 (NULL이면 stdout)
 */
void frame_dump_info(const Frame *frame, FILE *out)
{
  if (!out)
    out = stdout;

  if (!frame)
  {
    fprintf(out, "[Frame] (null)\n");
    return;
  }

  size_t total = frame->width * frame->height * frame->depth;

  fprintf(out, "[Frame]\n");
  fprintf(out, "  Resolution : %zux%zu\n", frame->width, frame->height);
  fprintf(out, "  Depth      : %u bytes\n", frame->depth);
  fprintf(out, "  Sequence   : %zu\n", frame->seq);
  fprintf(out, "  Data Ptr   : %p\n", frame->data);
  fprintf(out, "  Size       : %zu bytes\n", total);
}

static ssize_t full_write(int fd, const void *buf, size_t count)
{
  const char *p = buf;
  size_t written = 0;
  while (written < count)
  {
    ssize_t n = write(fd, p + written, count - written);
    if (n < 0)
    {
      if (errno == EINTR)
        continue;
      return -1;
    }
    written += n;
  }
  return written;
}

static ssize_t full_read(int fd, void *buf, size_t count)
{
  char *p = buf;
  size_t readn = 0;
  while (readn < count)
  {
    ssize_t n = read(fd, p + readn, count - readn);
    if (n < 0)
    {
      if (errno == EINTR)
        continue;
      return -1;
    }
    if (n == 0)
      break; // EOF
    readn += n;
  }
  return (readn == count) ? (ssize_t)readn : -1;
}

ssize_t frame_write_data(int fd, const Frame *frame)
{
  if (!frame || !frame->data)
  {
    errno = EINVAL;
    return -1;
  }

  size_t count = frame->width * frame->height * frame->depth;
  ssize_t ret = full_write(fd, frame->data, count);
  return ret;
}

ssize_t frame_read_data(int fd, Frame *frame)
{
  if (!frame || !frame->data)
  {
    errno = EINVAL;
    return -1;
  }

  size_t count = frame->width * frame->height * frame->depth;
  ssize_t ret = full_read(fd, frame->data, count);
  return ret;
}

ssize_t frame_read_loop(int fd, Frame *frame)
{
  if (!frame || !frame->data)
  {
    errno = EINVAL;
    return -1;
  }

  size_t count = frame->width * frame->height * frame->depth;
  char *buf = (char *)frame->data;
  size_t readn = 0;

  while (readn < count)
  {
    ssize_t n = read(fd, buf + readn, count - readn);
    if (n < 0)
    {
      if (errno == EINTR)
        continue;
      return -1;
    }
    if (n == 0)
    {
      // EOF → 파일 처음으로 이동
      if (lseek(fd, 0, SEEK_SET) < 0)
      {
        return -1;
      }
      continue;
    }
    readn += n;
  }

  return (ssize_t)readn;
}
