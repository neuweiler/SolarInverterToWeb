/*
 * Logger.h
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */


#ifndef LOGGER_H_
#define LOGGER_H_

#include <Arduino.h>

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
    static void init();
    static void debug(String, ...);
    static void info(String, ...);
    static void warn(String, ...);
    static void error(String, ...);
    static void console(String, ...);
    static void setLoglevel(LogLevel);
    static LogLevel getLogLevel();
    static boolean isDebug();
private:
    static LogLevel logLevel;
    static bool debugging;
    static char *msgBuffer;

    static void log(LogLevel, String format, va_list);
};

#endif /* LOGGER_H_ */
