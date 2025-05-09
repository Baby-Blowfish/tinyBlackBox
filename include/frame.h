/*
 * @file frame.h
 * @brief Generic Frame structure and operations
 *
 * Provides dynamic allocation, initialization, destruction, pixel data access,
 * and blocking I/O functions for arbitrary-depth frames.
 */
#ifndef FRAME_H
#define FRAME_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

  /**
   * @enum FrameDepth
   * @brief Defines bytes per pixel for a Frame.
   */
  typedef enum
  {
    GRAY = 1,
    RGB = 3,
    RGBA = 4,
  } DEPTH;

  /**
   * @struct Frame
   * @brief Represents a pixel frame with metadata and data buffer.
   */
  typedef struct Frame
  {
    size_t width;  /**< Frame width in pixels (>0) */
    size_t height; /**< Frame height in pixels (>0) */
    DEPTH depth;   /**< Bytes per pixel (>0) */
    size_t seq;    /**< Sequence number, starts at 0 */
    void *data;    /**< Pixel buffer of size width*height*depth bytes */
  } Frame;

  /**
   * @brief Allocate a new Frame and internal buffer.
   *
   * Allocates a Frame structure and zero-initializes its data buffer.
   * @param[in] width  Width in pixels (>0).
   * @param[in] height Height in pixels (>0).
   * @param[in] depth  Bytes per pixel (>0).
   * @return Non-NULL pointer to Frame on success; NULL on failure (errno set).
   * @retval NULL     Memory allocation or invalid parameters.
   * @retval otherwise Allocated Frame (use frame_destroy() to free).
   */
  Frame *frame_create(size_t width, size_t height, DEPTH depth);

  /**
   * @brief Free a Frame and its internal buffer.
   *
   * Safely handles NULL.
   * @param[in,out] frame Pointer to Frame to destroy.
   */
  void frame_destroy(Frame *frame);

  /**
   * @brief Initialize an existing Frame structure.
   *
   * Allocates and zero-initializes the data buffer.
   * @param[in,out] frame Pointer to pre-allocated Frame (>NULL).
   * @param[in] width     Width in pixels (>0).
   * @param[in] height    Height in pixels (>0).
   * @param[in] depth     Bytes per pixel (>0).
   * @return 0 on success; -1 on failure (errno set).
   * @retval -1   frame==NULL or width/height/depth invalid or overflow or ENOMEM.
   * @retval 0    Frame initialized (data malloc'ed).
   */
  int frame_init(Frame *frame, size_t width, size_t height, DEPTH depth);

  /**
   * @brief Release only the data buffer of a Frame.
   *
   * Does not free the Frame struct itself.
   * @param[in,out] frame Pointer to Frame with initialized data (NULL safe).
   */
  void frame_free(Frame *frame);

  /**
   * @brief Get writable pixel buffer.
   * @param[in] frame Frame pointer (NULL safe).
   * @return Pointer to data or NULL.
   */
  void *frame_get_data(Frame *frame);

  /**
   * @brief Get read-only pixel buffer.
   * @param[in] frame Frame pointer (NULL safe).
   * @return Const pointer to data or NULL.
   */
  const void *frame_get_data_const(const Frame *frame);

  /**
   * @brief Retrieve frame width.
   * @param[in] frame Frame pointer (NULL safe).
   * @return Width in pixels; 0 if frame NULL.
   */
  size_t frame_get_width(const Frame *frame);

  /**
   * @brief Retrieve frame height.
   * @param[in] frame Frame pointer (NULL safe).
   * @return Height in pixels; 0 if frame NULL.
   */
  size_t frame_get_height(const Frame *frame);

  /**
   * @brief Retrieve pixel depth.
   * @param[in] frame Frame pointer (NULL safe).
   * @return Bytes per pixel; 0 if frame NULL.
   */
  DEPTH frame_get_depth(const Frame *frame);

  /**
   * @brief Retrieve sequence number.
   * @param[in] frame Frame pointer (NULL safe).
   * @return Sequence; 0 if frame NULL.
   */
  size_t frame_get_seq(const Frame *frame);

  /**
   * @brief Print frame metadata.
   * @param[in] frame Frame pointer (NULL safe).
   * @param[in] out   FILE stream; defaults to stdout if NULL.
   */
  void frame_dump_info(const Frame *frame, FILE *out);

  /**
   * @brief Write only pixel data to file descriptor (blocking).
   *
   * Handles partial writes internally (EINTR retry).
   * @param[in] fd    Writable file descriptor.
   * @param[in] frame Frame with data pointer set.
   * @return Bytes written; -1 on error (errno set).
   */
  ssize_t frame_write_data(int fd, const Frame *frame);

  /**
   * @brief Read pixel data from file descriptor into frame (one-shot).
   *
   * Attempts to read exactly width*height*depth bytes; fails if EOF encountered early.
   * @param[in]  fd    Readable file descriptor.
   * @param[out] frame Initialized Frame with data buffer.
   * @return Bytes read; -1 on error or incomplete read (errno set).
   */
  ssize_t frame_read_data(int fd, Frame *frame);

  /**
   * @brief Continuously read pixel data; rewind on EOF.
   *
   * Loops until frame buffer is full, seeking to start on EOF.
   * @param[in]  fd    Readable file descriptor.
   * @param[out] frame Initialized Frame.
   * @return Bytes read; -1 on error (errno set).
   */
  ssize_t frame_read_loop(int fd, Frame *frame);

#ifdef __cplusplus
}
#endif
#endif // FRAME_H
