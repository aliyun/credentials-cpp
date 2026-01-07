#ifndef ALIBABACLOUD_CREDENTIAL_ACCESSKEYPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_ACCESSKEYPROVIDER_HPP_

#include <memory>
#include <string>

#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>

namespace AlibabaCloud {
namespace Credential {

class AccessKeyProvider : public Provider {
public:
  AccessKeyProvider(std::shared_ptr<Models::Config> config) {
    credential_.setAccessKeyId(config->getAccessKeyId())
        .setAccessKeySecret(config->getAccessKeySecret())
        .setType(Constant::ACCESS_KEY);
  }
  AccessKeyProvider(const std::string &accessKeyId,
                    const std::string &accessKeySecret) {
    credential_.setAccessKeyId(accessKeyId)
        .setAccessKeySecret(accessKeySecret)
        .setType(Constant::ACCESS_KEY);
  }
  virtual ~AccessKeyProvider() {}

  virtual Models::CredentialModel &getCredential() override { return credential_; }
  virtual const Models::CredentialModel &getCredential() const override {
    return credential_;
  }
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::ACCESS_KEY; }

protected:
  mutable Models::CredentialModel credential_;
};
} // namespace Credential

} // namespace AlibabaCloud
#endif