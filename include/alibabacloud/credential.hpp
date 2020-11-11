#ifndef ALIBABACLOUD_CREDENTIAL_H_
#define ALIBABACLOUD_CREDENTIAL_H_

#include <darabonba/core.hpp>
#include <boost/any.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cpprest/http_client.h>
#include <exception>
#include <iostream>
#include <json/json.h>
#include <map>

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
  explicit Config(const std::map<string, boost::any> &config) : Darabonba::Model(config) {
    fromMap(config);
  };

  void validate() override {}

  map<string, boost::any> toMap() override {
    map<string, boost::any> res;
    if (accessKeyId) {
      res["accessKeyId"] = boost::any(*accessKeyId);
    }
    if (accessKeySecret) {
      res["accessKeySecret"] = boost::any(*accessKeySecret);
    }
    if (securityToken) {
      res["securityToken"] = boost::any(*securityToken);
    }
    if (bearerToken) {
      res["bearerToken"] = boost::any(*bearerToken);
    }
    if (durationSeconds) {
      res["durationSeconds"] = boost::any(*durationSeconds);
    }
    if (roleArn) {
      res["roleArn"] = boost::any(*roleArn);
    }
    if (policy) {
      res["policy"] = boost::any(*policy);
    }
    if (roleSessionExpiration) {
      res["roleSessionExpiration"] = boost::any(*roleSessionExpiration);
    }
    if (roleSessionName) {
      res["roleSessionName"] = boost::any(*roleSessionName);
    }
    if (publicKeyId) {
      res["publicKeyId"] = boost::any(*publicKeyId);
    }
    if (privateKeyFile) {
      res["privateKeyFile"] = boost::any(*privateKeyFile);
    }
    if (roleName) {
      res["roleName"] = boost::any(*roleName);
    }
    if (type) {
      res["type"] = boost::any(*type);
    }
    return res;
  }

  void fromMap(map<string, boost::any> m) override {
    if (m.find("accessKeyId") != m.end()) {
      accessKeyId = make_shared<string>(boost::any_cast<string>(m["accessKeyId"]));
    }
    if (m.find("accessKeySecret") != m.end()) {
      accessKeySecret = make_shared<string>(boost::any_cast<string>(m["accessKeySecret"]));
    }
    if (m.find("securityToken") != m.end()) {
      securityToken = make_shared<string>(boost::any_cast<string>(m["securityToken"]));
    }
    if (m.find("bearerToken") != m.end()) {
      bearerToken = make_shared<string>(boost::any_cast<string>(m["bearerToken"]));
    }
    if (m.find("durationSeconds") != m.end()) {
      durationSeconds = make_shared<int>(boost::any_cast<int>(m["durationSeconds"]));
    }
    if (m.find("roleArn") != m.end()) {
      roleArn = make_shared<string>(boost::any_cast<string>(m["roleArn"]));
    }
    if (m.find("policy") != m.end()) {
      policy = make_shared<string>(boost::any_cast<string>(m["policy"]));
    }
    if (m.find("roleSessionExpiration") != m.end()) {
      roleSessionExpiration = make_shared<int>(boost::any_cast<int>(m["roleSessionExpiration"]));
    }
    if (m.find("roleSessionName") != m.end()) {
      roleSessionName = make_shared<string>(boost::any_cast<string>(m["roleSessionName"]));
    }
    if (m.find("publicKeyId") != m.end()) {
      publicKeyId = make_shared<string>(boost::any_cast<string>(m["publicKeyId"]));
    }
    if (m.find("privateKeyFile") != m.end()) {
      privateKeyFile = make_shared<string>(boost::any_cast<string>(m["privateKeyFile"]));
    }
    if (m.find("roleName") != m.end()) {
      roleName = make_shared<string>(boost::any_cast<string>(m["roleName"]));
    }
    if (m.find("type") != m.end()) {
      type = make_shared<string>(boost::any_cast<string>(m["type"]));
    }
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

  virtual string getAccessKeyId() { return *_config.accessKeyId; }

  virtual string getAccessKeySecret() { return *_config.accessKeySecret; }

  virtual string getSecurityToken() { return *_config.securityToken; }

  virtual string getBearerToken() { return *_config.bearerToken; }

  string getType() const { return *_config.type; }

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
