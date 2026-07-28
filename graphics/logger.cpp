#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "logger.h"

static LogLevel currentLogLevel = LOG_LEVEL_INFO;

static int isConsoleInitialized = 0;

// ANSI escape codes for all platforms
static const char* levelColors[] = {
    "\033[32m", // INFO: Green
    "\033[33m", // WARNING: Yellow
    "\033[31m", // ERROR: Red
    "\033[36m", // DEBUG: Cyan
};
static const char* resetColor = "\033[0m";

static const char* levelToString(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_INFO: return "INFO";
        case LOG_LEVEL_WARNING: return "WARNING";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

static void printTimestamp() {
    time_t t;
    time(&t);
    struct tm* localTime = localtime(&t);
    char buffer[20];
    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", localTime);
    printf("[%s] ", buffer);
}

// Initializes the logger
void initLogger(void) {
    // No initialization needed for console-only logging
    // If needed, additional setup could be done here (e.g., setting console colors)
}

void initConsole() {
#ifdef _WIN32
    if (isConsoleInitialized) return;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        DWORD dwMode;
        if (GetConsoleMode(hConsole, &dwMode)) {
            SetConsoleMode(hConsole, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        } else {
            fprintf(stderr, "Failed to get console mode\n");
        }
    } else {
        fprintf(stderr, "Failed to get console handle\n");
    }
    isConsoleInitialized = 1;
#endif
}

void logMessage(LogLevel level, const char* format, ...) {
    if (level < currentLogLevel) return;

    initConsole();

    printf("%s", levelColors[level]);
    printTimestamp();
    printf("[%s] ", levelToString(level));

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\n", resetColor);
}

// Sets the current log level
void setLogLevel(LogLevel level) {
    currentLogLevel = level;
}

void debugPrint(const char* file, int line) {
    printf("Called from file: %s, line: %d\n", file, line);
}
