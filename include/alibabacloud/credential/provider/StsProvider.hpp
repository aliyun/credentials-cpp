#ifndef AlibabaCloud_CREDENTIAL_STSPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_STSPROVIDER_HPP_

#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>
#include <memory>
#include <string>

namespace AlibabaCloud {
namespace Credential {
class StsProvider : public Provider {
public:
  StsProvider(std::shared_ptr<Models::Config> config) {
    credential_.setAccessKeyId(config->accessKeyId())
        .setAccessKeySecret(config->accessKeySecret())
        .setSecurityToken(config->securityToken())
        .setType(Constant::STS);
  }
  StsProvider(const std::string &accessKeyId,
              const std::string &accessKeySecret,
              const std::string &securityToken) {
    credential_.setAccessKeyId(accessKeyId)
        .setAccessKeySecret(accessKeySecret)
        .setSecurityToken(securityToken)
        .setType(Constant::STS);
  }

  virtual ~StsProvider() {}

  virtual Models::CredentialModel &getCredential() override { return credential_; }
  virtual const Models::CredentialModel &getCredential() const override {
    return credential_;
  }

protected:
  mutable Models::CredentialModel credential_;
};
} // namespace Credential

} // namespace AlibabaCloud
#endif