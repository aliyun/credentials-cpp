#ifndef AlibabaCloud_CREDENTIAL_BEARERTOKENPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_BEARERTOKENPROVIDER_HPP_

#include <alibabacloud/Config.hpp>
#include <alibabacloud/Constant.hpp>
#include <alibabacloud/provider/Provider.hpp>
#include <memory>
#include <string>

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