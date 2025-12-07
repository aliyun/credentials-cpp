#ifndef ALIBABACLOUD_CREDENTIAL_MODEL_HPP_
#define ALIBABACLOUD_CREDENTIAL_MODEL_HPP_

#include <memory>

#include <darabonba/Model.hpp>

// Forward declaration to avoid circular dependency
namespace AlibabaCloud {
namespace Credential {
class AuthUtil;
}
}

namespace AlibabaCloud {
namespace Credential {
namespace Models {

class CredentialModel : public Darabonba::Model {
  friend void to_json(Darabonba::Json &j, const CredentialModel &obj) {
    DARABONBA_PTR_TO_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_TO_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_TO_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_TO_JSON(securityToken, securityToken_);
    DARABONBA_PTR_TO_JSON(type, type_);
    DARABONBA_PTR_TO_JSON(providerName, providerName_);
  }

  friend void from_json(const Darabonba::Json &j, CredentialModel &obj) {
    DARABONBA_PTR_FROM_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_FROM_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_FROM_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_FROM_JSON(securityToken, securityToken_);
    DARABONBA_PTR_FROM_JSON(providerName, providerName_);
    DARABONBA_PTR_FROM_JSON(type, type_);
  }

public:
  CredentialModel() = default;
  CredentialModel(const CredentialModel &) = default;
  CredentialModel(CredentialModel &&) = default;
  CredentialModel(const Darabonba::Json &obj) { from_json(obj, *this); }

  virtual ~CredentialModel() = default;

  virtual void validate() const override {}

  virtual void fromMap(const Darabonba::Json &obj) override {
    from_json(obj, *this);
    validate();
  }

  virtual Darabonba::Json toMap() const override {
    Darabonba::Json obj;
    to_json(obj, *this);
    return obj;
  }

  virtual bool empty() const override {
    return accessKeyId_ == nullptr && accessKeySecret_ == nullptr &&
           bearerToken_ == nullptr && securityToken_ == nullptr &&
           type_ == nullptr && providerName_ == nullptr;
  }

  bool hasAccessKeyId() const { return this->accessKeyId_ != nullptr; }
  std::string accessKeyId() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeyId_, "");
  }
  CredentialModel &setAccessKeyId(const std::string &accessKeyId) {
    DARABONBA_PTR_SET_VALUE(accessKeyId_, accessKeyId);
    return *this;
  }
  CredentialModel &setAccessKeyId(std::string &&accessKeyId) {
    DARABONBA_PTR_SET_RVALUE(accessKeyId_, accessKeyId);
    return *this;
  }

  bool hasAccessKeySecret() const { return this->accessKeySecret_ != nullptr; }
  std::string accessKeySecret() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeySecret_, "");
  }
  CredentialModel &setAccessKeySecret(const std::string &accessKeySecret) {
    DARABONBA_PTR_SET_VALUE(accessKeySecret_, accessKeySecret);
    return *this;
  }
  CredentialModel &setAccessKeySecret(std::string &&accessKeySecret) {
    DARABONBA_PTR_SET_RVALUE(accessKeySecret_, accessKeySecret);
    return *this;
  }

  bool hasBearerToken() const { return this->bearerToken_ != nullptr; }
  std::string bearerToken() const {
    DARABONBA_PTR_GET_DEFAULT(bearerToken_, "");
  }
  CredentialModel &setBearerToken(const std::string &bearerToken) {
    DARABONBA_PTR_SET_VALUE(bearerToken_, bearerToken);
    return *this;
  }
  CredentialModel &setBearerToken(std::string &&bearerToken) {
    DARABONBA_PTR_SET_RVALUE(bearerToken_, bearerToken);
    return *this;
  }

  bool hasSecurityToken() const { return this->securityToken_ != nullptr; }
  std::string securityToken() const {
    DARABONBA_PTR_GET_DEFAULT(securityToken_, "");
  }
  CredentialModel &setSecurityToken(const std::string &securityToken) {
    DARABONBA_PTR_SET_VALUE(securityToken_, securityToken);
    return *this;
  }
  CredentialModel &setSecurityToken(std::string &&securityToken) {
    DARABONBA_PTR_SET_RVALUE(securityToken_, securityToken);
    return *this;
  }

  bool hasType() const { return this->type_ != nullptr; }
  std::string type() const { DARABONBA_PTR_GET_DEFAULT(type_, ""); }
  CredentialModel &setType(const std::string &type) {
    DARABONBA_PTR_SET_VALUE(type_, type);
    return *this;
  }
  CredentialModel &setType(std::string &&type) {
    DARABONBA_PTR_SET_RVALUE(type_, type);
    return *this;
  }

  bool hasProviderName() const { return this->providerName_ != nullptr; }
  std::string providerName() const { DARABONBA_PTR_GET_DEFAULT(providerName_, ""); }
  CredentialModel &setProviderName(const std::string &providerName) {
    DARABONBA_PTR_SET_VALUE(providerName_, providerName);
    return *this;
  }
  CredentialModel &setProviderName(std::string &&providerName) {
    DARABONBA_PTR_SET_RVALUE(providerName_, providerName);
    return *this;
  }

