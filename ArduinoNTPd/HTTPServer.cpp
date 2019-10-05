/*
 * File: HTTPServer.cpp
 * Description:
 *   HTTP server implementation.
 * Author: Mooneer Salem <mooneer@gmail.com>
 * License: New BSD License
 */

#if defined(ARDUINO)

#include <Ethernet.h>
#include "HTTPServer.h"

void HttpServer::beginListening()
{
    httpServerPort_.begin();
}

bool HttpServer::processOneRequest()
{
    bool processed = false;
    
    // Retrieve a connection if we're not already working on one.
    currentClient_ = httpServerPort_.available();
    
    // For the current connection, we'll want to read the first line,
    // pull out the request method and path and ignore everything else
    // up to the blank line.
    if (currentClient_)
    {
        while (currentClient_.connected())
        {
            if (currentClient_.available())
            {
                char c = currentClient_.read();
            
                if (c == '\n' && seenBlankLine_)
                {
                    // Finished reading the HTTP request.
                    routeRequest_();
                    resetServerState_();
                    processed = true;
                }
                else
                {
                    char tmp[2];
                    tmp[1] = 0;
                    tmp[0] = c;
                
                    if (c != '\r')
                    {
                        seenBlankLine_ = false;
                    }
                    
                    if (c == ' ')
                    {
                        // Split string based on delimiter.
                        currentSplit_++;
                    }
                    else if (c == '\n')
                    {
                        seenBlankLine_ = true;
                    }
                    else if (currentSplit_ == 0)
                    {
                        // Request method
                        httpMethod_ += String(tmp);
                    }
                    else if (currentSplit_ == 1)
                    {
                        // Request path.
                        requestPath_ += String(tmp);
                    }
                    else
                    {
                        // Ignore all other characters of request.
                    }
                }
            }
        }
    }
    
    return processed;
}

void HttpServer::responseRedirect(const char *url)
{
    currentClient_.println(F("HTTP/1.0 302 Found"));
    currentClient_.print(F("Location: "));
    currentClient_.println(url);
    currentClient_.println(F("Connection: close"));
    currentClient_.println(F("Content-Type: text/html"));
    currentClient_.println();
    currentClient_.print(F("See <a href=\""));
    currentClient_.print(url);
    currentClient_.print(F("\">"));
    currentClient_.print(url);
    currentClient_.println(F("</a> for new location."));
}

void HttpServer::routeRequest_()
{
    // We only support GET requests.
    if (httpMethod_ == "GET")
    {
        bool found = false;
        for (int index = 0; index < numUrls_; index++)
        {
            if (requestPath_ == urlHandlers_[index].path)
            {
                // Let valid handler handle this request.
                found = true;
                (*urlHandlers_[index].function)(this);
            }
        }
        
        if (!found)
        {
            // HTTP 404 Not Found
            sendHttpResponseHeaders_(404, "Not Found");
            currentClient_.println(F("Not found."));
        }
    }
    else
    {
        // HTTP 405 Method Not Allowed
        sendHttpResponseHeaders_(405, "Method Not Allowed");
        currentClient_.println(F("Not allowed."));
    }
}

void HttpServer::sendHttpResponseHeaders_(int code, const char *description)
{
    currentClient_.print(F("HTTP/1.0 "));
    currentClient_.print(code, DEC);
    currentClient_.print(F(" "));
    currentClient_.println(description);
    currentClient_.println(F("Connection: close"));
    currentClient_.println(F("Content-Type: text/html"));
    currentClient_.println();
}

#endif // defined(ARDUINO)
