/*
 * Logger.cpp
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#include "Logger.h"

// uncomment next line to route logging output to Serial/USB/UART0 - will not work with attached inverter, only for dec purposes
#define Serial1 Serial

/*
 * NOTE: Serial on Wemos D1 mini uses UART0 of ESP8266, which is mapped to pins TX (GPIO1) and RX (GPIO3).
 * Serial1 uses UART1 which is a transmit-only UART. UART1 TX pin is D4 (GPIO2, LED).
 *
 * You can also use pins D7 and D8 (= GPIO 13 / RXD2 or GPIO 15 / TXD2) instead. All you have to do is insert the
 * instruction Serial.swap() after Serial.begin() in the setup.
 */

void Logger::init() {
	Serial1.begin(115200); // debug port = D4, LED, GPIO2 (not the USB port!)
	logLevel = Debug;
	debugging = true;
	msgBuffer = new char[LOG_BUFFER_SIZE];
}

/*
 * Output a debug message with a variable amount of parameters.
 * printf() style, see Logger::log()
 *
 */
void Logger::debug(String message, ...) {
	if (logLevel > Debug) {
		return;
	}

	va_list args;
	va_start(args, message);
	Logger::log(Debug, message, args);
	va_end(args);
}

/*
 * Output a info message with a variable amount of parameters
 * printf() style, see Logger::log()
 */
void Logger::info(String message, ...) {
	if (logLevel > Info) {
		return;
	}

	va_list args;
	va_start(args, message);
	Logger::log(Info, message, args);
	va_end(args);
}

/*
 * Output a warning message with a variable amount of parameters
 * printf() style, see Logger::log()
 */
void Logger::warn(String message, ...) {
	if (logLevel > Warn) {
		return;
	}

	va_list args;
	va_start(args, message);
	Logger::log(Warn, message, args);
	va_end(args);
}

/*
 * Output a error message with a variable amount of parameters
 * printf() style, see Logger::log()
 */
void Logger::error(String message, ...) {
	if (logLevel > Error) {
		return;
	}

	va_list args;
	va_start(args, message);
	Logger::log(Error, message, args);
	va_end(args);
}

/*
 * Output a comnsole message with a variable amount of parameters
 * printf() style, see Logger::logMessage()
 */
void Logger::console(String message, ...) {
	va_list args;
	va_start(args, message);
	vsnprintf(msgBuffer, LOG_BUFFER_SIZE, message.c_str(), args);
	Serial1.println(msgBuffer);
	va_end(args);
}

/*
 * Set the log level. Any output below the specified log level will be omitted.
 * Also set the debugging flag for faster evaluation in isDebug().
 */
void Logger::setLoglevel(LogLevel level) {
	logLevel = level;
	debugging = (level == Debug);
}

Logger::LogLevel Logger::getLogLevel() {
	return logLevel;
}

/*
 * Returns if debug log level is enabled. This can be used in time critical
 * situations to prevent unnecessary string concatenation (if the message won't
 * be logged in the end).
 *
 * Example:
 * if (Logger::isDebug()) {
 *    Logger::debug("current time: %d", millis());
 * }
 */
boolean Logger::isDebug() {
	return debugging;
}

/*
 * Output a log message (called by debug(), info(), warn(), error(), console())
 *
 * Supports printf() syntax
 */
void Logger::log(LogLevel level, String format, va_list args) {
	String logLevel;

	switch (level) {
	case Info:
		logLevel = F("INFO");
		break;
	case Warn:
		logLevel = F("WARNING");
		break;
	case Error:
		logLevel = F("ERROR");
		break;
	default:
		logLevel = F("DEBUG");
		break;
	}
	vsnprintf(msgBuffer, LOG_BUFFER_SIZE, format.c_str(), args);

	// print to serial
	Serial1.print(millis());
	Serial1.print(F(" - "));
	Serial1.print(logLevel);
	Serial1.print(F(": "));
	Serial1.println(msgBuffer);
}

Logger logger;
