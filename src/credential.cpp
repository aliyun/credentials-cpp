#include <alibabacloud/credential.hpp>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <curl/curl.h>
#include <iostream>
#include <map>

using namespace std;
using namespace Alibabacloud_Credential;
using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

credential::credential() {}

credential::credential(const Config &config) {
  _config = config;
  _credentialType = config.type;
}

bool credential::hasExpired() const {
  long now = static_cast<long int>(time(nullptr));
  return _expiration - now <= 180;
}

/*** <<<<<<<<<  AccessKeyCredential  >>>>>>>> ***/
AccessKeyCredential::AccessKeyCredential(const Config &config)
    : credential(config){};

string AccessKeyCredential::getAccessKeyId() { return _config.accessKeyId; }

string AccessKeyCredential::getAccessKeySecret() {
  return _config.accessKeySecret;
}

/*** <<<<<<<<<  BearerTokenCredential  >>>>>>>> ***/
BearerTokenCredential::BearerTokenCredential(const Config &config)
    : credential(config){};

string BearerTokenCredential::getBearerToken() { return _config.bearerToken; }

/*** <<<<<<<<<  StsCredential  >>>>>>>> ***/
StsCredential::StsCredential(const Config &config) : credential(config) {}

string StsCredential::getAccessKeyId() { return _config.accessKeyId; }

string StsCredential::getAccessKeySecret() { return _config.accessKeySecret; }

string StsCredential::getSecurityToken() { return _config.securityToken; }

/*** <<<<<<<<<  EcsRamRoleCredential  >>>>>>>> ***/
EcsRamRoleCredential::EcsRamRoleCredential(const Config &config)
    : credential(config) {}

string EcsRamRoleCredential::getAccessKeyId() {
  refresh();
  return _config.accessKeyId;
}

string EcsRamRoleCredential::getAccessKeySecret() {
  refresh();
  return _config.accessKeySecret;
}

string EcsRamRoleCredential::getSecurityToken() {
  refresh();
  return _config.securityToken;
}

void EcsRamRoleCredential::refresh() {
  if (hasExpired()) {
    refreshRam();
  }
}

