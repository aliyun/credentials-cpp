#include <alibabacloud/Client.hpp>
#include <alibabacloud/Config.hpp>
#include <alibabacloud/Constant.hpp>
#include <alibabacloud/provider/AccessKeyProvider.hpp>
#include <alibabacloud/provider/BearerTokenProvider.hpp>
#include <alibabacloud/provider/DefaultProvider.hpp>
#include <alibabacloud/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/provider/RamRoleArnProvider.hpp>
#include <alibabacloud/provider/RsaKeyPairProvider.hpp>
#include <alibabacloud/provider/StsProvider.hpp>
#include <alibabacloud/provider/URLProvider.hpp>
namespace AlibabaCloud {
namespace Credential {
namespace Models {

std::shared_ptr<Provider> Models::Client::makeProvider(std::shared_ptr<Config> config) {

  auto type = config->type();

  if (type == Constant::ACCESS_KEY) {
    auto p = new AccessKeyProvider(config);
    return std::shared_ptr<Provider>(p);
  } else if (type == Constant::BEARER) {
    auto p = new BearerTokenProvider(config);
    return std::shared_ptr<Provider>(p);
  } else if (type == Constant::STS) {
    auto p = new StsProvider(config);
    return std::shared_ptr<Provider>(p);
  } else if (type == Constant::ECS_RAM_ROLE) {
    // auto p = new EcsRamRoleProvider(config);
    // return std::shared_ptr<Provider>(p);
  } else if (type == Constant::RAM_ROLE_ARN) {
    // auto p = new RamRoleArnProvider(config);
    // return std::shared_ptr<Provider>(p);
  } else if (type == Constant::RSA_KEY_PAIR) {
    // auto p = new RsaKeyPairProvider(config);
    // return std::shared_ptr<Provider>(p);
  } else if (type == Constant::OIDC_ROLE_ARN) {
    // auto p = new OIDCRoleArnProvider(config);
    // return std::shared_ptr<Provider>(p);
  } else if(type == Constant::URL_STS) {
    // auto p = new URLProvider(config);
    // return std::shared_ptr<Provider>(p);
  }else {
    // auto p = new DefaultProvider();
    // return std::shared_ptr<Provider>(p);
  }
}
} // namespace Models
} // namespace Credential
} // namespace AlibabaCloud
