#ifndef ALIBABACLOUD_CREDENTIAL_H_
#define ALIBABACLOUD_CREDENTIAL_H_

#include <alibabacloud/credential_utils.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cpprest/http_client.h>
#include <exception>
#include <iostream>
#include <map>
#include <json/json.h>

using namespace std;

namespace Alibabacloud_Credential {
class RefreshCredentialException : public exception {
public:
  explicit RefreshCredentialException(const string &msg) { _msg = msg; }

  const char *what() const noexcept override {
    string error_info;
    if (_msg.empty()) {
      error_info = "Refresh Credential Error ";
    } else {
      error_info = "Refresh Credential Error : " + _msg;
    }
    return error_info.c_str();
  }

  void setHttpResponse(const web::http::http_response &res) { _res = res; }

  web::http::http_response getHttpResponse() { return _res; }

private:
  string _msg;
  web::http::http_response _res;
};
class Config {
public:
  explicit Config(map<string, string> config);

  Config();

  ~Config();

  string accessKeyId;
  string accessKeySecret;
  string securityToken;
  string bearerToken;
  int durationSeconds;
  string roleArn;
  string policy;
  int roleSessionExpiration;
  string roleSessionName;
  string publicKeyId;
  string privateKeyFile;
  string privateKeySecret;
  string roleName;
  string type;
};

class Credential {
public:
  explicit Credential(const Alibabacloud_Credential::Config &config);
  Credential();

  long getExpiration() const { return _expiration; }

  virtual string getAccessKeyId() { return _config.accessKeyId; }

  virtual string getAccessKeySecret() { return _config.accessKeySecret; }

  virtual string getSecurityToken() { return _config.securityToken; }

  virtual string getBearerToken() { return _config.bearerToken; }

  string getType() const { return _config.type; }

  Alibabacloud_Credential::Config getConfig() const { return _config; }

protected:
  Alibabacloud_Credential::Config _config;
  string _credentialType = "access_key";
  long _expiration = 0;

  bool hasExpired() const;
};

class AccessKeyCredential : public Credential {
public:
  explicit AccessKeyCredential(const Config &config);

  string getAccessKeyId() override;

  string getAccessKeySecret() override;
};

class BearerTokenCredential : public Credential {
public:
  explicit BearerTokenCredential(const Alibabacloud_Credential::Config &config);

  string getBearerToken() override;
};

class StsCredential : public Credential {
public:
  explicit StsCredential(const Alibabacloud_Credential::Config &config);

  string getAccessKeyId() override;

  string getAccessKeySecret() override;

  string getSecurityToken() override;
};

class EcsRamRoleCredential : public Credential {
public:
  explicit EcsRamRoleCredential(const Alibabacloud_Credential::Config &config);

  string getAccessKeyId() override;

  string getAccessKeySecret() override;

  string getSecurityToken() override;

private:
  void refresh();

  void refreshRam();

  void refreshCredential();

  const string URL_IN_ECS_META_DATA =
      "/latest/meta-data/ram/security-credentials/";
  const string ECS_META_DATA_FETCH_ERROR_MSG =
      "Failed to get RAM session credentials from ECS metadata service.";
  const string META_DATA_SERVICE_HOST = "100.100.100.200";
};

class RamRoleArnCredential : public Credential {
public:
  explicit RamRoleArnCredential(const Alibabacloud_Credential::Config &config);

  string getAccessKeyId() override;

  string getAccessKeySecret() override;

  string getSecurityToken() override;

  string getRoleArn();

  string getPolicy();

private:
  void refresh();

  void refreshCredential();

  string _regionId = "cn-hangzhou";
};

class RsaKeyPairCredential : public Credential {
public:
  explicit RsaKeyPairCredential(const Alibabacloud_Credential::Config &config);
  string getPublicKeyId();
  string getPrivateKeySecret();
  string getAccessKeyId() override;
  string getAccessKeySecret() override;
  string getSecurityToken() override;

private:
  void refresh();
  void refreshCredential();
};

class Client {
public:
  Client();

  ~Client();

  explicit Client(Config *config);

  string getAccessKeyId();

  string getAccessKeySecret();

  string getSecurityToken();

  string getBearerToken();

  string getType();

  string getRoleArn();

  string getRoleSessionName();

  string getPolicy();

  string getRoleName();

  string getPublicKeyId();

  string getPrivateKey();

  Credential getCredential();

private:
  Credential *_credential;
};
} // namespace Alibabacloud_Credential

#endif
