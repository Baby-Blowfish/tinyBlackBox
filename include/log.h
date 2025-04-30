#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Logging level enumeration
   * @details Defines different severity levels for logging events in the application.
   *          Used to categorize log messages based on their importance and severity.
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  typedef enum
  {
    LOG_DEBUG,   ///< Debug level for detailed information useful for debugging
    LOG_INFO,    ///< Information level for general operational information
    LOG_WARNING, ///< Warning level for potentially harmful situations
    LOG_ERROR ///< Error level for error events that might still allow the application to continue
              ///< running
  } LogLevel;

  /**
   * @brief Logs an event with specified parameters
   * @param level The severity level of the log message
   * @param action The action being performed (e.g., "ADD", "DEL", "CONNECT")
   * @param fd The file descriptor associated with the event (client socket)
   * @param count A numeric value associated with the event (e.g., number of balls)
   * @param details Additional information about the event
   * @details Records events in the application with timestamp, severity level,
   *          and contextual information. Logs are written to both console and
   *          a log file for persistence.
   * @date 2025-04-07
   * @author Kim Hyo Jin
   */
  void log_event(LogLevel level, const char *action, int fd, int count, const char *details);

#ifdef __cplusplus
}
#endif

#endif // LOG_H
