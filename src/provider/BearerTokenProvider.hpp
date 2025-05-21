#ifndef ALIBABACLOUD_CREDENTIAL_BEARERTOKENPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_BEARERTOKENPROVIDER_HPP_

#include <alibabacloud/credential/Config.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/Provider.hpp>
#include <memory>
#include <string>

namespace Alibabacloud {
namespace Credential {
class BearerTokenProvider : public Provider {
public:
  BearerTokenProvider(std::shared_ptr<Config> config) {
    credential_.setBearerToken(config->bearerToken()).setType(Constant::BEARER);
  }
  BearerTokenProvider(const std::string &bearToken) {
    credential_.setBearerToken(bearToken).setType(Constant::BEARER);
  }

  virtual ~BearerTokenProvider() {}

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