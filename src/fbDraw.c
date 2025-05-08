#include "fbDraw.h"

int fb_init(dev_fb *fb)
{
  fb->fbfd = 0;
  fb->fbp = NULL;
  fb->fbfd = open(FBDEVICE, O_RDWR);

  if (fb->fbfd == -1)
    return FB_OPEN_FAIL;
  if (ioctl(fb->fbfd, FBIOGET_FSCREENINFO, &(fb->finfo)) < 0)
    return FB_GET_FINFO_FAIL;
  if (ioctl(fb->fbfd, FBIOGET_VSCREENINFO, &(fb->vinfo)) < 0)
    return FB_GET_VINFO_FAIL;

  // 현재 프레임 버퍼의 해상도 및 색상 깊이 정보 출력
  printf("Resolution : %dx%d, %dbpp\n", fb->vinfo.xres, fb->vinfo.yres, fb->vinfo.bits_per_pixel);
  printf("Virtual Resolution : %dx%d\n", fb->vinfo.xres_virtual, fb->vinfo.yres_virtual);

  // 각 색상 채널의 오프셋과 길이 출력
  printf("Red: offset = %d, length = %d\n", fb->vinfo.red.offset, fb->vinfo.red.length);
  printf("Green: offset = %d, length = %d\n", fb->vinfo.green.offset, fb->vinfo.green.length);
  printf("Blue: offset = %d, length = %d\n", fb->vinfo.blue.offset, fb->vinfo.blue.length);
  printf("Alpha (transparency): offset = %d, length = %d\n", fb->vinfo.transp.offset,
         fb->vinfo.transp.length);

  // 4) 필요하면 grayscale 포맷 설정 (옵션, 드라이버 지원 시)
  // var.grayscale = 1;
  // if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &var) < 0)
  //     perror("ioctl(FBIOPUT_VSCREENINFO) — grayscale");

  /*
  finfo.line_length : 가로 한줄에 GPU/하드웨어가 실제로 할당해 놓은 바이트
  실제 화면 해상도 xres × (bits_per_pixel/8) 보다 클 수 있는데, 줄 끝에 남겨둔 패딩(padding)이나
  가로 가상 해상도(xres_virtual) 차이를 메우기 위해 여유 공간을 포함한 값.
  */

  /*vinfo.yres_virtual : “메모리 상에 할당된 가상 세로 줄 수”
        실제 표시줄(yres)보다 크게 잡을 수 있어, 커서를 움직이는 panning 기능이나 더블 버퍼링 등을
     위해 여유 줄을 예약해 둡니다.
  */
  fb->screensize = fb->finfo.line_length * fb->vinfo.yres_virtual;

  fb->fbp = (ubyte *)mmap(NULL, fb->screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fbfd, 0);
  if (fb->fbp == MAP_FAILED)
  {
    fb->fbp = NULL;
    return FB_MMAP_FAIL;
  }

  return 0;
}

