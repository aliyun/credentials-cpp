#ifndef ALIBABACLOUD_CREDENTIAL_ACCESSKEYPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_ACCESSKEYPROVIDER_HPP_

#include <alibabacloud/credential/Config.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Credential.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>
#include <memory>
#include <string>

namespace Alibabacloud {
namespace Credential {

class AccessKeyProvider : public Provider {
public:
  AccessKeyProvider(std::shared_ptr<Config> config) {
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

  virtual Credential &getCredential() override { return credential_; }
  virtual const Credential &getCredential() const override {
    return credential_;
  }

protected:
  mutable Credential credential_;
};
} // namespace Credential

} // namespace Alibabacloud
#endif