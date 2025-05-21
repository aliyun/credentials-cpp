#ifndef AlibabaCloud_CREDENTIAL_URLPROVIDER_HPP_
#define AlibabaCloud_CREDENTIAL_URLPROVIDER_HPP_
#include <alibabacloud/Config.hpp>
#include <alibabacloud/Constant.hpp>
// #include <alibabacloud/provider/NeedFreshProvider.hpp>
#include <string>

namespace AlibabaCloud {
namespace Credential {

class URLProvider : public NeedFreshProvider,
                    std::enable_shared_from_this<URLProvider> {
public:

  URLProvider(std::shared_ptr<Models::Config> config) : url_(config->credentialsURL()) {
    credential_.setType(Constant::URL_STS);
  }

  URLProvider(const std::string &url) : url_(url) {
    credential_.setType(Constant::URL_STS);
  }


  virtual ~URLProvider() {}

protected:
  // virtual bool refreshCredential() const override;

  std::string url_;
  mutable Models::Credential credential_;
};

} // namespace Credential

} // namespace AlibabaCloud

#endif