protected:
  std::shared_ptr<std::string> accessKeyId_ = nullptr;
  std::shared_ptr<std::string> accessKeySecret_ = nullptr;
  std::shared_ptr<std::string> bearerToken_ = nullptr;
  std::shared_ptr<std::string> securityToken_ = nullptr;
  std::shared_ptr<std::string> providerName_ = nullptr;
  std::shared_ptr<std::string> type_ = nullptr;
};

class Config : public Darabonba::Model {
  friend void to_json(Darabonba::Json &j, const Config &obj) {
    DARABONBA_PTR_TO_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_TO_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_TO_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_TO_JSON(credentialsURL, credentialsURL_);
    DARABONBA_PTR_TO_JSON(durationSeconds, durationSeconds_);
    DARABONBA_PTR_TO_JSON(externalId, externalId_);
    DARABONBA_PTR_TO_JSON(host, host_);
    DARABONBA_PTR_TO_JSON(oidcProviderArn, oidcProviderArn_);
    DARABONBA_PTR_TO_JSON(oidcTokenFilePath, oidcTokenFilePath_);
    DARABONBA_PTR_TO_JSON(policy, policy_);
    DARABONBA_PTR_TO_JSON(privateKeyFile, privateKeyFile_);
    DARABONBA_PTR_TO_JSON(proxy, proxy_);
    DARABONBA_PTR_TO_JSON(publicKeyId, publicKeyId_);
    DARABONBA_PTR_TO_JSON(regionId, regionId_);
    DARABONBA_PTR_TO_JSON(roleArn, roleArn_);
    DARABONBA_PTR_TO_JSON(roleName, roleName_);
    DARABONBA_PTR_TO_JSON(roleSessionExpiration, roleSessionExpiration_);
    DARABONBA_PTR_TO_JSON(roleSessionName, roleSessionName_);
    DARABONBA_PTR_TO_JSON(securityToken, securityToken_);
    DARABONBA_PTR_TO_JSON(stsEndpoint, stsEndpoint_);
    DARABONBA_PTR_TO_JSON(stsRegionId, stsRegionId_);
    DARABONBA_PTR_TO_JSON(enableVpc, enableVpc_);
    DARABONBA_PTR_TO_JSON(type, type_);
    DARABONBA_PTR_TO_JSON(timeout, timeout_);
    DARABONBA_PTR_TO_JSON(connectTimeout, connectTimeout_);
    DARABONBA_PTR_TO_JSON(disableIMDSv1, disableIMDSv1_);
    DARABONBA_PTR_TO_JSON(reuseLastProviderEnabled, reuseLastProviderEnabled_);
  }

