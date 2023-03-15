#include "StdAfx.h"
#include "http_client.h"
#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "SDL/SDL_net.h"
#include "../misc.h"
#include "../defines.h"

#define HTTP_PORT 80 // Thanks, cap!

/* Юзер агент используется в highscores. Когда сервер получает запрос он проверяет юзерагент.
   Можно было бы оставить просто iichantra, но такой юзерагент было бы слишком просто подобрать */
#define HTTP_USER_AGENT "iichantra/SDL"

#define HTTP_RESPONSE_BUFFER_SIZE 10192

// @see http://www.zedwood.com/article/111/cpp-urlencode-function
std::string char2hex( char dec )
{
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=97-10;

    std::string r;
    r.append( &dig1, 1);
    r.append( &dig2, 1);
    return r;
}

// @see http://www.zedwood.com/article/111/cpp-urlencode-function
std::string urlencode(const std::string &c)
{
    
    std::string escaped="";
    int max = c.length();
    for(int i=0; i<max; i++)
    {
        if ( (48 <= c[i] && c[i] <= 57) ||//0-9
             (65 <= c[i] && c[i] <= 90) ||//abc...xyz
             (97 <= c[i] && c[i] <= 122) || //ABC...XYZ
             (c[i]=='~' || c[i]=='!' || c[i]=='*' || c[i]=='(' || c[i]==')' || c[i]=='\'')
        )
        {
            escaped.append( &c[i], 1);
        }
        else
        {
            escaped.append("%");
            escaped.append( char2hex(c[i]) );//converts char 255 to string "ff"
        }
    }
    return escaped;
}

/* На момент вызова этих функций SDL и SDLnet уже должны быть инициализированы */

// Пример использования:
// http_send_request("127.0.0.1", "/index.php", content) 
bool http_send_request(const std::string host, const std::string uri, std::string& content, const char method[] = "GET", std::map<std::string, std::string>* headers = NULL, std::map<std::string, std::string>* params = NULL)
{
	IPaddress ip;
	TCPsocket sock;
	Uint16 port;

	port = (Uint16)HTTP_PORT;

	if(SDLNet_ResolveHost(&ip, host.c_str(), port) == -1) {
		sLog(DEFAULT_NET_LOG_NAME, LOG_ERROR_EV, "Failed to resolve host %s", host.c_str());
		return false;
	}

	sock = SDLNet_TCP_Open(&ip);
	if(!sock) {
		sLog(DEFAULT_NET_LOG_NAME, LOG_ERROR_EV, "Failed to open TCP socket to host %s on port %d. Error: %s", host.c_str(), HTTP_PORT, SDLNet_GetError());
		return false;
	}

	if(method[0] == 'G' && params != NULL) {
		method = "POST";
	}

	stringstream msg (stringstream::in | stringstream::out);
	// Нельзя посылать HTTP/1.1, потому что ответ приходит в чанках, и это гемор.
	// В 1.0 мы сразу знаем длину контента (из заголовка Content-Length)
	// PS. По крайней мере это справедливо для apache/mod_Wsgi/django, больше нигде не проверял да и лень
	// В джанге еще приходится вручную Content-Length указывать -_-
	msg << method << " " << uri << " HTTP/1.0\r\n";
	msg << "Host: " << host << "\r\n";
	msg << "User-Agent: " << HTTP_USER_AGENT << "\r\n";
	if(headers != NULL) {
		std::map<std::string, std::string>::iterator it;

		for(it = headers->begin(); it != headers->end(); it++) {
			msg << it->first << ": " << it->second << "\r\n";
		}	
	}

	if(params != NULL) {
		msg << "Content-Type: application/x-www-form-urlencoded\r\n";

		std::map<std::string, std::string>::iterator it;
		std::string content;

		for(it = params->begin(); it != params->end(); it++) {
			if(it != params->begin()) {
				content += "&";
			}
			content += it->first + "=" + urlencode(it->second);
		}

		msg << "Content-Length: " << content.length() << "\r\n";
		msg << "\r\n";
		msg << content;
	} else {
		msg << "\r\n";
	}

	std::string tmp = msg.str();

	int result = SDLNet_TCP_Send(sock, msg.str().c_str(), msg.str().length());

	if((unsigned)result < msg.str().length()) {
		sLog(DEFAULT_NET_LOG_NAME, LOG_ERROR_EV, "Failed to send message. Response length: %d", result);
		SDLNet_TCP_Close(sock);
		return false;
	}

	// Получаем результат
	char response_buffer[HTTP_RESPONSE_BUFFER_SIZE];
	memset(response_buffer, '\0', HTTP_RESPONSE_BUFFER_SIZE);
	result = SDLNet_TCP_Recv(sock, response_buffer, HTTP_RESPONSE_BUFFER_SIZE-1);

	if(result < 0) {
		sLog(DEFAULT_NET_LOG_NAME, LOG_ERROR_EV, "Failed to get http response");
		return false;
	}

	/* Типовой ответ от сервера:
		HTTP/1.1 200 OK
		Date: Thu, 06 Jan 2011 00:05:56 GMT
		Server: Apache/2.2.16 (FreeBSD) DAV/2 mod_wsgi/3.2 Python/2.6.6 PHP/5.3.2 with Suhosin-Patch mod_ssl/2.2.16 OpenSSL/0.9.8k
		Content-Length: 2
		Connection: close
		Content-Type: text/plain

		OK
	*/
	 stringstream response (stringstream::in | stringstream::out);

	 response << response_buffer;
	 content = "";
	 bool content_flag = false;

	 while(!response.eof()) {
		 string line;
		 getline(response, line);
		 
		 if(startswith(line, "HTTP") && !endswith(line, "200 OK\r")) {
			 sLog(DEFAULT_NET_LOG_NAME, LOG_ERROR_EV, "Expected 200 OK but received %s", line.c_str());
			 return false;
		 }

		 if(line == "\r") {
			 // Теперь начинается контент
			 content_flag = true;
		 }

		 if(content_flag) {
			 if(!content.empty()) {
				 // Добавляем перенос строки
				 content += "\n";
			 }

			 if(line != "\r")
				content += line;
		 }
	}

	SDLNet_TCP_Close(sock);

	return true;
}
