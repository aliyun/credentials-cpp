#include "crypt/hmac.h"
#include "crypt/sha1.h"
#include <regex>
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

string hmacsha1(const std::string &src, const std::string &key) {
  boost::uint8_t hash_val[sha1::HASH_SIZE];
  hmac<sha1>::calc(src, key, hash_val);
  return base64::encode_from_array(hash_val, sha1::HASH_SIZE);
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

boost::any parse_json(boost::property_tree::ptree pt) {
  if (pt.empty()) {
    if (pt.data() == "true" || pt.data() == "false") {
      return boost::any(pt.get_value<bool>());
    } else if (regex_search(pt.data(), regex("^-?\\d+$"))) {
      long ln = atol(pt.data().c_str());
      if (ln > 2147483647 || ln < -2147483648) {
        return boost::any(ln);
      } else {
        return boost::any(atoi(pt.data().c_str()));
      }
    } else if (regex_search(pt.data(), regex(R"(^-?\d+\.{1}\d+$)"))) {
      return boost::any(atof(pt.data().c_str()));
    }
    return boost::any(pt.data());
  }
  vector<boost::any> vec;
  map<string, boost::any> m;
  for (const auto &it : pt) {
    if (!it.first.empty()) {
      m[it.first] = parse_json(it.second);
    } else {
      vec.push_back(parse_json(it.second));
    }
  }
  return vec.empty() ? boost::any(m) : boost::any(vec);
}

boost::any json_decode(const string &str) {
  std::stringstream ss(str);
  using namespace boost::property_tree;
  ptree pt;
  read_json(ss, pt);
  return parse_json(pt);
}

/*** <<<<<<<<<  Credential  >>>>>>>> ***/
Credential::Credential() = default;

Credential::Credential(const Config &config) {
  _config = config;
  _credentialType = *config.type;
}

Credential::~Credential() = default;

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

web::http::http_response Credential::request(string url) {
  http_client client(uri(conversions::to_string_t(url)));
  return client.request(methods::GET).get();
}

/*** <<<<<<<<<  AccessKeyCredential  >>>>>>>> ***/
AccessKeyCredential::AccessKeyCredential(const Config &config)
    : Credential(config){};

string AccessKeyCredential::getAccessKeyId() { return *_config.accessKeyId; }

string AccessKeyCredential::getAccessKeySecret() {
  return *_config.accessKeySecret;
}

/*** <<<<<<<<<  BearerTokenCredential  >>>>>>>> ***/
BearerTokenCredential::BearerTokenCredential(const Config &config)
    : Credential(config){};

string BearerTokenCredential::getBearerToken() { return *_config.bearerToken; }

/*** <<<<<<<<<  StsCredential  >>>>>>>> ***/
StsCredential::StsCredential(const Config &config) : Credential(config) {}

string StsCredential::getAccessKeyId() { return *_config.accessKeyId; }

string StsCredential::getAccessKeySecret() { return *_config.accessKeySecret; }

string StsCredential::getSecurityToken() { return *_config.securityToken; }

/*** <<<<<<<<<  EcsRamRoleCredential  >>>>>>>> ***/
EcsRamRoleCredential::EcsRamRoleCredential(const Config &config)
    : Credential(config) {}

string EcsRamRoleCredential::getAccessKeyId() {
  refresh();
  return *_config.accessKeyId;
}

string EcsRamRoleCredential::getAccessKeySecret() {
  refresh();
  return *_config.accessKeySecret;
}

string EcsRamRoleCredential::getSecurityToken() {
  refresh();
  return *_config.securityToken;
}

void EcsRamRoleCredential::refresh() {
  if (has_expired()) {
    refreshRam();
  }
}

void EcsRamRoleCredential::refreshRam() {
  if (!_config.roleName || _config.roleName->empty()) {
    string url =
        (string) "https://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA;
    http_client client(uri(conversions::to_string_t(url)));
    http_response response = request(url);
    concurrency::streams::stringstreambuf buffer;
    if (response.status_code() == status_codes::OK) {
      response.body().read_to_end(buffer).get();
      _config.roleName = make_shared<string>(string(buffer.collection()));
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
  string role_name = !_config.roleName ? "" : *_config.roleName;
  string url =
      "https://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA + role_name;
  http_client client(uri(conversions::to_string_t(url)));
  http_response response = request(url);
  concurrency::streams::stringstreambuf buffer;
  if (response.status_code() == status_codes::OK) {
    response.body().read_to_end(buffer).get();
    string data = buffer.collection();
    map<string, boost::any> result = boost::any_cast<map<string, boost::any>>(json_decode(data));
    string code = boost::any_cast<string>(result["Code"]);
    if (code == "Success") {
      _config.accessKeyId =
          make_shared<string>(boost::any_cast<string>(result["AccessKeyId"]));
      _config.accessKeySecret =
          make_shared<string>(boost::any_cast<string>(result["AccessKeySecret"]));
      _config.securityToken =
          make_shared<string>(boost::any_cast<string>(result["SecurityToken"]));
      _expiration = strtotime(boost::any_cast<string>(result["Expiration"]));
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
RamRoleArnCredential::RamRoleArnCredential(const Config &config)
    : Credential(config) {
  if (!_config.roleSessionName && _config.roleSessionName->empty()) {
    _config.roleSessionName = make_shared<string>("defaultSessionName");
  }
};

string RamRoleArnCredential::getAccessKeyId() {
  refresh();
  return *_config.accessKeyId;
}

string RamRoleArnCredential::getAccessKeySecret() {
  refresh();
  return *_config.accessKeySecret;
}

string RamRoleArnCredential::getSecurityToken() {
  refresh();
  return *_config.securityToken;
}

string RamRoleArnCredential::getRoleArn() {
  refresh();
  return *_config.roleArn;
}

string RamRoleArnCredential::getPolicy() {
  refresh();
  return *_config.policy;
}

void RamRoleArnCredential::refresh() {
  if (has_expired()) {
    refreshCredential();
  }
}

void RamRoleArnCredential::refreshCredential() {
  map<string, string> params;
  int duration_seconds =
      !_config.durationSeconds ? 0 : *_config.durationSeconds;
  string roleArn = !_config.roleArn ? "" : *_config.roleArn;
  string accessKeyId = !_config.accessKeyId ? "" : *_config.accessKeyId;
  string roleSessionName =
      !_config.roleSessionName ? "" : *_config.roleSessionName;
  string policy = !_config.policy ? "" : *_config.policy;

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
  string json = requestSTS(*_config.accessKeySecret, methods::GET, params);
  map<string, boost::any> result = boost::any_cast<map<string, boost::any>>(json_decode(json));
  string code = boost::any_cast<string>(result["Code"]);
  if (code == "Success") {
    map<string, boost::any> credential = boost::any_cast<map<string, boost::any>>(result["Credentials"]);
    _config.accessKeyId =
        make_shared<string>(boost::any_cast<string>(credential["AccessKeyId"]));
    _config.accessKeySecret = make_shared<string>(
        boost::any_cast<string>(credential["AccessKeySecret"]));
    _expiration = strtotime(boost::any_cast<string>(credential["Expiration"]));
    _config.securityToken =
        make_shared<string>(boost::any_cast<string>(credential["SecurityToken"]));
  } else {
    RefreshCredentialException rce(
        "Failed to get Security Token from STS service.");
    throw rce;
  }
}

/*** <<<<<<<<<  RsaKeyPairCredential  >>>>>>>> ***/
RsaKeyPairCredential::RsaKeyPairCredential(const Config &config)
    : Credential(config) {}

string RsaKeyPairCredential::getPublicKeyId() {
  refresh();
  return *_config.publicKeyId;
}

string RsaKeyPairCredential::getPrivateKeySecret() {
  refresh();
  return *_config.privateKeyFile;
}

string RsaKeyPairCredential::getAccessKeyId() {
  refresh();
  return *_config.accessKeyId;
}

string RsaKeyPairCredential::getAccessKeySecret() {
  refresh();
  return *_config.accessKeySecret;
}

string RsaKeyPairCredential::getSecurityToken() {
  refresh();
  return *_config.securityToken;
}

void RsaKeyPairCredential::refresh() {
  if (has_expired()) {
    refreshCredential();
  }
}

void RsaKeyPairCredential::refreshCredential() {
  map<string, string> params;
  int duration_seconds =
      !_config.durationSeconds ? 0 : *_config.durationSeconds;
  string access_key_id = !_config.publicKeyId ? "" : *_config.publicKeyId;
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
  string secret = !_config.accessKeySecret ? "" : *_config.accessKeySecret;
  string json = requestSTS(secret, methods::GET, params);

  map<string, boost::any> result = boost::any_cast<map<string, boost::any>>(json_decode(json));
  string code = boost::any_cast<string>(result["Code"]);
  if (code == "Success") {
    map<string, boost::any> SessionAccessKey = boost::any_cast<map<string, boost::any>>(result["SessionAccessKey"]);
    _config.accessKeyId =
        make_shared<string>(boost::any_cast<string>(SessionAccessKey["SessionAccessKeyId"]));
    _config.accessKeySecret = make_shared<string>(
        boost::any_cast<string>(SessionAccessKey["SessionAccessKeySecret"]));
    _expiration = strtotime(boost::any_cast<string>(SessionAccessKey["Expiration"]));
  } else {
    RefreshCredentialException rce(
        "Failed to get Security Token from STS service.");
    throw rce;
  }
}