  friend void from_json(const Darabonba::Json &j, Config &obj) {
    DARABONBA_PTR_FROM_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_FROM_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_FROM_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_FROM_JSON(credentialsURL, credentialsURL_);
    DARABONBA_PTR_FROM_JSON(durationSeconds, durationSeconds_);
    DARABONBA_PTR_FROM_JSON(externalId, externalId_);
    DARABONBA_PTR_FROM_JSON(host, host_);
    DARABONBA_PTR_FROM_JSON(oidcProviderArn, oidcProviderArn_);
    DARABONBA_PTR_FROM_JSON(oidcTokenFilePath, oidcTokenFilePath_);
    DARABONBA_PTR_FROM_JSON(policy, policy_);
    DARABONBA_PTR_FROM_JSON(privateKeyFile, privateKeyFile_);
    DARABONBA_PTR_FROM_JSON(proxy, proxy_);
    DARABONBA_PTR_FROM_JSON(publicKeyId, publicKeyId_);
    DARABONBA_PTR_FROM_JSON(regionId, regionId_);
    DARABONBA_PTR_FROM_JSON(roleArn, roleArn_);
    DARABONBA_PTR_FROM_JSON(roleName, roleName_);
    DARABONBA_PTR_FROM_JSON(roleSessionExpiration, roleSessionExpiration_);
    DARABONBA_PTR_FROM_JSON(roleSessionName, roleSessionName_);
    DARABONBA_PTR_FROM_JSON(securityToken, securityToken_);
    DARABONBA_PTR_FROM_JSON(stsEndpoint, stsEndpoint_);
    DARABONBA_PTR_FROM_JSON(stsRegionId, stsRegionId_);
    DARABONBA_PTR_FROM_JSON(enableVpc, enableVpc_);
    DARABONBA_PTR_FROM_JSON(type, type_);
    DARABONBA_PTR_FROM_JSON(timeout, timeout_);
    DARABONBA_PTR_FROM_JSON(connectTimeout, connectTimeout_);
    DARABONBA_PTR_FROM_JSON(disableIMDSv1, disableIMDSv1_);
    DARABONBA_PTR_FROM_JSON(reuseLastProviderEnabled, reuseLastProviderEnabled_);
  }

public:
  Config() = default;
  Config(const Config &) = default;
  Config(Config &&) = default;
  Config(const Darabonba::Json &obj) { from_json(obj, *this); }

  virtual ~Config() = default;

  Config &operator=(const Config &) = default;
  Config &operator=(Config &&) = default;

  virtual void validate() const override {}

  virtual void fromMap(const Darabonba::Json &obj) override {
    from_json(obj, *this);
    validate();
  }

  virtual Darabonba::Json toMap() const override {
    Darabonba::Json obj;
    to_json(obj, *this);
    return obj;
  }

