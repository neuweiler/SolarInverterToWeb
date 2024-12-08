/*
 * WebServer.cpp
 *
 * Provides a small webserver to read out data as pure JSON or in a dashboard.
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#include "WebServer.h"

WebServer::WebServer() {
	uploadPath = "";
	server = new ESP8266WebServer(80);
}

WebServer::~WebServer() {
	delete server;
}

/**
 * Initialize the web server.
 */
void WebServer::init() {
	pinMode(PIN_LED_CLIENT_CONNECTED, OUTPUT);
	pinMode(PIN_POWER_OVERRIDE, INPUT);
	digitalWrite(PIN_LED_CLIENT_CONNECTED, LOW);

    server->addHandler(this);
	server->serveStatic("/", LittleFS, "/");
	server->begin();
	logger.info(F("started webserver"));
}

/**
 * The main processing logic.
 */
void WebServer::loop() {
	server->handleClient();
	digitalWrite(PIN_LED_CLIENT_CONNECTED, server->client().connected() ? HIGH : LOW);
}

/**
 * Find out if we can handle the request.
 */
bool WebServer::canHandle(HTTPMethod method, const String& uri) {
	if (logger.isDebug())
		logger.debug(F("http request: %d, url: %s"), method, uri.c_str());

	if (method == HTTP_GET && (uri.equals(F("/data")) || uri.equals(F("/list")) || uri.equals(F("/maxCurrent")))) {
		return true;
	}
	if (method == HTTP_POST && uri.equals(F("/grid"))) {
		return true;
	}
	return false;
}

bool WebServer::canUpload(const String& uri) {
	return (uri.equals(F("/upload")));
}

/**
 * Handle a request and send the inverter data.
 */
bool WebServer::handle(ESP8266WebServer& server, HTTPMethod requestMethod, const String& requestUri) {
	if (requestUri.equals(F("/data"))) {
		server.send(200, F("application/json"), inverter.toJSON());
	} else if (requestUri.equals(F("/maxCurrent"))) {
		uint16_t maxCurrent = digitalRead(PIN_POWER_OVERRIDE) == HIGH ? 0xffff : inverter.getMaximumSolarCurrent();
		server.send(200, F("application/json"), String(F("{\"maxCurrent\": ")) + maxCurrent + "}");
	} else if (requestUri.equals(F("/list"))) {
		handleFileList();
	} else if (requestUri.equals(F("/upload")) && requestMethod == HTTP_POST) {
		server.sendHeader(F("Location"), String("/list?dir=" + uploadPath), true);
		server.send(302, F("text/plain"), "");
	} else if (requestUri.equals(F("/grid"))) {
		inverter.switchToGrid();
		server.send(200, F("text/plain"), F("Switched to grid mode"));
	} else {
		return false;
	}

	return true;
}

/**
 * Receive a file.
 */
void WebServer::upload(ESP8266WebServer& server, const String& requestUri, HTTPUpload& upload) {
	if (upload.status == UPLOAD_FILE_START) {
		String filename = uploadPath + "/" + upload.filename.c_str();
		if (!filename.startsWith("/")) { // Make sure paths always start with "/"
			filename = "/" + filename;
		}
		fsUploadFile = LittleFS.open(filename, "w");
		if (!fsUploadFile) {
			return replyServerError(F("CREATE FAILED"));
		}
		logger.debug(F("Upload: START, filename: %s"), filename);
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		if (fsUploadFile) {
			size_t bytesWritten = fsUploadFile.write(upload.buf, upload.currentSize);
			if (bytesWritten != upload.currentSize) {
				return replyServerError(F("WRITE FAILED"));
			}
		}
		logger.debug(F("Upload: WRITE, Bytes: %d"), upload.currentSize);
	} else if (upload.status == UPLOAD_FILE_END) {
		if (fsUploadFile) {
			fsUploadFile.close();
		}
		logger.debug(F("Upload: END, Size: %d"), upload.totalSize);

		config.load(); // re-load the config from new file
	}
}

/**
 * Create file list and send as response.
 */
void WebServer::handleFileList() {
	String path = server->hasArg(F("dir")) ? server->arg(F("dir")) : "/";
	uploadPath = path;

	Dir dir = LittleFS.openDir(path);
	String output = F("<html><body>");
	output += F("<h3>") + path + F("</h3><table>");

	if (path.length() > 1) {
		String parent = path.substring(0, path.lastIndexOf('/'));
		if (parent.length() == 0) {
			parent = "/";
		}
		output += F("<tr><td colspan='4'><a href='/list?dir=") + parent + F("'>..</a></td></tr>");
	}
	if (!path.endsWith("/")) {
		path += "/";
	}

	while (dir.next()) {
		File file = dir.openFile("r");
		time_t time = file.getLastWrite();
		String prefix = (file.isDirectory() ? F("/list?dir=") : F(""));
		output += F("<tr>");
		output += F("<td><a href='") + prefix + path + file.name() + F("'>") + file.name() + F("</a></td>");
		output += F("<td style='text-align: right;'>") + (file.isDirectory() ? F("(dir)") : String(file.size()))
				+ F("</td>");
		output += F("<td style='text-align: right;'>") + String(ctime(&time)) + F("</td>");
		output += F("</tr>");
		file.close();
	}
	output += F("</table>");
	output += F("<form action='/upload' method='POST' enctype='multipart/form-data'>");
	output += F("Upload File: <input type='file' id='uploadFile' name='filename'>");
	output += F("<input type='submit' value='Upload'>");
	output += F("</form>");
	output += F("</body></html>");

	server->send(200, F("text/html"), output);
}

void WebServer::replyServerError(String msg) {
	logger.error(msg);
	server->send(500, F("text/plain"), msg + "\r\n");
}

WebServer webServer;
