#ifndef AlibabaCloud_CREDENTIAL_STSPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_STSPROVIDER_HPP_

#include <alibabacloud/Config.hpp>
#include <alibabacloud/Constant.hpp>
#include <alibabacloud/provider/Provider.hpp>
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

  virtual Models::Credential &getCredential() override { return credential_; }
  virtual const Models::Credential &getCredential() const override {
    return credential_;
  }

protected:
  mutable Models::Credential credential_;
};
} // namespace Credential

} // namespace AlibabaCloud
#endif