int fb_drawGray(dev_fb *fb, const ubyte *gray, int raw_w, int raw_h)
{
  if (!fb || !fb->fbp || !gray || raw_w <= 0 || raw_h <= 0)
    return -1;

  const int fb_w = fb->vinfo.xres;
  const int fb_h = fb->vinfo.yres;
  const int bpp = fb->vinfo.bits_per_pixel;

  for (int y = 0; y < fb_h; y++)
  {
    int src_y = y * raw_h / fb_h;
    for (int x = 0; x < fb_w; x++)
    {
      int src_x = x * raw_w / fb_w;
      ubyte g = gray[src_y * raw_w + src_x];
      /*
        x = 0 → src_x = 0×1920/800 = ０
        x = 1 → src_x = 1×1920/800 = 2.4 → 정수로 2
        x = 2 → src_x = 2×1920/800 = 4.8 → 정수로 4
        x = 100 → src_x = 100×1920/800 = 240
        …
        x = 799 → src_x = 799×1920/800 ≈ 1917.6 → 1917
        800픽셀짜리 출력 영역을 1920픽셀짜리 원본 영상에 “nearest‐neighbor” 방식으로 맵핑 ->
        출력 한칸에 원본 2~3픽셀을 대응시켜 전체 영상을 화면 크기에 맞춰 축소

        x = 0 → 0.0 → src_x = 0
        x = 1 → 0.416 → src_x = 0
        x = 2 → 0.833 → src_x = 0
        x = 3 → 1.25 → src_x = 1
        x = 4 → 1.666 → src_x = 1
        x = 5 → 2.083 → src_x = 2
        …
        x = 1919 → ≈799.166 → src_x = 799
        즉 한 개의 원본 픽셀(예: raw[0], raw[1], raw[2]…)이 약 2~3번 출력 픽셀로 반복 매핑되면서
        업샘플링
      */

      size_t loc = locate(
          fb, x, y); // 픽셀 오프셋 계산
                     // :contentReference[oaicite:0]{index=0}:contentReference[oaicite:1]{index=1}

      if (bpp == 32)
      {
        uint32_t pixel = ((g >> (8 - fb->vinfo.red.length)) << fb->vinfo.red.offset) |
                         ((g >> (8 - fb->vinfo.green.length)) << fb->vinfo.green.offset) |
                         ((g >> (8 - fb->vinfo.blue.length)) << fb->vinfo.blue.offset);
        *(uint32_t *)(fb->fbp + loc) = pixel;
      }
      else if (bpp == 16)
      {
        uint16_t r5 = (g >> (8 - fb->vinfo.red.length)) & ((1 << fb->vinfo.red.length) - 1);
        uint16_t g6 = (g >> (8 - fb->vinfo.green.length)) & ((1 << fb->vinfo.green.length) - 1);
        uint16_t b5 = (g >> (8 - fb->vinfo.blue.length)) & ((1 << fb->vinfo.blue.length) - 1);
        uint16_t pixel = (r5 << fb->vinfo.red.offset) | (g6 << fb->vinfo.green.offset) |
                         (b5 << fb->vinfo.blue.offset);
        *(uint16_t *)(fb->fbp + loc) = pixel;
      }
      else
      {
        return -2; // 지원하지 않는 bpp
      }
    }
  }

  return 0;
}

pixel fb_toPixel(int x, int y)
{
  pixel px;
  px.x = x;
  px.y = y;
  return px;
}

//전달 받은 좌표가 프래임버퍼가 출력할 수 있는 영역내에 있는지 확인하기 위한 함수
int fb_checkPx(dev_fb *fb, int x, int y)
{
  return (x >= 0 && y >= 0 && (unsigned int)x < fb->vinfo.xres && (unsigned int)y < fb->vinfo.yres);
}

size_t locate(dev_fb *fb, int x, int y)
{
  // 1. y 방향: (y + yoffset) 줄이 한 줄당 line_length 바이트씩 건너뛰고
  size_t row_offset = (y + fb->vinfo.yoffset) * fb->finfo.line_length;

  // 2. x 방향: (x + xoffset) 픽셀이 픽셀당 bpp/8 바이트씩 건너뛰고
  size_t col_offset = (x + fb->vinfo.xoffset) * (fb->vinfo.bits_per_pixel / 8);

  return row_offset + col_offset;
}

void fb_drawPixelPx(dev_fb *fb, pixel px, char r, char g, char b)
{
  fb_drawPixel(fb, px.x, px.y, r, g, b);
}

void fb_drawPixel(dev_fb *fb, int x, int y, char r, char g, char b)
{
  size_t location = locate(fb, x, y);
  if (fb_checkPx(fb, x, y))
  {
    if (fb->vinfo.bits_per_pixel == 32)
    {
      *(fb->fbp + location) = b;
      *(fb->fbp + location + 1) = g;
      *(fb->fbp + location + 2) = r;
      *(fb->fbp + location + 3) = 0;
    }
    else
    {
      unsigned short int t = r << 11 | g << 5 | b;
      *((unsigned short int *)(fb->fbp + location)) = t;
    }
  }
}