void EcsRamRoleCredential::refreshRam() {
  if (_config.roleName.empty()) {
    string url =
        (string) "https://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA;
    http_client client(uri(conversions::to_string_t(url)));
    http_response response;
    if (hasMockHttpResponse()) {
      response = popMockHttpResponse();
    } else {
      response = client.request(methods::GET).get();
    }
    concurrency::streams::stringstreambuf buffer;
    if (response.status_code() == status_codes::OK) {
      response.body().read_to_end(buffer).get();
      _config.roleName = buffer.collection();
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
  string url = "https://" + META_DATA_SERVICE_HOST + URL_IN_ECS_META_DATA +
               _config.roleName;
  http_client client(uri(conversions::to_string_t(url)));
  http_response response;
  if (hasMockHttpResponse()) {
    response = popMockHttpResponse();
  } else {
    response = client.request(methods::GET).get();
  }
  concurrency::streams::stringstreambuf buffer;
  if (response.status_code() == status_codes::OK) {
    response.body().read_to_end(buffer).get();
    string data = buffer.collection();
    Json::Value result = json_decode(data);
    string code = result["Code"].asString();
    if (code == "Success") {
      _config.accessKeyId = result["AccessKeyId"].asString();
      _config.accessKeySecret = result["AccessKeySecret"].asString();
      _config.securityToken = result["SecurityToken"].asString();
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
RamRoleArnCredential::RamRoleArnCredential(const Config &config)
    : credential(config) {
  if (_config.roleSessionName.empty()) {
    _config.roleSessionName = "defaultSessionName";
  }
};

string RamRoleArnCredential::getAccessKeyId() {
  refresh();
  return _config.accessKeyId;
}

string RamRoleArnCredential::getAccessKeySecret() {
  refresh();
  return _config.accessKeySecret;
}

string RamRoleArnCredential::getSecurityToken() {
  refresh();
  return _config.securityToken;
}

string RamRoleArnCredential::getRoleArn() {
  refresh();
  return _config.roleArn;
}

string RamRoleArnCredential::getPolicy() {
  refresh();
  return _config.policy;
}

void RamRoleArnCredential::refresh() {
  if (hasExpired()) {
    refreshCredential();
  }
}

void RamRoleArnCredential::refreshCredential() {
  map<string, string> params;
  params.insert(pair<string, string>("Action", "AssumeRole"));
  params.insert(pair<string, string>("Format", "JSON"));
  params.insert(pair<string, string>("Version", "2015-04-01"));
  params.insert(pair<string, string>("DurationSeconds",
                                     to_string(_config.durationSeconds)));
  params.insert(pair<string, string>("RoleArn", _config.roleArn));
  params.insert(pair<string, string>("AccessKeyId", _config.accessKeyId));
  params.insert(pair<string, string>("RegionId", _regionId));
  params.insert(
      pair<string, string>("RoleSessionName", _config.roleSessionName));
  params.insert(pair<string, string>("SignatureVersion", "1.0"));
  params.insert(pair<string, string>("SignatureMethod", "HMAC-SHA1"));
  params.insert(pair<string, string>("Timestamp", gmt_datetime()));
  params.insert(pair<string, string>("SignatureNonce", uuid()));
  if (!_config.policy.empty()) {
    params.insert(pair<string, string>("Policy", _config.policy));
  }
  string json = stsRequest(_config.accessKeySecret, methods::GET, params);
  Json::Value result = json_decode(json);
  string code = result["Code"].asString();
  if (code == "Success") {
    _config.accessKeyId = result["Credentials"]["AccessKeyId"].asString();
    _config.accessKeySecret =
        result["Credentials"]["AccessKeySecret"].asString();
    _expiration = strtotime(result["Credentials"]["Expiration"].asString());
    _config.securityToken = result["Credentials"]["SecurityToken"].asString();
  } else {
    RefreshCredentialException rce(
        "Failed to get Security Token from STS service.");
    throw rce;
  }
}

/*** <<<<<<<<<  RsaKeyPairCredential  >>>>>>>> ***/
RsaKeyPairCredential::RsaKeyPairCredential(const Config &config)
    : credential(config) {}

string RsaKeyPairCredential::getPublicKeyId() {
  refresh();
  return _config.publicKeyId;
}

string RsaKeyPairCredential::getPrivateKeySecret() {
  refresh();
  return _config.privateKeyFile;
}

string RsaKeyPairCredential::getAccessKeyId() {
  refresh();
  return _config.accessKeyId;
}

string RsaKeyPairCredential::getAccessKeySecret() {
  refresh();
  return _config.accessKeySecret;
}

string RsaKeyPairCredential::getSecurityToken() {
  refresh();
  return _config.securityToken;
}

void RsaKeyPairCredential::refresh() {
  if (hasExpired()) {
    refreshCredential();
  }
}

void RsaKeyPairCredential::refreshCredential() {
  map<string, string> params;
  params.insert(pair<string, string>("Action", "GenerateSessionAccessKey"));
  params.insert(pair<string, string>("Format", "JSON"));
  params.insert(pair<string, string>("Version", "2015-04-01"));
  params.insert(pair<string, string>("DurationSeconds",
                                     to_string(_config.durationSeconds)));
  params.insert(pair<string, string>("AccessKeyId", _config.accessKeyId));
  params.insert(pair<string, string>("RegionId", "cn-hangzhou"));
  params.insert(pair<string, string>("SignatureVersion", "1.0"));
  params.insert(pair<string, string>("SignatureMethod", "HMAC-SHA1"));
  params.insert(pair<string, string>("Timestamp", gmt_datetime()));
  params.insert(pair<string, string>("SignatureNonce", uuid()));

  string json = stsRequest(_config.privateKeySecret, methods::GET, params);
  Json::Value result = json_decode(json);
  string code = result["Code"].asString();
  if (code == "Success") {
    _config.accessKeyId = result["Credentials"]["AccessKeyId"].asString();
    _config.accessKeySecret =
        result["Credentials"]["AccessKeySecret"].asString();
    _expiration = strtotime(result["Credentials"]["Expiration"].asString());
    _config.securityToken = result["Credentials"]["SecurityToken"].asString();
  } else {
    RefreshCredentialException rce(
        "Failed to get Security Token from STS service.");
    throw rce;
  }
}