#ifndef ALIBABACLOUD_CREDENTIAL_URLPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_URLPROVIDER_HPP_
#include <alibabacloud/credential/Config.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/NeedFreshProvider.hpp>
#include <string>

namespace Alibabacloud {
namespace Credential {

class URLProvider : public NeedFreshProvider,
                    std::enable_shared_from_this<URLProvider> {
public:

  URLProvider(std::shared_ptr<Config> config) : url_(config->credentialsURL()) {
    credential_.setType(Constant::URL_STS);
  }

  URLProvider(const std::string &url) : url_(url) {
    credential_.setType(Constant::URL_STS);
  }


  virtual ~URLProvider() {}

protected:
  virtual bool refreshCredential() const override;

  std::string url_;
  mutable Credential credential_;
};

} // namespace Credential

} // namespace Alibabacloud

#endif