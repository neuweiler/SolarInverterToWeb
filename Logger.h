/*
 * Logger.h
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */


#ifndef LOGGER_H_
#define LOGGER_H_

#include <Arduino.h>

#define LOG_BUFFER_SIZE 160

class Logger
{
public:
    enum LogLevel
    {
        Debug = 0,
        Info = 1,
        Warn = 2,
        Error = 3,
        Off = 4
    };
    void init();
    void debug(String, ...);
    void info(String, ...);
    void warn(String, ...);
    void error(String, ...);
    void console(String, ...);
    void setLoglevel(LogLevel);
    LogLevel getLogLevel();
    boolean isDebug();
private:
    LogLevel logLevel;
    bool debugging;
    char *msgBuffer;

    void log(LogLevel, String format, va_list);
};

extern Logger logger;

#endif /* LOGGER_H_ */
