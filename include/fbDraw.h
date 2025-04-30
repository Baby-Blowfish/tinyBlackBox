#ifndef __FBDRAW_H__
#define __FBDRAW_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>


/**
 * @brief Error codes for framebuffer operations
 */
#define FB_OPEN_FAIL 1      ///< Failed to open framebuffer device
#define FB_GET_FINFO_FAIL 2 ///< Failed to get fixed screen information
#define FB_GET_VINFO_FAIL 3 ///< Failed to get variable screen information
#define FB_MMAP_FAIL 4      ///< Failed to memory map the framebuffer
#define RADIUS 20           ///< Default radius for circle drawing
#define FBDEVICE "/dev/fb0" ///< Path to the framebuffer device

  /**
   * @brief Type definition for unsigned byte
   */
  typedef unsigned char ubyte;

  /**
   * @brief Framebuffer device structure
   * @details Contains all necessary information to interact with the Linux framebuffer
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  typedef struct dev_fb_t
  {
    int fbfd;                       ///< Framebuffer file descriptor
    struct fb_var_screeninfo vinfo; ///< Variable screen information
    struct fb_fix_screeninfo finfo; ///< Fixed screen information
    long int screensize;            ///< Size of the framebuffer in bytes
    ubyte *fbp;                     ///< Pointer to the mapped framebuffer memory
  } dev_fb;

  /**
   * @brief Pixel coordinate structure
   * @details Represents a point in 2D space with x and y coordinates
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  typedef struct pixel_t
  {
    int x; ///< X-coordinate
    int y; ///< Y-coordinate
  } pixel;

  /**
   * @brief Initializes the framebuffer device
   * @param fb Pointer to the framebuffer device structure to initialize
   * @return 0 on success, error code on failure
   * @details Opens the framebuffer device, retrieves screen information,
   *          and maps the framebuffer memory for direct access
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  int fb_init(dev_fb *fb);

  /**
   * @brief Creates a pixel object from x and y coordinates
   * @param x X-coordinate
   * @param y Y-coordinate
   * @return A pixel structure containing the specified coordinates
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  pixel fb_toPixel(int x, int y);

  /**
   * @brief Draws a pixel at the specified pixel coordinates
   * @param fb Pointer to the framebuffer device
   * @param px Pixel coordinates where to draw
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @details Writes color values directly to the framebuffer memory
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_drawPixelPx(dev_fb *fb, pixel px, char r, char g, char b);

  /**
   * @brief Draws a pixel at the specified x,y coordinates
   * @param fb Pointer to the framebuffer device
   * @param x X-coordinate
   * @param y Y-coordinate
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @details Writes color values directly to the framebuffer memory
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_drawPixel(dev_fb *fb, int x, int y, char r, char g, char b);

  /**
   * @brief Draws a pixel with alpha transparency
   * @param fb Pointer to the framebuffer device
   * @param x X-coordinate
   * @param y Y-coordinate
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @param a Alpha transparency value (0-255)
   * @details Writes color and alpha values directly to the framebuffer memory
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_drawPixelwithAlpha(dev_fb *fb, int x, int y, char r, char g, char b, char a);

  /**
   * @brief Fills the entire screen with a solid color
   * @param fb Pointer to the framebuffer device
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @details Sets all pixels in the framebuffer to the specified color
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_fillScr(dev_fb *fb, char r, char g, char b);

  /**
   * @brief Draws a box outline
   * @param fb Pointer to the framebuffer device
   * @param px Starting pixel coordinates
   * @param w Width of the box
   * @param h Height of the box
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @details Draws a rectangular outline starting from the specified pixel
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_drawBox(dev_fb *fb, pixel px, int w, int h, char r, char g, char b);

  /**
   * @brief Draws a box outline with alpha transparency
   * @param fb Pointer to the framebuffer device
   * @param px Starting pixel coordinates
   * @param w Width of the box
   * @param h Height of the box
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @param a Alpha transparency value (0-255)
   * @details Draws a rectangular outline with transparency starting from the specified pixel
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_drawBoxWidthAlpa(dev_fb *fb, pixel px, int w, int h, char r, char g, char b, char a);

  /**
   * @brief Fills a box with a solid color
   * @param fb Pointer to the framebuffer device
   * @param px Starting pixel coordinates
   * @param w Width of the box
   * @param h Height of the box
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @details Fills a rectangular area with the specified color
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_fillBox(dev_fb *fb, pixel px, int w, int h, char r, char g, char b);

  /**
   * @brief Draws a line between two points
   * @param fb Pointer to the framebuffer device
   * @param start Starting pixel coordinates
   * @param end Ending pixel coordinates
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @details Draws a line from the start point to the end point using the specified color
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_drawLine(dev_fb *fb, pixel start, pixel end, char r, char g, char b);

  /**
   * @brief Draws a single character
   * @param fb Pointer to the framebuffer device
   * @param c Character to draw
   * @param start Starting pixel coordinates
   * @param height Height of the character
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @details Draws a single character (numbers, letters, symbols) at the specified position
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_drawChar(dev_fb *fb, char c, pixel start, short height, char r, char g, char b);

  /**
   * @brief Prints a string of characters
   * @param fb Pointer to the framebuffer device
   * @param str String to print
   * @param cursor Pointer to the current cursor position (updated after printing)
   * @param height Height of the characters
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @details Prints a string of characters starting at the cursor position,
   *          handling newlines and tabs appropriately
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_printStr(dev_fb *fb, const char *str, pixel *cursor, short height, char r, char g,
                   char b);

  /**
   * @brief Draws a filled circle
   * @param fb Pointer to the framebuffer device
   * @param center Center pixel coordinates
   * @param r Red color component (0-255)
   * @param g Green color component (0-255)
   * @param b Blue color component (0-255)
   * @details Draws a filled circle with the specified radius around the center point
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_drawFilledCircle(dev_fb *fb, pixel center, char r, char g, char b);

  /**
   * @brief Closes the framebuffer device
   * @param fb Pointer to the framebuffer device
   * @details Unmaps the framebuffer memory and closes the device file
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void fb_close(dev_fb *fb);

#ifdef __cplusplus
}
#endif

#endif //__FBDRAW_H__