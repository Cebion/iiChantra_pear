#include "StdAfx.h"
#include "highscores.h"
#include "../misc.h"
#include <string>
#include <map>
#include "http_client.h"

#define CHECKSUMS_ON

#ifdef CHECKSUMS_ON
#include <sstream>
#include "../crc32.h"
static const char HS_VALIDATION_URI[] = "/validate_client/";
#endif

static const size_t HS_NICKNAME_MAX_LEN = 16;
static const char HS_SALT[] = "UBW";
static const char HS_HOST[] = "highscores.iichantra.ru";
static const char HS_SUBMIT_URI[] =  "/submit/";
// Это значение нужно менять с каждым релизом
// Чтобы хайскоры записывались в таблицу привязанную к этому релизу
// Чексуммы тоже будут привязаны к названию релиза
static const char HS_TABLE[] = "2011_winter";

using namespace std;



std::string hsGetValidationList()
{
        std::string content;

        map<string, string> params;
        params.insert(pair<string, string>("version", std::string(HS_TABLE)));

        bool send_result = http_send_request(HS_HOST, HS_VALIDATION_URI, content, "POST", NULL, &params);

        if(!send_result) {
                sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Cannot get validation list: http_send_request failed");
                return ""; // return empty string instead of false
        }

        return content;
}

bool hsSendScore(const char* nickname, const int score, const int seconds)
{

	if(score < 0) {
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Cannot send highscores: score must be positive or zero");
		return false;
	}

	if(seconds < 0) {
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Cannot send highscores: seconds must be positive");
		return false;
	}

	if(strlen(nickname) > HS_NICKNAME_MAX_LEN) {
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Cannot send highscores: nickname is too long");
		return false;
	}

#ifdef CHECKSUMS_ON
	// Получаем список файлов на валидацию
	// Будем считать crc32
	std::string validation_list = hsGetValidationList();
	stringstream stream (stringstream::in | stringstream::out);
	stream << validation_list;

	//char checksum_buffer[256];
	//std::string checksum_string;
	stringstream checksum (stringstream::in | stringstream::out);
	while(!stream.eof()) {
		string line;
		getline(stream, line);

		if(line.empty() || line == "\n")
			continue;

		uint32_t file_crc32 = crc32f(line.c_str());
		if(file_crc32 != 0xFFFFFFFF && file_crc32 != 0x00000000) {
			
			if(!checksum.str().empty())
				checksum << "|";

			checksum << line << ":" << file_crc32;
		}
	}
#endif

	stringstream token (stringstream::in | stringstream::out);
	token << nickname << "$$$";
	token << score << "$$$";
	token << seconds << "$$$";
	token << HS_TABLE << "$$$";
#ifdef CHECKSUMS_ON
	token << checksum.str();
	//sLog(DEFAULT_LOG_NAME, logLevelInfo,  checksum.str().c_str());
#endif

	string out;
	for (unsigned int x = 0; x < token.str().length(); x++) {
		out += token.str()[x] ^ HS_SALT[x % strlen(HS_SALT)];
	}
	string tmp = token.str();

	map<string, string> params;
	params.insert(pair<string, string>("token", out));

	std::string content;

	bool send_result = http_send_request(HS_HOST, HS_SUBMIT_URI, content, "POST", NULL, &params);

	if(!send_result) {
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Cannot send highscores: http_send_request failed");
		return false;
	}

	if(content != "OK") {
		sLog(DEFAULT_LOG_NAME, LOG_ERROR_EV, "Cannot send highscores: server response is %s", content.c_str());
		return false;
	}

	sLog(DEFAULT_LOG_NAME, logLevelInfo, "Highscores succesfully send");
	return true;
}