  virtual bool empty() const override {
    return accessKeyId_ == nullptr && accessKeySecret_ == nullptr &&
           bearerToken_ == nullptr && credentialsURL_ == nullptr &&
           durationSeconds_ == nullptr && externalId_ == nullptr &&
           host_ == nullptr && oidcProviderArn_ == nullptr &&
           oidcTokenFilePath_ == nullptr && policy_ == nullptr &&
           privateKeyFile_ == nullptr && proxy_ == nullptr &&
           publicKeyId_ == nullptr && regionId_ == nullptr &&
           roleArn_ == nullptr && roleName_ == nullptr &&
           roleSessionExpiration_ == nullptr && roleSessionName_ == nullptr &&
           securityToken_ == nullptr && stsEndpoint_ == nullptr &&
           type_ == nullptr && timeout_ == nullptr &&
           connectTimeout_ == nullptr && disableIMDSv1_ == nullptr;
  }
  bool hasAccessKeyId() const { return this->accessKeyId_ != nullptr; }
  std::string accessKeyId() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeyId_, "");
  }
  Config &setAccessKeyId(const std::string &accessKeyId) {
    DARABONBA_PTR_SET_VALUE(accessKeyId_, accessKeyId);
    return *this;
  }
  Config &setAccessKeyId(std::string &&accessKeyId) {
    DARABONBA_PTR_SET_RVALUE(accessKeyId_, accessKeyId);
    return *this;
  }

  bool hasAccessKeySecret() const { return this->accessKeySecret_ != nullptr; }
  std::string accessKeySecret() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeySecret_, "");
  }
  Config &setAccessKeySecret(const std::string &accessKeySecret) {
    DARABONBA_PTR_SET_VALUE(accessKeySecret_, accessKeySecret);
    return *this;
  }
  Config &setAccessKeySecret(std::string &&accessKeySecret) {
    DARABONBA_PTR_SET_RVALUE(accessKeySecret_, accessKeySecret);
    return *this;
  }

  bool hasBearerToken() const { return this->bearerToken_ != nullptr; }
  std::string bearerToken() const {
    DARABONBA_PTR_GET_DEFAULT(bearerToken_, "");
  }
  Config &setBearerToken(const std::string &bearerToken) {
    DARABONBA_PTR_SET_VALUE(bearerToken_, bearerToken);
    return *this;
  }
  Config &setBearerToken(std::string &&bearerToken) {
    DARABONBA_PTR_SET_RVALUE(bearerToken_, bearerToken);
    return *this;
  }

  bool hasCredentialsURL() const { return this->credentialsURL_ != nullptr; }
  std::string credentialsURL() const {
    DARABONBA_PTR_GET_DEFAULT(credentialsURL_, "");
  }
  Config &setCredentialsURL(const std::string &credentialsURL) {
    DARABONBA_PTR_SET_VALUE(credentialsURL_, credentialsURL);
    return *this;
  }
  Config &setCredentialsURL(std::string &&credentialsURL) {
    DARABONBA_PTR_SET_RVALUE(credentialsURL_, credentialsURL);
    return *this;
  }

  bool hasDurationSeconds() const { return this->durationSeconds_ != nullptr; }
  int64_t durationSeconds() const {
    DARABONBA_PTR_GET_DEFAULT(durationSeconds_, 0);
  }
  Config &setDurationSeconds(int64_t durationSeconds) {
    DARABONBA_PTR_SET_VALUE(durationSeconds_, durationSeconds);
    return *this;
  }

  bool hasExternalId() const { return this->externalId_ != nullptr; }
  std::string externalId() const { DARABONBA_PTR_GET_DEFAULT(externalId_, ""); }
  Config &setExternalId(const std::string &externalId) {
    DARABONBA_PTR_SET_VALUE(externalId_, externalId);
    return *this;
  }
  Config &setExternalId(std::string &&externalId) {
    DARABONBA_PTR_SET_RVALUE(externalId_, externalId);
    return *this;
  }

  bool hasHost() const { return this->host_ != nullptr; }
  std::string host() const { DARABONBA_PTR_GET_DEFAULT(host_, ""); }
  Config &setHost(const std::string &host) {
    DARABONBA_PTR_SET_VALUE(host_, host);
    return *this;
  }
  Config &setHost(std::string &&host) {
    DARABONBA_PTR_SET_RVALUE(host_, host);
    return *this;
  }

  bool hasOidcProviderArn() const { return this->oidcProviderArn_ != nullptr; }
  std::string oidcProviderArn() const {
    DARABONBA_PTR_GET_DEFAULT(oidcProviderArn_, "");
  }
  Config &setOidcProviderArn(const std::string &oidcProviderArn) {
    DARABONBA_PTR_SET_VALUE(oidcProviderArn_, oidcProviderArn);
    return *this;
  }
  Config &setOidcProviderArn(std::string &&oidcProviderArn) {
    DARABONBA_PTR_SET_RVALUE(oidcProviderArn_, oidcProviderArn);
    return *this;
  }

  bool hasOidcTokenFilePath() const {
    return this->oidcTokenFilePath_ != nullptr;
  }
  std::string oidcTokenFilePath() const {
    DARABONBA_PTR_GET_DEFAULT(oidcTokenFilePath_, "");
  }
  Config &setOidcTokenFilePath(const std::string &oidcTokenFilePath) {
    DARABONBA_PTR_SET_VALUE(oidcTokenFilePath_, oidcTokenFilePath);
    return *this;
  }
  Config &setOidcTokenFilePath(std::string &&oidcTokenFilePath) {
    DARABONBA_PTR_SET_RVALUE(oidcTokenFilePath_, oidcTokenFilePath);
    return *this;
  }

  bool hasPolicy() const { return this->policy_ != nullptr; }
  std::string policy() const { DARABONBA_PTR_GET_DEFAULT(policy_, ""); }
  Config &setPolicy(const std::string &policy) {
    DARABONBA_PTR_SET_VALUE(policy_, policy);
    return *this;
  }
  Config &setPolicy(std::string &&policy) {
    DARABONBA_PTR_SET_RVALUE(policy_, policy);
    return *this;
  }

  bool hasPrivateKeyFile() const { return this->privateKeyFile_ != nullptr; }
  std::string privateKeyFile() const {
    DARABONBA_PTR_GET_DEFAULT(privateKeyFile_, "");
  }
  Config &setPrivateKeyFile(const std::string &privateKeyFile) {
    DARABONBA_PTR_SET_VALUE(privateKeyFile_, privateKeyFile);
    return *this;
  }
  Config &setPrivateKeyFile(std::string &&privateKeyFile) {
    DARABONBA_PTR_SET_RVALUE(privateKeyFile_, privateKeyFile);
    return *this;
  }

  bool hasProxy() const { return this->proxy_ != nullptr; }
  std::string proxy() const { DARABONBA_PTR_GET_DEFAULT(proxy_, ""); }
  Config &setProxy(const std::string &proxy) {
    DARABONBA_PTR_SET_VALUE(proxy_, proxy);
    return *this;
  }
  Config &setProxy(std::string &&proxy) {
    DARABONBA_PTR_SET_RVALUE(proxy_, proxy);
    return *this;
  }

  bool hasPublicKeyId() const { return this->publicKeyId_ != nullptr; }
  std::string publicKeyId() const {
    DARABONBA_PTR_GET_DEFAULT(publicKeyId_, "");
  }
  Config &setPublicKeyId(const std::string &publicKeyId) {
    DARABONBA_PTR_SET_VALUE(publicKeyId_, publicKeyId);
    return *this;
  }
  Config &setPublicKeyId(std::string &&publicKeyId) {
    DARABONBA_PTR_SET_RVALUE(publicKeyId_, publicKeyId);
    return *this;
  }

  bool hasRegionId() const { return this->regionId_ != nullptr; }
  std::string regionId() const { DARABONBA_PTR_GET_DEFAULT(regionId_, ""); }
  Config &setRegionId(const std::string &regionId) {
    DARABONBA_PTR_SET_VALUE(regionId_, regionId);
    return *this;
  }
  Config &setRegionId(std::string &&regionId) {
    DARABONBA_PTR_SET_RVALUE(regionId_, regionId);
    return *this;
  }

  bool hasRoleArn() const { return this->roleArn_ != nullptr; }
  std::string roleArn() const { DARABONBA_PTR_GET_DEFAULT(roleArn_, ""); }
  Config &setRoleArn(const std::string &roleArn) {
    DARABONBA_PTR_SET_VALUE(roleArn_, roleArn);
    return *this;
  }
  Config &setRoleArn(std::string &&roleArn) {
    DARABONBA_PTR_SET_RVALUE(roleArn_, roleArn);
    return *this;
  }

  bool hasRoleName() const { return this->roleName_ != nullptr; }
  std::string roleName() const { DARABONBA_PTR_GET_DEFAULT(roleName_, ""); }
  Config &setRoleName(const std::string &roleName) {
    DARABONBA_PTR_SET_VALUE(roleName_, roleName);
    return *this;
  }
  Config &setRoleName(std::string &&roleName) {
    DARABONBA_PTR_SET_RVALUE(roleName_, roleName);
    return *this;
  }

  bool hasRoleSessionExpiration() const {
    return this->roleSessionExpiration_ != nullptr;
  }
  int64_t roleSessionExpiration() const {
    DARABONBA_PTR_GET_DEFAULT(roleSessionExpiration_, 0);
  }
  Config &setRoleSessionExpiration(int64_t roleSessionExpiration) {
    DARABONBA_PTR_SET_VALUE(roleSessionExpiration_, roleSessionExpiration);
    return *this;
  }

  bool hasRoleSessionName() const { return this->roleSessionName_ != nullptr; }
  std::string roleSessionName() const;  // Implemented in Model.cpp for dynamic default
  Config &setRoleSessionName(const std::string &roleSessionName) {
    DARABONBA_PTR_SET_VALUE(roleSessionName_, roleSessionName);
    return *this;
  }
  Config &setRoleSessionName(std::string &&roleSessionName) {
    DARABONBA_PTR_SET_RVALUE(roleSessionName_, roleSessionName);
    return *this;
  }

  bool hasSecurityToken() const { return this->securityToken_ != nullptr; }
  std::string securityToken() const {
    DARABONBA_PTR_GET_DEFAULT(securityToken_, "");
  }
  Config &setSecurityToken(const std::string &securityToken) {
    DARABONBA_PTR_SET_VALUE(securityToken_, securityToken);
    return *this;
  }
  Config &setSecurityToken(std::string &&securityToken) {
    DARABONBA_PTR_SET_RVALUE(securityToken_, securityToken);
    return *this;
  }

  bool hasStsEndpoint() const { return this->stsEndpoint_ != nullptr; }
  std::string stsEndpoint() const {
    DARABONBA_PTR_GET_DEFAULT(stsEndpoint_, "");
  }
  Config &setStsEndpoint(const std::string &stsEndpoint) {
    DARABONBA_PTR_SET_VALUE(stsEndpoint_, stsEndpoint);
    return *this;
  }
  Config &setStsEndpoint(std::string &&stsEndpoint) {
    DARABONBA_PTR_SET_RVALUE(stsEndpoint_, stsEndpoint);
    return *this;
  }

  bool hasStsRegionId() const { return this->stsRegionId_ != nullptr; }
  std::string stsRegionId() const {
    DARABONBA_PTR_GET_DEFAULT(stsRegionId_, "");
  }
  Config &setStsRegionId(const std::string &stsRegionId) {
    DARABONBA_PTR_SET_VALUE(stsRegionId_, stsRegionId);
    return *this;
  }
  Config &setStsRegionId(std::string &&stsRegionId) {
    DARABONBA_PTR_SET_RVALUE(stsRegionId_, stsRegionId);
    return *this;
  }

  bool hasEnableVpc() const { return this->enableVpc_ != nullptr; }
  bool enableVpc() const { DARABONBA_PTR_GET_DEFAULT(enableVpc_, false); }
  Config &setEnableVpc(bool enableVpc) {
    DARABONBA_PTR_SET_VALUE(enableVpc_, enableVpc);
    return *this;
  }

  bool hasType() const { return this->type_ != nullptr; }
  std::string type() const { DARABONBA_PTR_GET_DEFAULT(type_, ""); }
  Config &setType(const std::string &type) {
    DARABONBA_PTR_SET_VALUE(type_, type);
    return *this;
  }
  Config &setType(std::string &&type) {
    DARABONBA_PTR_SET_RVALUE(type_, type);
    return *this;
  }

  bool hasTimeout() const { return this->timeout_ != nullptr; }
  int64_t timeout() const { DARABONBA_PTR_GET_DEFAULT(timeout_, 5000); }
  Config &setTimeout(int64_t timeout) {
    DARABONBA_PTR_SET_VALUE(timeout_, timeout);
    return *this;
  }

  bool hasConnectTimeout() const { return this->connectTimeout_ != nullptr; }
  int64_t connectTimeout() const { DARABONBA_PTR_GET_DEFAULT(connectTimeout_, 10000); }
  Config &setConnectTimeout(int64_t connectTimeout) {
    DARABONBA_PTR_SET_VALUE(connectTimeout_, connectTimeout);
    return *this;
  }

  bool hasDisableIMDSv1() const { return this->disableIMDSv1_ != nullptr; }
  bool disableIMDSv1() const { DARABONBA_PTR_GET_DEFAULT(disableIMDSv1_, false); }
  Config &setDisableIMDSv1(bool disableIMDSv1) {
    DARABONBA_PTR_SET_VALUE(disableIMDSv1_, disableIMDSv1);
    return *this;
  }

  bool hasReuseLastProviderEnabled() const { return this->reuseLastProviderEnabled_ != nullptr; }
  bool reuseLastProviderEnabled() const { DARABONBA_PTR_GET_DEFAULT(reuseLastProviderEnabled_, false); }
  Config &setReuseLastProviderEnabled(bool reuseLastProviderEnabled) {
    DARABONBA_PTR_SET_VALUE(reuseLastProviderEnabled_, reuseLastProviderEnabled);
    return *this;
  }

