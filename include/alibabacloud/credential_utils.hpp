#ifndef ALIBABACLOUD_CREDENTIAL_UTILS_H_
#define ALIBABACLOUD_CREDENTIAL_UTILS_H_

#include <alibabacloud/credential.hpp>
#include <boost/algorithm/string/join.hpp>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <json/json.h>
#include <map>
#include <sstream>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <wincrypt.h>
#include <windows.h>
#else

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <openssl/hmac.h>

#endif

using namespace std;
using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
using namespace boost::algorithm;

void pushMockHttpResponse(const http_response &res);

bool hasMockHttpResponse();

http_response popMockHttpResponse();

string lowercase(string data);

Json::Value json_decode(const string &rawJson);

string hmacSha1(const std::string &src, const std::string &secret);

string url_encode(const string &value);

string httpQueryString(map<string, string> query);

string composeUrl(const string &host, const map<string, string> &params,
                  const string &pathname, const string &schema,
                  const string &port);

string stringToSign(const method &mtd, map<string, string> query);

string stsRequest(const string &accessKeySecret, const method &mtd,
                  map<string, string> query);

string uuid();

string gmt_datetime();

long long strtotime(const string &gmt_datetime);

#endif
