#ifndef ALIBABACLOUD_CREDENTIAL_UTILS_H_
#define ALIBABACLOUD_CREDENTIAL_UTILS_H_

#include <alibabacloud/credential.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/date_time.hpp>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <json/json.h>
#include <map>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <wincrypt.h>
#include <windows.h>
#else

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <list>
#include <openssl/hmac.h>

#endif

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

string hmacSha1(const std::string &src, const std::string &secret) {
  if (src.empty())
    return std::string();

#ifdef _WIN32
  typedef struct _my_blob {
    BLOBHEADER hdr;
    DWORD dwKeySize;
    BYTE rgbKeyData[];
  } my_blob;

  DWORD kbLen = sizeof(my_blob) + secret.size();
  my_blob *kb = (my_blob *)LocalAlloc(LPTR, kbLen);
  kb->hdr.bType = PLAINTEXTKEYBLOB;
  kb->hdr.bVersion = CUR_BLOB_VERSION;
  kb->hdr.reserved = 0;
  kb->hdr.aiKeyAlg = CALG_RC2;
  kb->dwKeySize = secret.size();
  memcpy(&kb->rgbKeyData, secret.c_str(), secret.size());

  HCRYPTPROV hProv = 0;
  HCRYPTKEY hKey = 0;
  HCRYPTHASH hHmacHash = 0;
  BYTE pbHash[32];
  DWORD dwDataLen = 32;
  HMAC_INFO HmacInfo;
  ZeroMemory(&HmacInfo, sizeof(HmacInfo));
  HmacInfo.HashAlgid = CALG_SHA1;

  CryptAcquireContext(&hProv, NULL, MS_ENHANCED_PROV, PROV_RSA_FULL,
                      CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET);
  CryptImportKey(hProv, (BYTE *)kb, kbLen, 0, CRYPT_IPSEC_HMAC_KEY, &hKey);
  CryptCreateHash(hProv, CALG_HMAC, hKey, 0, &hHmacHash);
  CryptSetHashParam(hHmacHash, HP_HMAC_INFO, (BYTE *)&HmacInfo, 0);
  CryptHashData(hHmacHash, (BYTE *)(src.c_str()), src.size(), 0);
  CryptGetHashParam(hHmacHash, HP_HASHVAL, pbHash, &dwDataLen, 0);

  LocalFree(kb);
  CryptDestroyHash(hHmacHash);
  CryptDestroyKey(hKey);
  CryptReleaseContext(hProv, 0);

  DWORD dlen = 0;
  CryptBinaryToString(pbHash, dwDataLen,
                      CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &dlen);
  char *dest = new char[dlen];
  CryptBinaryToString(pbHash, dwDataLen,
                      CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, dest, &dlen);

  std::string ret = std::string(dest, dlen);
  delete dest;
  return ret;
#else
  unsigned char md[EVP_MAX_BLOCK_LENGTH];
  unsigned int mdLen = EVP_MAX_BLOCK_LENGTH;

  if (HMAC(EVP_sha1(), secret.c_str(), secret.size(),
           reinterpret_cast<const unsigned char *>(src.c_str()), src.size(), md,
           &mdLen) == nullptr)
    return std::string();

  char encodedData[100];
  EVP_EncodeBlock(reinterpret_cast<unsigned char *>(encodedData), md, mdLen);
  return encodedData;
#endif
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

string stsRequest(const string &accessKeySecret, const method &mtd,
                  map<string, string> query) {
  string strToSign = stringToSign(mtd, query);
  query.insert(
      pair<string, string>("Signature", hmacSha1(strToSign, accessKeySecret)));

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
  struct tm tm{};
  setenv("TZ", "UTC", 1);
  strptime(gmt_datetime.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm);
  time_t t = mktime(&tm);
  return boost::lexical_cast<long long>(t);
}

#endif