protected:
  std::shared_ptr<std::string> accessKeyId_ = nullptr;
  std::shared_ptr<std::string> accessKeySecret_ = nullptr;
  std::shared_ptr<std::string> bearerToken_ = nullptr;
  std::shared_ptr<std::string> credentialsURL_ = nullptr;
  std::shared_ptr<int64_t> durationSeconds_ = std::make_shared<int64_t>(3600);
  std::shared_ptr<std::string> externalId_ = nullptr;
  std::shared_ptr<std::string> host_ = nullptr;
  std::shared_ptr<std::string> oidcProviderArn_ = nullptr;
  std::shared_ptr<std::string> oidcTokenFilePath_ = nullptr;
  std::shared_ptr<std::string> policy_ = nullptr;
  std::shared_ptr<std::string> privateKeyFile_ = nullptr;
  std::shared_ptr<std::string> proxy_ = nullptr;
  std::shared_ptr<std::string> publicKeyId_ = nullptr;
  std::shared_ptr<std::string> regionId_ = std::make_shared<std::string>("cn-hangzhou");
  std::shared_ptr<std::string> roleArn_ = nullptr;
  std::shared_ptr<std::string> roleName_ = nullptr;
  std::shared_ptr<int64_t> roleSessionExpiration_ = nullptr;
  std::shared_ptr<std::string> roleSessionName_ = nullptr;  // Dynamic default via generateSessionName()
  std::shared_ptr<std::string> securityToken_ = nullptr;
  std::shared_ptr<std::string> stsEndpoint_ = std::make_shared<std::string>("sts.aliyuncs.com");
  std::shared_ptr<std::string> stsRegionId_ = nullptr;
  std::shared_ptr<bool> enableVpc_ = std::make_shared<bool>(false);
  std::shared_ptr<std::string> type_ = nullptr;
  std::shared_ptr<int64_t> timeout_ = std::make_shared<int64_t>(5000);
  std::shared_ptr<int64_t> connectTimeout_ = std::make_shared<int64_t>(10000);
  std::shared_ptr<bool> disableIMDSv1_ = std::make_shared<bool>(false);
  std::shared_ptr<bool> reuseLastProviderEnabled_ = std::make_shared<bool>(false);
};

} // namespace Credential
} // namespace AlibabaCloud
} // namespace Models
#endif