#ifndef ALIBABACLOUD_CREDENTIAL_MODEL_HPP_
#define ALIBABACLOUD_CREDENTIAL_MODEL_HPP_

#include <memory>

#include <darabonba/Model.hpp>

// Forward declaration to avoid circular dependency
namespace AlibabaCloud {
namespace Credential {
class AuthUtil;
}
} // namespace AlibabaCloud

namespace AlibabaCloud {
namespace Credential {
namespace Models {
class CredentialModel : public Darabonba::Model {
public:
  friend void to_json(Darabonba::Json &j, const CredentialModel &obj) {
    DARABONBA_PTR_TO_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_TO_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_TO_JSON(securityToken, securityToken_);
    DARABONBA_PTR_TO_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_TO_JSON(type, type_);
    DARABONBA_PTR_TO_JSON(providerName, providerName_);
  };
  friend void from_json(const Darabonba::Json &j, CredentialModel &obj) {
    DARABONBA_PTR_FROM_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_FROM_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_FROM_JSON(securityToken, securityToken_);
    DARABONBA_PTR_FROM_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_FROM_JSON(type, type_);
    DARABONBA_PTR_FROM_JSON(providerName, providerName_);
  };
  CredentialModel() = default;
  CredentialModel(const CredentialModel &) = default;
  CredentialModel(CredentialModel &&) = default;
  CredentialModel(const Darabonba::Json &obj) { from_json(obj, *this); };
  virtual ~CredentialModel() = default;
  CredentialModel &operator=(const CredentialModel &) = default;
  CredentialModel &operator=(CredentialModel &&) = default;
  virtual void validate() const override {};
  virtual void fromMap(const Darabonba::Json &obj) override {
    from_json(obj, *this);
    validate();
  };
  virtual Darabonba::Json toMap() const override {
    Darabonba::Json obj;
    to_json(obj, *this);
    return obj;
  };
  virtual bool empty() const override {
    return this->accessKeyId_ == nullptr && this->accessKeySecret_ == nullptr &&
           this->securityToken_ == nullptr && this->bearerToken_ == nullptr &&
           this->type_ == nullptr && this->providerName_ == nullptr;
  };
  // accessKeyId Field Functions
  bool hasAccessKeyId() const { return this->accessKeyId_ != nullptr; };
  void deleteAccessKeyId() { this->accessKeyId_ = nullptr; };
  inline string getAccessKeyId() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeyId_, "")
  };
  inline CredentialModel &setAccessKeyId(string accessKeyId) {
    DARABONBA_PTR_SET_VALUE(accessKeyId_, accessKeyId)
  };

  // accessKeySecret Field Functions
  bool hasAccessKeySecret() const { return this->accessKeySecret_ != nullptr; };
  void deleteAccessKeySecret() { this->accessKeySecret_ = nullptr; };
  inline string getAccessKeySecret() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeySecret_, "")
  };
  inline CredentialModel &setAccessKeySecret(string accessKeySecret) {
    DARABONBA_PTR_SET_VALUE(accessKeySecret_, accessKeySecret)
  };

  // securityToken Field Functions
  bool hasSecurityToken() const { return this->securityToken_ != nullptr; };
  void deleteSecurityToken() { this->securityToken_ = nullptr; };
  inline string getSecurityToken() const {
    DARABONBA_PTR_GET_DEFAULT(securityToken_, "")
  };
  inline CredentialModel &setSecurityToken(string securityToken) {
    DARABONBA_PTR_SET_VALUE(securityToken_, securityToken)
  };

  // bearerToken Field Functions
  bool hasBearerToken() const { return this->bearerToken_ != nullptr; };
  void deleteBearerToken() { this->bearerToken_ = nullptr; };
  inline string getBearerToken() const {
    DARABONBA_PTR_GET_DEFAULT(bearerToken_, "")
  };
  inline CredentialModel &setBearerToken(string bearerToken) {
    DARABONBA_PTR_SET_VALUE(bearerToken_, bearerToken)
  };

  // type Field Functions
  bool hasType() const { return this->type_ != nullptr; };
  void deleteType() { this->type_ = nullptr; };
  inline string getType() const { DARABONBA_PTR_GET_DEFAULT(type_, "") };
  inline CredentialModel &setType(string type) {
    DARABONBA_PTR_SET_VALUE(type_, type)
  };

  // providerName Field Functions
  bool hasProviderName() const { return this->providerName_ != nullptr; };
  void deleteProviderName() { this->providerName_ = nullptr; };
  inline string getProviderName() const {
    DARABONBA_PTR_GET_DEFAULT(providerName_, "")
  };
  inline CredentialModel &setProviderName(string providerName) {
    DARABONBA_PTR_SET_VALUE(providerName_, providerName)
  };

