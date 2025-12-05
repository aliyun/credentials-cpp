#ifndef ALIBABACLOUD_CREDENTIAL_BEARERTOKENPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_BEARERTOKENPROVIDER_HPP_

#include <memory>
#include <string>

#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>

namespace AlibabaCloud {
namespace Credential {
class BearerTokenProvider : public Provider {
public:
  BearerTokenProvider(std::shared_ptr<Models::Config> config) {
    credential_.setBearerToken(config->bearerToken()).setType(Constant::BEARER);
  }
  BearerTokenProvider(const std::string &bearToken) {
    credential_.setBearerToken(bearToken).setType(Constant::BEARER);
  }

  virtual ~BearerTokenProvider() {}

  virtual Models::CredentialModel &getCredential() override { return credential_; }
  virtual const Models::CredentialModel &getCredential() const override {
    return credential_;
  }
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::BEARER; }

protected:
  mutable Models::CredentialModel credential_;
};
} // namespace Credential

} // namespace AlibabaCloud
#endif