void fb_drawPixelwithAlpha(dev_fb *fb, int x, int y, char r, char g, char b, char a)
{
  size_t location = locate(fb, x, y);
  if (fb_checkPx(fb, x, y))
  {
    if (fb->vinfo.bits_per_pixel == 32)
    {
      *(fb->fbp + location) = b;
      *(fb->fbp + location + 1) = g;
      *(fb->fbp + location + 2) = r;
      *(fb->fbp + location + 3) = a;
    }
    else
    {
      unsigned short int t = r << 11 | g << 5 | b;
      *((unsigned short int *)(fb->fbp + location)) = t;
    }
  }
}

void fb_fillScr(dev_fb *fb, char r, char g, char b)
{
  int x, y;
  size_t location;
  for (y = 0; y < (int)fb->vinfo.yres; y++)
  {
    for (x = 0; x < (int)fb->vinfo.xres; x++)
    {
      location = locate(fb, x, y);
      *(fb->fbp + location) = b;
      *(fb->fbp + location + 1) = g;
      *(fb->fbp + location + 2) = r;
      *(fb->fbp + location + 3) = 0;
    }
  }
}

void fb_drawBox(dev_fb *fb, pixel px, int w, int h, char r, char g, char b)
{
  int x, y;

  for (x = 0; x < w; x++)
    fb_drawPixel(fb, px.x + x, px.y, r, g, b);
  for (y = 1; y < (h - 1); y++)
  {
    fb_drawPixel(fb, px.x, px.y + y, r, g, b);
    fb_drawPixel(fb, px.x + (w - 1), px.y + y, r, g, b);
  }
  for (x = 0; x < w; x++)
    fb_drawPixel(fb, px.x + x, px.y + (h - 1), r, g, b);
}

void fb_drawBoxWidthAlpa(dev_fb *fb, pixel px, int w, int h, char r, char g, char b, char a)
{
  int x, y;

  for (x = 0; x < w; x++)
    fb_drawPixelwithAlpha(fb, px.x + x, px.y, r, g, b, a);
  for (y = 1; y < (h - 1); y++)
  {
    fb_drawPixelwithAlpha(fb, px.x, px.y + y, r, g, b, a);
    fb_drawPixelwithAlpha(fb, px.x + (w - 1), px.y + y, r, g, b, a);
  }
  for (x = 0; x < w; x++)
    fb_drawPixelwithAlpha(fb, px.x + x, px.y + (h - 1), r, g, b, a);
}

void fb_fillBox(dev_fb *fb, pixel px, int w, int h, char r, char g, char b)
{
  int x, y;
  for (y = 0; y < h; y++)
  {
    for (x = 0; x < w; x++)
    {
      fb_drawPixel(fb, px.x + x, px.y + y, r, g, b);
    }
  }
}

/*
        fb_drawLine: Bresenham's line equation
*/
void fb_drawLine(dev_fb *fb, pixel start, pixel end, char r, char g, char b)
{
  int error, x, y;
  char swap;
  char ydir;
  if (abs(end.y - start.y) > abs(end.x - start.x))
  {
    swap = 1;
    error = start.x;
    start.x = start.y;
    start.y = error;
    error = end.x;
    end.x = end.y;
    end.y = error;
  }
  else
    swap = 0;

  if (start.x > end.x)
  {
    error = start.x;
    start.x = end.x;
    end.x = error;
    error = start.y;
    start.y = end.y;
    end.y = error;
  }

  int dx = end.x - start.x;
  int dy = abs(end.y - start.y);
  error = -(dx / 2);

  if (start.y < end.y)
    ydir = 1;
  else
    ydir = -1;

  y = start.y;

  if (swap == 1)
  {
    for (x = start.x; x <= end.x; x++)
    {
      fb_drawPixel(fb, y, x, r, g, b);
      error += dy;
      if (error > 0)
      {
        if (ydir == 1)
          y++;
        else
          y--;
        error -= dx;
      }
    }
  }
  else
  {
    for (x = start.x; x <= end.x; x++)
    {
      fb_drawPixel(fb, x, y, r, g, b);
      error += dy;
      if (error > 0)
      {
        if (ydir == 1)
          y++;
        else
          y--;
        error -= dx;
      }
    }
  }
}