protected:
  // accesskey id
  shared_ptr<string> accessKeyId_{};
  // accesskey secret
  shared_ptr<string> accessKeySecret_{};
  // security token
  shared_ptr<string> securityToken_{};
  // bearer token
  shared_ptr<string> bearerToken_{};
  // type
  shared_ptr<string> type_{};
  // provider name
  shared_ptr<string> providerName_{};
};
/**
 * Model for initing credential
 */
class Config : public Darabonba::Model {
public:
  friend void to_json(Darabonba::Json &j, const Config &obj) {
    DARABONBA_PTR_TO_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_TO_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_TO_JSON(securityToken, securityToken_);
    DARABONBA_PTR_TO_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_TO_JSON(durationSeconds, durationSeconds_);
    DARABONBA_PTR_TO_JSON(roleArn, roleArn_);
    DARABONBA_PTR_TO_JSON(policy, policy_);
    DARABONBA_PTR_TO_JSON(roleSessionExpiration, roleSessionExpiration_);
    DARABONBA_PTR_TO_JSON(roleSessionName, roleSessionName_);
    DARABONBA_PTR_TO_JSON(publicKeyId, publicKeyId_);
    DARABONBA_PTR_TO_JSON(privateKeyFile, privateKeyFile_);
    DARABONBA_PTR_TO_JSON(roleName, roleName_);
    DARABONBA_PTR_TO_JSON(credentialsUri, credentialsUri_);
    DARABONBA_PTR_TO_JSON(type, type_);
    DARABONBA_PTR_TO_JSON(STSEndpoint, STSEndpoint_);
    DARABONBA_PTR_TO_JSON(externalId, externalId_);
  };
  friend void from_json(const Darabonba::Json &j, Config &obj) {
    DARABONBA_PTR_FROM_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_FROM_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_FROM_JSON(securityToken, securityToken_);
    DARABONBA_PTR_FROM_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_FROM_JSON(durationSeconds, durationSeconds_);
    DARABONBA_PTR_FROM_JSON(roleArn, roleArn_);
    DARABONBA_PTR_FROM_JSON(policy, policy_);
    DARABONBA_PTR_FROM_JSON(roleSessionExpiration, roleSessionExpiration_);
    DARABONBA_PTR_FROM_JSON(roleSessionName, roleSessionName_);
    DARABONBA_PTR_FROM_JSON(publicKeyId, publicKeyId_);
    DARABONBA_PTR_FROM_JSON(privateKeyFile, privateKeyFile_);
    DARABONBA_PTR_FROM_JSON(roleName, roleName_);
    DARABONBA_PTR_FROM_JSON(credentialsUri, credentialsUri_);
    DARABONBA_PTR_FROM_JSON(type, type_);
    DARABONBA_PTR_FROM_JSON(STSEndpoint, STSEndpoint_);
    DARABONBA_PTR_FROM_JSON(externalId, externalId_);
  };
  Config() = default;
  Config(const Config &) = default;
  Config(Config &&) = default;
  Config(const Darabonba::Json &obj) { from_json(obj, *this); };
  virtual ~Config() = default;
  Config &operator=(const Config &) = default;
  Config &operator=(Config &&) = default;
  virtual void validate() const override {};
  virtual void fromMap(const Darabonba::Json &obj) override {
    from_json(obj, *this);
    validate();
  };
  virtual Darabonba::Json toMap() const override {
    Darabonba::Json obj;
    to_json(obj, *this);
    return obj;
  };
  virtual bool empty() const override {
    return this->accessKeyId_ == nullptr && this->accessKeySecret_ == nullptr &&
           this->securityToken_ == nullptr && this->bearerToken_ == nullptr &&
           this->durationSeconds_ == nullptr && this->roleArn_ == nullptr &&
           this->policy_ == nullptr &&
           this->roleSessionExpiration_ == nullptr &&
           this->roleSessionName_ == nullptr && this->publicKeyId_ == nullptr &&
           this->privateKeyFile_ == nullptr && this->roleName_ == nullptr &&
           this->credentialsUri_ == nullptr && this->type_ == nullptr &&
           this->STSEndpoint_ == nullptr && this->externalId_ == nullptr;
  };
  // accessKeyId Field Functions
  bool hasAccessKeyId() const { return this->accessKeyId_ != nullptr; };
  void deleteAccessKeyId() { this->accessKeyId_ = nullptr; };
  inline string getAccessKeyId() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeyId_, "")
  };
  inline Config &setAccessKeyId(string accessKeyId) {
    DARABONBA_PTR_SET_VALUE(accessKeyId_, accessKeyId)
  };

  // accessKeySecret Field Functions
  bool hasAccessKeySecret() const { return this->accessKeySecret_ != nullptr; };
  void deleteAccessKeySecret() { this->accessKeySecret_ = nullptr; };
  inline string getAccessKeySecret() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeySecret_, "")
  };
  inline Config &setAccessKeySecret(string accessKeySecret) {
    DARABONBA_PTR_SET_VALUE(accessKeySecret_, accessKeySecret)
  };

  // securityToken Field Functions
  bool hasSecurityToken() const { return this->securityToken_ != nullptr; };
  void deleteSecurityToken() { this->securityToken_ = nullptr; };
  inline string getSecurityToken() const {
    DARABONBA_PTR_GET_DEFAULT(securityToken_, "")
  };
  inline Config &setSecurityToken(string securityToken) {
    DARABONBA_PTR_SET_VALUE(securityToken_, securityToken)
  };

  // bearerToken Field Functions
  bool hasBearerToken() const { return this->bearerToken_ != nullptr; };
  void deleteBearerToken() { this->bearerToken_ = nullptr; };
  inline string getBearerToken() const {
    DARABONBA_PTR_GET_DEFAULT(bearerToken_, "")
  };
  inline Config &setBearerToken(string bearerToken) {
    DARABONBA_PTR_SET_VALUE(bearerToken_, bearerToken)
  };

  // durationSeconds Field Functions
  bool hasDurationSeconds() const { return this->durationSeconds_ != nullptr; };
  void deleteDurationSeconds() { this->durationSeconds_ = nullptr; };
  inline int64_t getDurationSeconds() const {
    DARABONBA_PTR_GET_DEFAULT(durationSeconds_, 0)
  };
  inline Config &setDurationSeconds(int64_t durationSeconds) {
    DARABONBA_PTR_SET_VALUE(durationSeconds_, durationSeconds)
  };

  // roleArn Field Functions
  bool hasRoleArn() const { return this->roleArn_ != nullptr; };
  void deleteRoleArn() { this->roleArn_ = nullptr; };
  inline string getRoleArn() const { DARABONBA_PTR_GET_DEFAULT(roleArn_, "") };
  inline Config &setRoleArn(string roleArn) {
    DARABONBA_PTR_SET_VALUE(roleArn_, roleArn)
  };

  // policy Field Functions
  bool hasPolicy() const { return this->policy_ != nullptr; };
  void deletePolicy() { this->policy_ = nullptr; };
  inline string getPolicy() const { DARABONBA_PTR_GET_DEFAULT(policy_, "") };
  inline Config &setPolicy(string policy) {
    DARABONBA_PTR_SET_VALUE(policy_, policy)
  };

  // roleSessionExpiration Field Functions
  bool hasRoleSessionExpiration() const {
    return this->roleSessionExpiration_ != nullptr;
  };
  void deleteRoleSessionExpiration() {
    this->roleSessionExpiration_ = nullptr;
  };
  inline int64_t getRoleSessionExpiration() const {
    DARABONBA_PTR_GET_DEFAULT(roleSessionExpiration_, 0)
  };
  inline Config &setRoleSessionExpiration(int64_t roleSessionExpiration) {
    DARABONBA_PTR_SET_VALUE(roleSessionExpiration_, roleSessionExpiration)
  };

  // roleSessionName Field Functions
  bool hasRoleSessionName() const { return this->roleSessionName_ != nullptr; };
  void deleteRoleSessionName() { this->roleSessionName_ = nullptr; };
  inline string getRoleSessionName() const {
    DARABONBA_PTR_GET_DEFAULT(roleSessionName_, "")
  };
  inline Config &setRoleSessionName(string roleSessionName) {
    DARABONBA_PTR_SET_VALUE(roleSessionName_, roleSessionName)
  };

  // publicKeyId Field Functions
  bool hasPublicKeyId() const { return this->publicKeyId_ != nullptr; };
  void deletePublicKeyId() { this->publicKeyId_ = nullptr; };
  inline string getPublicKeyId() const {
    DARABONBA_PTR_GET_DEFAULT(publicKeyId_, "")
  };
  inline Config &setPublicKeyId(string publicKeyId) {
    DARABONBA_PTR_SET_VALUE(publicKeyId_, publicKeyId)
  };

  // privateKeyFile Field Functions
  bool hasPrivateKeyFile() const { return this->privateKeyFile_ != nullptr; };
  void deletePrivateKeyFile() { this->privateKeyFile_ = nullptr; };
  inline string getPrivateKeyFile() const {
    DARABONBA_PTR_GET_DEFAULT(privateKeyFile_, "")
  };
  inline Config &setPrivateKeyFile(string privateKeyFile) {
    DARABONBA_PTR_SET_VALUE(privateKeyFile_, privateKeyFile)
  };

  // roleName Field Functions
  bool hasRoleName() const { return this->roleName_ != nullptr; };
  void deleteRoleName() { this->roleName_ = nullptr; };
  inline string getRoleName() const {
    DARABONBA_PTR_GET_DEFAULT(roleName_, "")
  };
  inline Config &setRoleName(string roleName) {
    DARABONBA_PTR_SET_VALUE(roleName_, roleName)
  };

  // credentialsUri Field Functions
  bool hasCredentialsUri() const { return this->credentialsUri_ != nullptr; };
  void deleteCredentialsUri() { this->credentialsUri_ = nullptr; };
  inline string getCredentialsUri() const {
    DARABONBA_PTR_GET_DEFAULT(credentialsUri_, "")
  };
  inline Config &setCredentialsUri(string credentialsUri) {
    DARABONBA_PTR_SET_VALUE(credentialsUri_, credentialsUri)
  };

  // type Field Functions
  bool hasType() const { return this->type_ != nullptr; };
  void deleteType() { this->type_ = nullptr; };
  inline string getType() const { DARABONBA_PTR_GET_DEFAULT(type_, "") };
  inline Config &setType(string type) { DARABONBA_PTR_SET_VALUE(type_, type) };

  // STSEndpoint Field Functions
  bool hasSTSEndpoint() const { return this->STSEndpoint_ != nullptr; };
  void deleteSTSEndpoint() { this->STSEndpoint_ = nullptr; };
  inline string getSTSEndpoint() const {
    DARABONBA_PTR_GET_DEFAULT(STSEndpoint_, "")
  };
  inline Config &setSTSEndpoint(string STSEndpoint) {
    DARABONBA_PTR_SET_VALUE(STSEndpoint_, STSEndpoint)
  };

  // externalId Field Functions
  bool hasExternalId() const { return this->externalId_ != nullptr; };
  void deleteExternalId() { this->externalId_ = nullptr; };
  inline string getExternalId() const {
    DARABONBA_PTR_GET_DEFAULT(externalId_, "")
  };
  inline Config &setExternalId(string externalId) {
    DARABONBA_PTR_SET_VALUE(externalId_, externalId)
  };

protected:
  // accesskey id
  shared_ptr<string> accessKeyId_{};
  // accesskey secret
  shared_ptr<string> accessKeySecret_{};
  // security token
  shared_ptr<string> securityToken_{};
  // bearer token
  shared_ptr<string> bearerToken_{};
  // duration seconds
  shared_ptr<int64_t> durationSeconds_{};
  // role arn
  shared_ptr<string> roleArn_{};
  // policy
  shared_ptr<string> policy_{};
  // role session expiration
  shared_ptr<int64_t> roleSessionExpiration_{};
  // role session name
  shared_ptr<string> roleSessionName_{};
  // publicKey id
  shared_ptr<string> publicKeyId_{};
  // privateKey file
  shared_ptr<string> privateKeyFile_{};
  // role name
  shared_ptr<string> roleName_{};
  // credentials uri
  shared_ptr<string> credentialsUri_{};
  // credential type
  shared_ptr<string> type_{};
  // sts endpoint
  shared_ptr<string> STSEndpoint_{};
  // external id for ram role arn
  shared_ptr<string> externalId_{};
};

} // namespace Models
} // namespace Credential
} // namespace AlibabaCloud
#endif