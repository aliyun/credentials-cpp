#ifndef ALIBABACLOUD_CREDENTIAL_CREDENTIAL_HPP_
#define ALIBABACLOUD_CREDENTIAL_CREDENTIAL_HPP_

#include <alibabacloud/credential/Config.hpp>
#include <darabonba/Model.hpp>
#include <memory>
#include <string>

namespace Alibabacloud {
namespace Credential {

class Credential : public Darabonba::Model {
  friend void to_json(Darabonba::Json &j, const Credential &obj) {
    DARABONBA_PTR_TO_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_TO_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_TO_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_TO_JSON(securityToken, securityToken_);
    DARABONBA_PTR_TO_JSON(type, type_);
  }

  friend void from_json(const Darabonba::Json &j, Credential &obj) {
    DARABONBA_PTR_FROM_JSON(accessKeyId, accessKeyId_);
    DARABONBA_PTR_FROM_JSON(accessKeySecret, accessKeySecret_);
    DARABONBA_PTR_FROM_JSON(bearerToken, bearerToken_);
    DARABONBA_PTR_FROM_JSON(securityToken, securityToken_);
    DARABONBA_PTR_FROM_JSON(type, type_);
  }

public:
  Credential() = default;
  Credential(const Credential &) = default;
  Credential(Credential &&) = default;
  Credential(const Darabonba::Json &obj) { from_json(obj, *this); }

  virtual ~Credential() = default;

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
           type_ == nullptr;
  }

  bool hasAccessKeyId() const { return this->accessKeyId_ != nullptr; }
  std::string accessKeyId() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeyId_, "");
  }
  Credential &setAccessKeyId(const std::string &accessKeyId) {
    DARABONBA_PTR_SET_VALUE(accessKeyId_, accessKeyId);
  }
  Credential &setAccessKeyId(std::string &&accessKeyId) {
    DARABONBA_PTR_SET_RVALUE(accessKeyId_, accessKeyId);
  }

  bool hasAccessKeySecret() const { return this->accessKeySecret_ != nullptr; }
  std::string accessKeySecret() const {
    DARABONBA_PTR_GET_DEFAULT(accessKeySecret_, "");
  }
  Credential &setAccessKeySecret(const std::string &accessKeySecret) {
    DARABONBA_PTR_SET_VALUE(accessKeySecret_, accessKeySecret);
  }
  Credential &setAccessKeySecret(std::string &&accessKeySecret) {
    DARABONBA_PTR_SET_RVALUE(accessKeySecret_, accessKeySecret);
  }

  bool hasBearerToken() const { return this->bearerToken_ != nullptr; }
  std::string bearerToken() const {
    DARABONBA_PTR_GET_DEFAULT(bearerToken_, "");
  }
  Credential &setBearerToken(const std::string &bearerToken) {
    DARABONBA_PTR_SET_VALUE(bearerToken_, bearerToken);
  }
  Credential &setBearerToken(std::string &&bearerToken) {
    DARABONBA_PTR_SET_RVALUE(bearerToken_, bearerToken);
  }

  bool hasSecurityToken() const { return this->securityToken_ != nullptr; }
  std::string securityToken() const {
    DARABONBA_PTR_GET_DEFAULT(securityToken_, "");
  }
  Credential &setSecurityToken(const std::string &securityToken) {
    DARABONBA_PTR_SET_VALUE(securityToken_, securityToken);
  }
  Credential &setSecurityToken(std::string &&securityToken) {
    DARABONBA_PTR_SET_RVALUE(securityToken_, securityToken);
  }

  bool hasType() const { return this->type_ != nullptr; }
  std::string type() const { DARABONBA_PTR_GET_DEFAULT(type_, ""); }
  Credential &setType(const std::string &type) {
    DARABONBA_PTR_SET_VALUE(type_, type);
  }
  Credential &setType(std::string &&type) {
    DARABONBA_PTR_SET_RVALUE(type_, type);
  }

protected:
  std::shared_ptr<std::string> accessKeyId_ = nullptr;
  std::shared_ptr<std::string> accessKeySecret_ = nullptr;
  std::shared_ptr<std::string> bearerToken_ = nullptr;
  std::shared_ptr<std::string> securityToken_ = nullptr;
  std::shared_ptr<std::string> type_ = nullptr;
};

} // namespace Credential
} // namespace Alibabacloud
#endif