void fb_drawChar(dev_fb *fb, char c, pixel start, short h, char r, char g, char b)
{
  short w = h / 3;
  if (w % 2)
    w++;
  pixel topRt, btmLt, btmRt, midLt, midRt;
  topRt.x = start.x + w;
  topRt.y = start.y;
  btmLt.x = start.x;
  btmLt.y = start.y + h;
  btmRt.x = start.x + w;
  btmRt.y = start.y + h;
  midLt = fb_toPixel(start.x, start.y + (h / 2));
  midRt = fb_toPixel(topRt.x, midLt.y);

  switch (c)
  {
  case '0':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, btmLt, topRt, r, g, b);
    break;
  case '1':
    fb_drawLine(fb, fb_toPixel(start.x, start.y + (h / 4)), fb_toPixel(start.x + (w / 2), start.y),
                r, g, b);
    fb_drawLine(fb, fb_toPixel(start.x + (w / 2), start.y),
                fb_toPixel(start.x + (w / 2), start.y + h), r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    break;
  case '2':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, midRt, r, g, b);
    fb_drawLine(fb, midRt, midLt, r, g, b);
    fb_drawLine(fb, midLt, btmLt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    break;
  case '3':
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, midLt, midRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    break;
  case '4':
    fb_drawLine(fb, topRt, midLt, r, g, b);
    fb_drawLine(fb, midLt, midRt, r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    break;
  case 's':
  case 'S':
  case '5':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, start, midLt, r, g, b);
    fb_drawLine(fb, midRt, midLt, r, g, b);
    fb_drawLine(fb, midRt, btmRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    break;
  case '6':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, midRt, midLt, r, g, b);
    fb_drawLine(fb, midRt, btmRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    break;
  case '7':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, btmLt, r, g, b);
    break;
  case '8':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, midLt, midRt, r, g, b);
    break;
  case '9':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    fb_drawLine(fb, start, midLt, r, g, b);
    fb_drawLine(fb, midLt, midRt, r, g, b);
    break;
  case '!':
    fb_drawLine(fb, fb_toPixel(start.x + (w / 2), start.y),
                fb_toPixel(start.x + (w / 2), btmRt.y - (h / 4)), r, g, b);
    fb_drawLine(fb, fb_toPixel(start.x + (w / 2), start.y + h),
                fb_toPixel(start.x + (w / 2), start.y + h - 1), r, g, b);
    break;
  case '?':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, midRt, r, g, b);
    fb_drawLine(fb, midRt, fb_toPixel(midLt.x + (w / 4), midLt.y), r, g, b);
    fb_drawLine(fb, fb_toPixel(midLt.x + (w / 4), midLt.y),
                fb_toPixel(start.x + (w / 4), btmRt.y - (h / 4)), r, g, b);
    fb_drawLine(fb, fb_toPixel(start.x + (w / 4), start.y + h),
                fb_toPixel(start.x + (w / 4), start.y + h - 1), r, g, b);
    break;
  case '.':
    fb_drawLine(fb, fb_toPixel(start.x + (w / 2), start.y + h),
                fb_toPixel(start.x + (w / 2) + 1, start.y + h), r, g, b);
    fb_drawLine(fb, fb_toPixel(start.x + (w / 2), start.y + h - 1),
                fb_toPixel(start.x + (w / 2) + 1, start.y + h - 1), r, g, b);
    break;
  case ',':
    fb_drawLine(fb, fb_toPixel(start.x + (w / 2) + 1, start.y + h),
                fb_toPixel(start.x + (w / 2) + 1, start.y + h - 2), r, g, b);
    break;
  case '/':
    fb_drawLine(fb, btmLt, topRt, r, g, b);
    break;
  case '\\':
    fb_drawLine(fb, start, btmRt, r, g, b);
    break;
  case '(':
  case '{':
  case '[':
    fb_drawLine(fb, start, fb_toPixel(start.x + (w / 2), start.y), r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, btmLt, fb_toPixel(start.x + (w / 2), btmLt.y), r, g, b);
    break;
  case ')':
  case '}':
  case ']':
    fb_drawLine(fb, topRt, fb_toPixel(start.x + (w / 2), start.y), r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, btmRt, fb_toPixel(start.x + (w / 2), btmLt.y), r, g, b);
    break;
  case 'a':
  case 'A':
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, midLt, midRt, r, g, b);
    break;
  case 'b':
  case 'B':
    fb_drawLine(fb, start, fb_toPixel(topRt.x - (w / 3), start.y), r, g, b);
    fb_drawLine(fb, fb_toPixel(topRt.x - (w / 3), topRt.y), fb_toPixel(topRt.x, topRt.y + (h / 8)),
                r, g, b);
    fb_drawLine(fb, fb_toPixel(topRt.x, topRt.y + (h / 8)), fb_toPixel(topRt.x, midRt.y - (h / 8)),
                r, g, b);
    fb_drawLine(fb, fb_toPixel(topRt.x, midRt.y - (h / 8)), fb_toPixel(midRt.x - (w / 3), midRt.y),
                r, g, b);
    fb_drawLine(fb, midLt, fb_toPixel(midRt.x - (w / 3), midRt.y), r, g, b);
    fb_drawLine(fb, fb_toPixel(midRt.x - (w / 3), midRt.y), fb_toPixel(midRt.x, midRt.y + (h / 8)),
                r, g, b);
    fb_drawLine(fb, fb_toPixel(midRt.x, midRt.y + (h / 8)), fb_toPixel(btmRt.x, btmRt.y - (h / 8)),
                r, g, b);
    fb_drawLine(fb, fb_toPixel(btmRt.x, btmRt.y - (h / 8)), fb_toPixel(btmRt.x - (w / 3), btmRt.y),
                r, g, b);
    fb_drawLine(fb, btmLt, fb_toPixel(btmRt.x - (w / 3), btmRt.y), r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    break;
  case 'c':
  case 'C':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    break;
  case 'd':
  case 'D':
    fb_drawLine(fb, start, fb_toPixel(topRt.x - (w / 3), start.y), r, g, b);
    fb_drawLine(fb, fb_toPixel(topRt.x - (w / 3), topRt.y), fb_toPixel(topRt.x, topRt.y + (h / 8)),
                r, g, b);
    fb_drawLine(fb, fb_toPixel(topRt.x, topRt.y + (h / 8)), fb_toPixel(btmRt.x, btmRt.y - (h / 8)),
                r, g, b);
    fb_drawLine(fb, fb_toPixel(btmRt.x, btmRt.y - (h / 8)), fb_toPixel(btmRt.x - (w / 3), btmRt.y),
                r, g, b);
    fb_drawLine(fb, btmLt, fb_toPixel(btmRt.x - (w / 3), btmRt.y), r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    break;
  case 'e':
  case 'E':
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, midLt, midRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    break;
  case 'f':
  case 'F':
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, midLt, midRt, r, g, b);
    break;
  case 'g':
  case 'G':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, midRt, fb_toPixel(midRt.x - (w / 3), midRt.y), r, g, b);
    fb_drawLine(fb, midRt, btmRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    break;
  case 'h':
  case 'H':
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, midLt, midRt, r, g, b);
    break;
  case 'i':
  case 'I':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    fb_drawLine(fb, fb_toPixel(start.x + (w / 2), start.y), fb_toPixel(start.x + (w / 2), btmRt.y),
                r, g, b);
    break;
  case 'j':
  case 'J':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, fb_toPixel(start.x + (w / 2), start.y), fb_toPixel(start.x + (w / 2), btmRt.y),
                r, g, b);
    fb_drawLine(fb, btmLt, fb_toPixel(start.x + (w / 2), btmRt.y), r, g, b);
    fb_drawLine(fb, midLt, btmLt, r, g, b);
    break;
  case 'k':
  case 'K':
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, midLt, topRt, r, g, b);
    fb_drawLine(fb, midLt, btmRt, r, g, b);
    break;
  case 'l':
  case 'L':
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    break;
  case 'm':
  case 'M':
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, start, fb_toPixel(midLt.x + (w / 2), midLt.y), r, g, b);
    fb_drawLine(fb, topRt, fb_toPixel(midLt.x + (w / 2), midLt.y), r, g, b);
    break;
  case 'n':
  case 'N':
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, start, btmRt, r, g, b);
    break;
  case 'o':
  case 'O':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    break;
  case 'p':
  case 'P':
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, midRt, r, g, b);
    fb_drawLine(fb, midLt, midRt, r, g, b);
    break;
  case 'q':
  case 'Q':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, fb_toPixel(midLt.x + w / 2, midLt.y), btmRt, r, g, b);
    break;
  case 'r':
  case 'R':
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, topRt, midRt, r, g, b);
    fb_drawLine(fb, midLt, midRt, r, g, b);
    fb_drawLine(fb, midLt, btmRt, r, g, b);
    break;
  case 't':
  case 'T':
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, fb_toPixel(start.x + (w / 2), start.y), fb_toPixel(start.x + (w / 2), btmRt.y),
                r, g, b);
    break;
  case 'u':
  case 'U':
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    fb_drawLine(fb, start, btmLt, r, g, b);
    break;
  case 'v':
  case 'V':
    fb_drawLine(fb, start, fb_toPixel(btmLt.x + w / 2, btmLt.y), r, g, b);
    fb_drawLine(fb, topRt, fb_toPixel(btmLt.x + w / 2, btmLt.y), r, g, b);
    break;
  case 'w':
  case 'W':
    fb_drawLine(fb, start, btmLt, r, g, b);
    fb_drawLine(fb, topRt, btmRt, r, g, b);
    fb_drawLine(fb, btmLt, fb_toPixel(midLt.x + (w / 2), midLt.y), r, g, b);
    fb_drawLine(fb, btmRt, fb_toPixel(midLt.x + (w / 2), midLt.y), r, g, b);
    break;
  case 'x':
  case 'X':
    fb_drawLine(fb, start, btmRt, r, g, b);
    fb_drawLine(fb, topRt, btmLt, r, g, b);
    break;
  case 'y':
  case 'Y':
    fb_drawLine(fb, start, fb_toPixel(midLt.x + (w / 2), midLt.y), r, g, b);
    fb_drawLine(fb, topRt, fb_toPixel(midLt.x + (w / 2), midLt.y), r, g, b);
    fb_drawLine(fb, fb_toPixel(midLt.x + (w / 2), midLt.y), fb_toPixel(midLt.x + (w / 2), btmLt.y),
                r, g, b);
    break;
  case 'z':
  case 'Z':
    fb_drawLine(fb, topRt, btmLt, r, g, b);
    fb_drawLine(fb, start, topRt, r, g, b);
    fb_drawLine(fb, btmLt, btmRt, r, g, b);
    break;
  default:
    break;
  }
}

