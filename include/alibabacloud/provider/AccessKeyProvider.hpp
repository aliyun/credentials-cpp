#ifndef AlibabaCloud_CREDENTIAL_ACCESSKEYPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_ACCESSKEYPROVIDER_HPP_

#include <alibabacloud/Config.hpp>
#include <alibabacloud/Constant.hpp>
#include <alibabacloud/Credential.hpp>
#include <alibabacloud/provider/Provider.hpp>
#include <memory>
#include <string>

namespace AlibabaCloud {
namespace Credential {

class AccessKeyProvider : public Provider {
public:
  AccessKeyProvider(std::shared_ptr<Models::Config> config) {
    credential_.setAccessKeyId(config->accessKeyId())
        .setAccessKeySecret(config->accessKeySecret())
        .setType(Constant::ACCESS_KEY);
  }
  AccessKeyProvider(const std::string &accessKeyId,
                    const std::string &accessKeySecret) {
    credential_.setAccessKeyId(accessKeyId)
        .setAccessKeySecret(accessKeySecret)
        .setType(Constant::ACCESS_KEY);
  }
  virtual ~AccessKeyProvider() {}

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