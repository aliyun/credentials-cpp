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

Credential::Credential() {}

Credential::Credential(Config *config) {
  _config = config;
  _credentialType = config->type;
}

Credential::~Credential() { delete _config; }

bool Credential::hasExpired() const {
  long now = static_cast<long int>(time(nullptr));
  return _expiration - now <= 180;
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
  if (hasExpired()) {
    refreshRam();
  }
}

void EcsRamRoleCredential::refreshRam() {
  if (nullptr == _config->roleName || _config->roleName->empty()) {
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
  if (hasExpired()) {
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
  string json = stsRequest(*_config->accessKeySecret, methods::GET, params);
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
  if (hasExpired()) {
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
  string json = stsRequest(secret, methods::GET, params);
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