void fb_printStr(dev_fb *fb, const char *str, pixel *cursor, short height, char r, char g, char b)
{
  size_t l = strlen(str);
  short w = height / 3;
  int i, j;
  int lnStart = cursor->x;
  if (w % 2)
    w++;
  short c_offset = (2 * w);

  for (i = 0; i < (int)l; i++)
  {
    if (str[i] == '\n')
    {
      cursor->y += 3 * height / 2;
      cursor->x = lnStart;
    }
    else if (str[i] == '\t')
    {
      for (j = 0; j < 4; j++)
      {
        fb_drawChar(fb, ' ', *cursor, height, r, g, b);
        cursor->x += c_offset;
        if (cursor->x + w > (int)fb->vinfo.xres)
        {
          cursor->y += 3 * height / 2;
          cursor->x = lnStart;
          j = 0;
        }
      }
    }
    else
    {
      fb_drawChar(fb, str[i], *cursor, height, r, g, b);
      cursor->x += c_offset;
      if (cursor->x + w > (int)fb->vinfo.xres)
      {
        cursor->y += 3 * height / 2;
        cursor->x = lnStart;
      }
    }
  }
}

void fb_drawFilledCircle(dev_fb *fb, pixel center, char r, char g, char b)
{
  int cx = center.x;
  int cy = center.y;

  int iCircleX, iCircleY;

  int distance = -RADIUS;
  iCircleY = RADIUS;

  fb_drawLine(fb, fb_toPixel(cx, cy + RADIUS), fb_toPixel(cx, cy - RADIUS), r, g, b); // y축 선 긋기
  fb_drawLine(fb, fb_toPixel(cx + RADIUS, cy), fb_toPixel(cx - RADIUS, cy), r, g, b); // x축 선 긋기

  for (iCircleX = 1; iCircleX <= iCircleY; iCircleX++)
  {
    distance += (iCircleX << 1) - 1; // 2 * iCircleX - 1;

    if (distance >= 0)
    {
      iCircleY--;
      distance += (-iCircleY << 1) + 2;
    }

    fb_drawLine(fb, fb_toPixel(-iCircleX + cx, iCircleY + cy),
                fb_toPixel(iCircleX + cx, iCircleY + cy), r, g, b);
    fb_drawLine(fb, fb_toPixel(-iCircleX + cx, -iCircleY + cy),
                fb_toPixel(iCircleX + cx, -iCircleY + cy), r, g, b);
    fb_drawLine(fb, fb_toPixel(-iCircleY + cx, iCircleX + cy),
                fb_toPixel(iCircleY + cx, iCircleX + cy), r, g, b);
    fb_drawLine(fb, fb_toPixel(-iCircleY + cx, -iCircleX + cy),
                fb_toPixel(iCircleY + cx, -iCircleX + cy), r, g, b);
  }
}

