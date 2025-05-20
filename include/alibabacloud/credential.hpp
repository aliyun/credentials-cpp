#ifndef ALIBABACLOUD_CREDENTIAL_H_
#define ALIBABACLOUD_CREDENTIAL_H_

#include <darabonba/core.hpp>
#include <boost/any.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cpprest/http_client.h>
#include <exception>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>

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
    const char *c = error_info.c_str();
    return c;
  }

  void setHttpResponse(const web::http::http_response &res) { _res = res; }

  web::http::http_response getHttpResponse() { return _res; }

private:
  string _msg;
  web::http::http_response _res;
};

class Config : public Darabonba::Model {
public:
  Config() {}
  // explicit Config(const Darabonba::Json &config) { fromMap(config); }

    // 实现基类的 validate 函数，保持 const 修饰符
  void validate() const override {
    // 这里可以添加具体的验证逻辑
  }

  Darabonba::Json toMap() const override{
     Darabonba::Json j;
    if (accessKeyId) j["accessKeyId"] = *accessKeyId;
    if (accessKeySecret) j["accessKeySecret"] = *accessKeySecret;
    if (securityToken) j["securityToken"] = *securityToken;
    if (bearerToken) j["bearerToken"] = *bearerToken;
    if (durationSeconds) j["durationSeconds"] = *durationSeconds;
    if (roleArn) j["roleArn"] = *roleArn;
    if (policy) j["policy"] = *policy;
    if (roleSessionExpiration) j["roleSessionExpiration"] = *roleSessionExpiration;
    if (roleSessionName) j["roleSessionName"] = *roleSessionName;
    if (publicKeyId) j["publicKeyId"] = *publicKeyId;
    if (privateKeyFile) j["privateKeyFile"] = *privateKeyFile;
    if (roleName) j["roleName"] = *roleName;
    if (type) j["type"] = *type;
    return j;
  }

  void fromMap(const Darabonba::Json &j) override {
    if (j.contains("accessKeyId") && !j["accessKeyId"].is_null()) {
      accessKeyId = std::make_shared<std::string>(j["accessKeyId"].get<std::string>());
    }
    if (j.contains("accessKeySecret") && !j["accessKeySecret"].is_null()) {
      accessKeySecret = std::make_shared<std::string>(j["accessKeySecret"].get<std::string>());
    }
    if (j.contains("securityToken") && !j["securityToken"].is_null()) {
      securityToken = std::make_shared<std::string>(j["securityToken"].get<std::string>());
    }
  }
  virtual bool empty() const override {
    
  }

  shared_ptr<string> accessKeyId{};
  shared_ptr<string> accessKeySecret{};
  shared_ptr<string> securityToken{};
  shared_ptr<string> bearerToken{};
  shared_ptr<int> durationSeconds{};
  shared_ptr<string> roleArn{};
  shared_ptr<string> policy{};
  shared_ptr<int> roleSessionExpiration{};
  shared_ptr<string> roleSessionName{};
  shared_ptr<string> publicKeyId{};
  shared_ptr<string> privateKeyFile{};
  shared_ptr<string> roleName{};
  shared_ptr<string> type{};

  ~Config() = default;
};

class Credential {
public:
  explicit Credential(const Config &config);
  Credential();
  ~Credential();

  long getExpiration() const { return _expiration; }

  virtual string getAccessKeyId() {
    return _config.accessKeyId ? *_config.accessKeyId : "";
  }

  virtual string getAccessKeySecret() {
    return _config.accessKeySecret ? *_config.accessKeySecret : "";
  }

  virtual string getSecurityToken() {
    return _config.securityToken ? *_config.securityToken : "";
  }

  virtual string getBearerToken() {
    return _config.bearerToken ? *_config.bearerToken : "";
  }

  string getType() const { return _config.type ? *_config.type : ""; }

  Config getConfig() { return _config; }

  string requestSTS(string accessKeySecret, web::http::method mtd,
                    map<string, string> query);

  virtual web::http::http_response request(string url);

protected:
  Config _config;
  string _credentialType;
  long _expiration = 0;

  bool has_expired() const;
};

class AccessKeyCredential : public Credential {
public:
  explicit AccessKeyCredential(const Config &config);

  string getAccessKeyId() override;

  string getAccessKeySecret() override;
};

class BearerTokenCredential : public Credential {
public:
  explicit BearerTokenCredential(const Config &config);

  string getBearerToken() override;
};

class StsCredential : public Credential {
public:
  explicit StsCredential(const Config &config);

  string getAccessKeyId() override;

  string getAccessKeySecret() override;

  string getSecurityToken() override;
};

class EcsRamRoleCredential : public Credential {
public:
  explicit EcsRamRoleCredential(const Config &config);

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
  explicit RamRoleArnCredential(const Config &config);

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
  explicit RsaKeyPairCredential(const Config &config);
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

  explicit Client(shared_ptr<Config> config);

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
  Credential *_credential{};
};
} // namespace Alibabacloud_Credential

#endif
