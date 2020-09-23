#include "crypt/hmac.h"
#include "crypt/sha1.h"
#include <alibabacloud/credential.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <curl/curl.h>
#include <iostream>
#include <json/json.h>
#include <map>

using namespace std;
using namespace Alibabacloud_Credential;
using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

/*** <<<<<<<<<  helper  >>>>>>>> ***/
bool Credential::has_expired() const {
  long now = static_cast<long int>(time(nullptr));
  return _expiration - now <= 180;
}

std::string hmacsha1(const std::string &src, const std::string &key) {
  boost::uint8_t hash_val[sha1::HASH_SIZE];
  hmac<sha1>::calc(src, key, hash_val);
  return base64::encode_from_array(hash_val, sha1::HASH_SIZE);
}

Json::Value json_decode(const string &str) {
  const auto len = static_cast<int>(str.length());
  JSONCPP_STRING err;
  Json::Value root;

  Json::CharReaderBuilder builder;
  const unique_ptr<Json::CharReader> reader(builder.newCharReader());
  if (!reader->parse(str.c_str(), str.c_str() + len, &root, &err)) {
    string error_info = "json_decode error : " + err;
    throw error_info;
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

std::string implode(const std::vector<std::string> &vec,
                    const std::string &glue) {
  string res;
  int n = 0;
  for (const auto &str : vec) {
    if (n == 0) {
      res = str;
    } else {
      res += glue + str;
    }
    n++;
  }
  return res;
}

string build_http_query_str(map<string, string> query) {
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
    str = implode(arr, "&");
  }
  return str;
}

long long strtotime(const string &gmt_datetime) {
  struct tm tm {};
  strptime(gmt_datetime.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm);
  time_t t = timegm(&tm);
  return boost::lexical_cast<long long>(t);
}

string gmt_datetime() {
  time_t now;
  time(&now);
  char buf[20];
  strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
  return buf;
}

string uuid() {
  boost::uuids::uuid uid = boost::uuids::random_generator()();
  return boost::uuids::to_string(uid);
}

string lowercase(string data) {
  std::transform(data.begin(), data.end(), data.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return data;
}

string compose_url(const string &host, const map<string, string> &params,
                   const string &pathname, const string &schema,
                   const string &port) {
  string url = schema + "://" + host;
  url = lowercase(url);
  if (port != "80" && port != "443") {
    url = url + ":" + port;
  }
  url = url + pathname;
  if (!params.empty()) {
    string queryString = build_http_query_str(params);
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

string sign_string(const method &mtd, map<string, string> query) {
  vector<string> params;
  params.push_back(mtd);
  params.push_back(url_encode("/"));
  params.push_back(url_encode(build_http_query_str(std::move(query))));
  return implode(params, "&");
}

/*** <<<<<<<<<  Credential  >>>>>>>> ***/
Credential::Credential() {}

Credential::Credential(Config *config) {
  _config = config;
  _credentialType = config->type;
}

Credential::~Credential() { delete _config; }

string Credential::requestSTS(string accessKeySecret, web::http::method mtd,
                              map<string, string> query) {
  string strToSign = sign_string(mtd, query);

  query.insert(
      pair<string, string>("Signature", hmacsha1(strToSign, accessKeySecret)));

  string url = compose_url("sts.aliyuncs.com", query, "", "https", "80");
  http_client client(uri(conversions::to_string_t(url)));
  http_response response = request(url);
  if (nullptr != getenv("DEBUG")) {
    printf("status_code : %hu\n", response.status_code());
    printf("response : %s\n", response.to_string().c_str());
  }
  if (response.status_code() != status_codes::OK) {
    Alibabacloud_Credential::RefreshCredentialException rce(
        "Failed request STS service.");
    rce.setHttpResponse(response);
    throw rce;
  }
  concurrency::streams::stringstreambuf buffer;
  response.body().read_to_end(buffer).get();
  string &data = buffer.collection();
  return data;
}

web::http::http_response Credential::request(const string &url) {
  http_client client(uri(conversions::to_string_t(url)));
  return client.request(methods::GET).get();
}

/*** <<<<<<<<<  AccessKeyCredential  >>>>>>>> ***/
AccessKeyCredential::AccessKeyCredential(Config *config) : Credential(config){};

string AccessKeyCredential::getAccessKeyId() { return *_config->accessKeyId; }

string AccessKeyCredential::getAccessKeySecret() {
  return *_config->accessKeySecret;
}

/*** <<<<<<<<<  BearerTokenCredential  >>>>>>>> ***/
BearerTokenCredential::BearerTokenCredential(Config *config)
    : Credential(config){};

string BearerTokenCredential::getBearerToken() { return *_config->bearerToken; }

/*** <<<<<<<<<  StsCredential  >>>>>>>> ***/
StsCredential::StsCredential(Config *config) : Credential(config) {}

string StsCredential::getAccessKeyId() { return *_config->accessKeyId; }

string StsCredential::getAccessKeySecret() { return *_config->accessKeySecret; }

string StsCredential::getSecurityToken() { return *_config->securityToken; }

/*** <<<<<<<<<  EcsRamRoleCredential  >>>>>>>> ***/
EcsRamRoleCredential::EcsRamRoleCredential(Config *config)
    : Credential(config) {}

string EcsRamRoleCredential::getAccessKeyId() {
  refresh();
  return *_config->accessKeyId;
}

string EcsRamRoleCredential::getAccessKeySecret() {
  refresh();
  return *_config->accessKeySecret;
}

string EcsRamRoleCredential::getSecurityToken() {
  refresh();
  return *_config->securityToken;
}

void EcsRamRoleCredential::refresh() {
  if (has_expired()) {
    refreshRam();
  }
}

void EcsRamRoleCredential::refreshRam() {
  if (nullptr == _config->roleName || _config->roleName->empty()) {
    string url =
        (string) "https://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA;
    http_client client(uri(conversions::to_string_t(url)));
    http_response response = request(url);
    concurrency::streams::stringstreambuf buffer;
    if (response.status_code() == status_codes::OK) {
      response.body().read_to_end(buffer).get();
      _config->roleName = new string(buffer.collection());
      refreshCredential();
    } else {
      auto body = response.extract_string();
      RefreshCredentialException rce(body.get());
      rce.setHttpResponse(response);
      throw rce;
    }
  } else {
    refreshCredential();
  }
}

void EcsRamRoleCredential::refreshCredential() {
  string role_name = nullptr == _config->roleName ? "" : *_config->roleName;
  string url =
      "https://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA + role_name;
  http_client client(uri(conversions::to_string_t(url)));
  http_response response = request(url);
  concurrency::streams::stringstreambuf buffer;
  if (response.status_code() == status_codes::OK) {
    response.body().read_to_end(buffer).get();
    string data = buffer.collection();
    Json::Value result = json_decode(data);
    string code = result["Code"].asString();
    if (code == "Success") {
      _config->accessKeyId = new string(result["AccessKeyId"].asString());
      _config->accessKeySecret =
          new string(result["AccessKeySecret"].asString());
      _config->securityToken = new string(result["SecurityToken"].asString());
      _expiration = strtotime(result["Expiration"].asString());
    } else {
      RefreshCredentialException rce(ECS_META_DATA_FETCH_ERROR_MSG);
      rce.setHttpResponse(response);
      throw rce;
    }
  } else {
    RefreshCredentialException rce(ECS_META_DATA_FETCH_ERROR_MSG);
    rce.setHttpResponse(response);
    throw rce;
  }
}

/*** <<<<<<<<<  RamRoleArnCredential  >>>>>>>> ***/
RamRoleArnCredential::RamRoleArnCredential(Config *config)
    : Credential(config) {
  if (nullptr != _config->roleSessionName &&
      _config->roleSessionName->empty()) {
    _config->roleSessionName = new string("defaultSessionName");
  }
};

string RamRoleArnCredential::getAccessKeyId() {
  refresh();
  return *_config->accessKeyId;
}

string RamRoleArnCredential::getAccessKeySecret() {
  refresh();
  return *_config->accessKeySecret;
}

string RamRoleArnCredential::getSecurityToken() {
  refresh();
  return *_config->securityToken;
}

string RamRoleArnCredential::getRoleArn() {
  refresh();
  return *_config->roleArn;
}

string RamRoleArnCredential::getPolicy() {
  refresh();
  return *_config->policy;
}

void RamRoleArnCredential::refresh() {
  if (has_expired()) {
    refreshCredential();
  }
}

void RamRoleArnCredential::refreshCredential() {
  map<string, string> params;
  int duration_seconds =
      nullptr == _config->durationSeconds ? 0 : *_config->durationSeconds;
  string roleArn = nullptr == _config->roleArn ? "" : *_config->roleArn;
  string accessKeyId =
      nullptr == _config->accessKeyId ? "" : *_config->accessKeyId;
  string roleSessionName =
      nullptr == _config->roleSessionName ? "" : *_config->roleSessionName;
  string policy = nullptr == _config->policy ? "" : *_config->policy;

  params.insert(pair<string, string>("Action", "AssumeRole"));
  params.insert(pair<string, string>("Format", "JSON"));
  params.insert(pair<string, string>("Version", "2015-04-01"));
  params.insert(
      pair<string, string>("DurationSeconds", to_string(duration_seconds)));
  params.insert(pair<string, string>("RoleArn", roleArn));
  params.insert(pair<string, string>("AccessKeyId", accessKeyId));
  params.insert(pair<string, string>("RegionId", _regionId));
  params.insert(pair<string, string>("RoleSessionName", roleSessionName));
  params.insert(pair<string, string>("SignatureVersion", "1.0"));
  params.insert(pair<string, string>("SignatureMethod", "HMAC-SHA1"));
  params.insert(pair<string, string>("Timestamp", gmt_datetime()));
  params.insert(pair<string, string>("SignatureNonce", uuid()));
  if (!policy.empty()) {
    params.insert(pair<string, string>("Policy", policy));
  }
  string json = requestSTS(*_config->accessKeySecret, methods::GET, params);
  Json::Value result = json_decode(json);
  string code = result["Code"].asString();
  if (code == "Success") {
    _config->accessKeyId =
        new string(result["Credentials"]["AccessKeyId"].asString());
    _config->accessKeySecret =
        new string(result["Credentials"]["AccessKeySecret"].asString());
    _expiration = strtotime(result["Credentials"]["Expiration"].asString());
    _config->securityToken =
        new string(result["Credentials"]["SecurityToken"].asString());
  } else {
    RefreshCredentialException rce(
        "Failed to get Security Token from STS service.");
    throw rce;
  }
}

/*** <<<<<<<<<  RsaKeyPairCredential  >>>>>>>> ***/
RsaKeyPairCredential::RsaKeyPairCredential(Config *config)
    : Credential(config) {}

string RsaKeyPairCredential::getPublicKeyId() {
  refresh();
  return *_config->publicKeyId;
}

string RsaKeyPairCredential::getPrivateKeySecret() {
  refresh();
  return *_config->privateKeyFile;
}

string RsaKeyPairCredential::getAccessKeyId() {
  refresh();
  return *_config->accessKeyId;
}

string RsaKeyPairCredential::getAccessKeySecret() {
  refresh();
  return *_config->accessKeySecret;
}

string RsaKeyPairCredential::getSecurityToken() {
  refresh();
  return *_config->securityToken;
}

void RsaKeyPairCredential::refresh() {
  if (has_expired()) {
    refreshCredential();
  }
}

void RsaKeyPairCredential::refreshCredential() {
  map<string, string> params;
  int duration_seconds =
      nullptr == _config->durationSeconds ? 0 : *_config->durationSeconds;
  string access_key_id =
      nullptr == _config->publicKeyId ? "" : *_config->publicKeyId;
  params.insert(pair<string, string>("Action", "GenerateSessionAccessKey"));
  params.insert(pair<string, string>("Format", "JSON"));
  params.insert(pair<string, string>("Version", "2015-04-01"));
  params.insert(
      pair<string, string>("DurationSeconds", to_string(duration_seconds)));
  params.insert(pair<string, string>("AccessKeyId", access_key_id));
  params.insert(pair<string, string>("RegionId", "cn-hangzhou"));
  params.insert(pair<string, string>("SignatureVersion", "1.0"));
  params.insert(pair<string, string>("SignatureMethod", "HMAC-SHA1"));
  params.insert(pair<string, string>("Timestamp", gmt_datetime()));
  params.insert(pair<string, string>("SignatureNonce", uuid()));
  string secret =
      nullptr == _config->privateKeySecret ? "" : *_config->privateKeySecret;
  string json = requestSTS(secret, methods::GET, params);
  Json::Value result = json_decode(json);
  string code = result["Code"].asString();
  if (code == "Success") {
    _config->accessKeyId =
        new string(result["Credentials"]["AccessKeyId"].asString());
    _config->accessKeySecret =
        new string(result["Credentials"]["AccessKeySecret"].asString());
    _expiration = strtotime(result["Credentials"]["Expiration"].asString());
    _config->securityToken =
        new string(result["Credentials"]["SecurityToken"].asString());
  } else {
    RefreshCredentialException rce(
        "Failed to get Security Token from STS service.");
    throw rce;
  }
}