#ifndef ALIBABACLOUD_CREDENTIAL_UTILS_H_
#define ALIBABACLOUD_CREDENTIAL_UTILS_H_

#include "crypt/hmac.h"
#include "crypt/sha1.h"
#include <alibabacloud/credential.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/date_time.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <json/json.h>
#include <map>
#include <utility>
#include <vector>

using namespace std;
using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
using namespace boost::algorithm;

list<http_response> mock_http_response;

void pushMockHttpResponse(const http_response &res) {
  mock_http_response.push_front(res);
}

bool hasMockHttpResponse() { return !mock_http_response.empty(); }

http_response popMockHttpResponse() {
  http_response res = mock_http_response.front();
  mock_http_response.pop_front();
  return res;
}

string lowercase(string data) {
  std::transform(data.begin(), data.end(), data.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return data;
}

Json::Value json_decode(const string &rawJson) {
  const auto rawJsonLength = static_cast<int>(rawJson.length());
  JSONCPP_STRING err;
  Json::Value root;

  Json::CharReaderBuilder builder;
  const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  if (!reader->parse(rawJson.c_str(), rawJson.c_str() + rawJsonLength, &root,
                     &err)) {
    std::cout << "error" << std::endl;
    return EXIT_FAILURE;
  }
  return root;
}

string url_encode(const string &value) {
  ostringstream escaped;
  escaped.fill('0');
  escaped << hex;

  for (char c : value) {
    // Keep alphanumeric and other accepted characters intact
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped << c;
      continue;
    }

    // Any other characters are percent-encoded
    escaped << uppercase;
    escaped << '%' << setw(2) << int((unsigned char)c);
    escaped << nouppercase;
  }

  return escaped.str();
}

string httpQueryString(map<string, string> query) {
  string str;
  typedef map<string, string>::iterator MapIterator;
  if (!query.empty()) {
    vector<string> keys;
    keys.reserve(query.size());
    for (auto &it : query) {
      keys.push_back(it.first);
    }
    sort(keys.begin(), keys.end());
    vector<string> arr;
    for (const auto &k : keys) {
      if (query[k].empty()) {
        continue;
      }
      string tmp = k + "=" + url_encode(query[k]);
      arr.push_back(tmp);
    }
    str = join(arr, "&");
  }
  return str;
}

string composeUrl(const string &host, const map<string, string> &params,
                  const string &pathname, const string &schema,
                  const string &port) {
  string url = schema + "://" + host;
  url = lowercase(url);
  if (port != "80" && port != "443") {
    url = url + ":" + port;
  }
  url = url + pathname;
  if (!params.empty()) {
    string queryString = httpQueryString(params);
    if (url.find('?') != string::npos) {
      if (url.back() != '&') {
        url = url + "&" + queryString;
      } else {
        url = url + queryString;
      }
    } else {
      url = url + "?" + queryString;
    }
  }
  return url;
}

string stringToSign(const method &mtd, map<string, string> query) {
  vector<string> params;
  params.push_back(mtd);
  params.push_back(url_encode("/"));
  params.push_back(url_encode(httpQueryString(std::move(query))));
  return join(params, "&");
}

std::string hmacsha1(const std::string &src, const std::string &key) {
  boost::uint8_t hash_val[sha1::HASH_SIZE];
  hmac<sha1>::calc(src, key, hash_val);
  return base64::encode_from_array(hash_val, sha1::HASH_SIZE);
}

string stsRequest(const string &accessKeySecret, const method &mtd,
                  map<string, string> query) {
  string strToSign = stringToSign(mtd, query);
  query.insert(
      pair<string, string>("Signature", hmacsha1(strToSign, accessKeySecret)));

  string url = composeUrl("sts.aliyuncs.com", query, "", "https", "80");
  http_client client(uri(conversions::to_string_t(url)));
  http_response response;
  if (hasMockHttpResponse()) {
    response = popMockHttpResponse();
  } else {
    response = client.request(methods::GET).get();
  }
  if (nullptr != getenv("DEBUG")) {
    printf("status_code : %hu\n", response.status_code());
    printf("response : %s\n", response.to_string().c_str());
  }
  if (response.status_code() == status_codes::OK) {
    concurrency::streams::stringstreambuf buffer;
    response.body().read_to_end(buffer).get();
    string &data = buffer.collection();
    return data;
  }
  Alibabacloud_Credential::RefreshCredentialException rce(
      "Failed request STS service.");
  rce.setHttpResponse(response);
  throw rce;
}

string uuid() {
  boost::uuids::uuid uid = boost::uuids::random_generator()();
  return boost::uuids::to_string(uid);
}

string gmt_datetime() {
  time_t now;
  time(&now);
  char buf[20];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
  return buf;
}

long long strtotime(const string &gmt_datetime) {
  struct tm tm {};
  strptime(gmt_datetime.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm);
  time_t t = timegm(&tm);
  return boost::lexical_cast<long long>(t);
}

#endif