int fb_displayGrayFrame(dev_fb *fb, const ubyte *gray, int raw_w, int raw_h)
{
  if (!fb || !fb->fbp || !gray || raw_w <= 0 || raw_h <= 0)
    return -1;

  int fb_w = fb->vinfo.xres;
  int fb_h = fb->vinfo.yres;
  int bpp = fb->vinfo.bits_per_pixel;

  for (int y = 0; y < fb_h; y++)
  {
    int src_y = (y * raw_h) / fb_h;
    for (int x = 0; x < fb_w; x++)
    {
      int src_x = (x * raw_w) / fb_w;
      ubyte g = gray[src_y * raw_w + src_x];

      size_t loc = locate(
          fb, x, y); // 픽셀 메모리 오프셋 계산
                     // :contentReference[oaicite:0]{index=0}:contentReference[oaicite:1]{index=1}

      if (bpp == 32)
      {
        // XRGB8888: B,G,R,A 순서로 채워넣기
        // :contentReference[oaicite:2]{index=2}:contentReference[oaicite:3]{index=3}
        fb->fbp[loc + 0] = g; // Blue
        fb->fbp[loc + 1] = g; // Green
        fb->fbp[loc + 2] = g; // Red
        fb->fbp[loc + 3] = 0; // Alpha
      }
      else if (bpp == 16)
      {
        // RGB565: R 5bit, G 6bit, B 5bit
        uint16_t r5 = (g >> (8 - fb->vinfo.red.length)) & ((1 << fb->vinfo.red.length) - 1);
        uint16_t g6 = (g >> (8 - fb->vinfo.green.length)) & ((1 << fb->vinfo.green.length) - 1);
        uint16_t b5 = (g >> (8 - fb->vinfo.blue.length)) & ((1 << fb->vinfo.blue.length) - 1);
        uint16_t pixel = (r5 << fb->vinfo.red.offset) | (g6 << fb->vinfo.green.offset) |
                         (b5 << fb->vinfo.blue.offset);
        *((uint16_t *)(fb->fbp + loc)) = pixel;
      }
      else
      {
        // 지원하지 않는 비트 깊이
        return -2;
      }
    }
  }
  return 0;
}

void fb_close(dev_fb *fb)
{
  munmap(fb->fbp, fb->screensize);
  close(fb->fbfd);
}
