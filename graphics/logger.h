#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_DEBUG
} LogLevel;

static const char* levelToString(LogLevel level);

static void printTimestamp();

void initLogger(void);

void logMessage(LogLevel level, const char* format, ...);

void setLogLevel(LogLevel level);

void debugPrint(const char* file, int line);

#ifdef DEBUG
    #define LOG_DEBUG(fmt, ...) logMessage(LOG_LEVEL_DEBUG, fmt " | Function: %s | File: %s:%d", ##__VA_ARGS__, __func__, __FILE__, __LINE__)
    #define DEBUG_HERE() debugPrint(__FILE__, __LINE__)
    // #define LOG_INFO(fmt, ...) logMessage(LOG_LEVEL_INFO, fmt " | Function: %s | File: %s:%d", ##__VA_ARGS__, __func__, __FILE__, __LINE__)
    #define LOG_INFO(fmt, ...) logMessage(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
    #define LOG_WARN(fmt, ...) logMessage(LOG_LEVEL_WARNING, fmt " | Function: %s | File: %s:%d", ##__VA_ARGS__, __func__, __FILE__, __LINE__)
    #define LOG_ERROR(fmt, ...) logMessage(LOG_LEVEL_ERROR, fmt " | Function: %s | File: %s:%d", ##__VA_ARGS__, __func__, __FILE__, __LINE__)
    #define LOG_FATAL(fmt, ...) \
        do { \
            logMessage(LOG_LEVEL_ERROR, "FATAL: " fmt " | Function: %s | File: %s:%d", ##__VA_ARGS__, __func__, __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } while (0)
#else
    #define LOG_DEBUG(fmt, ...) ((void)0)
    #define DEBUG_HERE() ((void)0)
    #define LOG_INFO(fmt, ...) logMessage(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
    #define LOG_WARN(fmt, ...) logMessage(LOG_LEVEL_WARNING, fmt, ##__VA_ARGS__)
    #define LOG_ERROR(fmt, ...) logMessage(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
    #define LOG_FATAL(fmt, ...) \
        do { \
            logMessage(LOG_LEVEL_ERROR, "FATAL: " fmt, ##__VA_ARGS__); \
            exit(EXIT_FAILURE); \
        } while (0)
#endif
#endif