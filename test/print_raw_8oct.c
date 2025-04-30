#include <stdio.h>
#include <stdlib.h>

unsigned int read_uint32_be(const unsigned char *buf) {
  return ((unsigned int)buf[0] << 24) |
         ((unsigned int)buf[1] << 16) |
         ((unsigned int)buf[2] << 8)  |
         ((unsigned int)buf[3]);
}


int main()
{
  const char *filename = "../data/video1.raw"; // 읽고 싶은 RAW 파일 이름
  FILE *fp = fopen(filename, "rb");
  if (!fp)
  {
    perror("파일 열기 실패");
    return 1;
  }

  // 리틀엔디안으로 저장 되었다고 가정정
  // unsigned char wh_buf[8]; // 4바이트씩 width, height
  // fread(wh_buf, 1, 8, fp);

  // unsigned int width_little = wh_buf[0] | (wh_buf[1] << 8) | (wh_buf[2] << 16) | (wh_buf[3] << 24);
  // unsigned int height_little = wh_buf[4] | (wh_buf[5] << 8) | (wh_buf[6] << 16) | (wh_buf[7] << 24);

  // printf("영상 크기(little edian): %u x %u\n", width_little, height_little);

  // unsigned int width_big = read_uint32_be(&wh_buf[0]);
  // unsigned int height_big = read_uint32_be(&wh_buf[4]);

  // printf("영상 크기(big edian): %u x %u\n", width_big, height_big);

  int width = 0, height = 0;

  // 실제 영상에서 width, height 읽기
  fread(&width, sizeof(long), 1, fp);
  fread(&height, sizeof(long), 1, fp);
  printf("영상 크기(big edian): %u x %u\n", width, height);

  unsigned char buffer[10];
  size_t read_count = fread(buffer, 1, sizeof(buffer), fp);
  if (read_count < sizeof(buffer))
  {
    fprintf(stderr, "파일에서 충분한 데이터를 읽지 못했습니다. (%zu bytes)\n", read_count);
    fclose(fp);
    return 1;
  }

  printf("앞 10바이트 (8진수):\n");
  for (int i = 0; i < 10; i++)
  {
    printf("[%02d] = 0%03o\n", i, buffer[i]);
  }

  fclose(fp);
  return 0;
}
