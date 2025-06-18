#include <alibabacloud/credential/Credential.hpp>
// #include <alibabacloud/credential/Config.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credential/provider/BearerTokenProvider.hpp>
#include <alibabacloud/credential/provider/DefaultProvider.hpp>
#include <alibabacloud/credential/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credential/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/RamRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/RsaKeyPairProvider.hpp>
#include <alibabacloud/credential/provider/StsProvider.hpp>
#include <alibabacloud/credential/provider/URLProvider.hpp>
namespace AlibabaCloud {
namespace Credential {

std::shared_ptr<Provider> Client::makeProvider(std::shared_ptr<Models::Config> config) {
  if(config == nullptr) {
    return std::shared_ptr<Provider>(new DefaultProvider());
  }

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
    auto p = new EcsRamRoleProvider(config);
    return std::shared_ptr<Provider>(p);
  } else if (type == Constant::RAM_ROLE_ARN) {
    auto p = new RamRoleArnProvider(config);
    return std::shared_ptr<Provider>(p);
  } else if (type == Constant::RSA_KEY_PAIR) {
    auto p = new RsaKeyPairProvider(config);
    return std::shared_ptr<Provider>(p);
  } else if (type == Constant::OIDC_ROLE_ARN) {
    auto p = new OIDCRoleArnProvider(config);
    return std::shared_ptr<Provider>(p);
  } else if(type == Constant::URL_STS) {
    auto p = new URLProvider(config);
    return std::shared_ptr<Provider>(p);
  
  }else {
    auto p = new DefaultProvider();
    return std::shared_ptr<Provider>(p);
  }
}

} // namespace Credential
} // namespace AlibabaCloud
