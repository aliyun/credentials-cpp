#include <alibabacloud/credentials/Client.hpp>
#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credentials/provider/BearerTokenProvider.hpp>
#include <alibabacloud/credentials/provider/CloudSSOCredentialsProvider.hpp>
#include <alibabacloud/credentials/provider/DefaultProvider.hpp>
#include <alibabacloud/credentials/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credentials/provider/OAuthCredentialsProvider.hpp>
#include <alibabacloud/credentials/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credentials/provider/RamRoleArnProvider.hpp>
#include <alibabacloud/credentials/provider/RsaKeyPairProvider.hpp>
#include <alibabacloud/credentials/provider/StsProvider.hpp>
#include <alibabacloud/credentials/provider/URLProvider.hpp>
#include <utility>

namespace AlibabaCloud {
namespace Credentials {

// Default constructor
Client::Client() : provider_(std::make_shared<DefaultProvider>()) {}

// Config-based constructors
Client::Client(const Models::Config &obj)
    : config_(std::make_shared<Models::Config>(obj)),
      provider_(makeProvider(config_)) {}

Client::Client(Models::Config &&obj)
    : config_(std::make_shared<Models::Config>(std::move(obj))),
      provider_(makeProvider(config_)) {}

Client::Client(std::shared_ptr<Models::Config> config)
    : config_(std::move(config)), provider_(makeProvider(config_)) {}

// Provider-based constructor
Client::Client(std::shared_ptr<Provider> provider)
    : provider_(std::move(provider)) {}

std::shared_ptr<Provider>
Client::makeProvider(std::shared_ptr<Models::Config> config) {
  if (config == nullptr) {
    return std::make_shared<DefaultProvider>();
  }

  const auto type = config->getType();

  if (type == Constant::ACCESS_KEY) {
    const auto p = new AccessKeyProvider(config);
    return std::shared_ptr<Provider>(p);
  }
  if (type == Constant::BEARER) {
    const auto p = new BearerTokenProvider(config);
    return std::shared_ptr<Provider>(p);
  }
  if (type == Constant::STS) {
    const auto p = new StsProvider(config);
    return std::shared_ptr<Provider>(p);
  }
  if (type == Constant::ECS_RAM_ROLE) {
    const auto p = new EcsRamRoleProvider(config);
    return std::shared_ptr<Provider>(p);
  }
  if (type == Constant::RAM_ROLE_ARN) {
    const auto p = new RamRoleArnProvider(config);
    return std::shared_ptr<Provider>(p);
  }
  if (type == Constant::RSA_KEY_PAIR) {
    const auto p = new RsaKeyPairProvider(config);
    return std::shared_ptr<Provider>(p);
  }
  if (type == Constant::OIDC_ROLE_ARN) {
    auto p = new OIDCRoleArnProvider(config);
    return std::shared_ptr<Provider>(p);
  }
  if (type == Constant::URL_STS) {
    const auto p = new URLProvider(config);
    return std::shared_ptr<Provider>(p);
  }
  if (type == Constant::CLOUD_SSO) {
    const auto p = new CloudSSOCredentialsProvider(config);
    return std::shared_ptr<Provider>(p);
  }
  if (type == Constant::OAUTH) {
    const auto p = new OAuthCredentialsProvider(config);
    return std::shared_ptr<Provider>(p);
  }
  const auto p = new DefaultProvider();
  return std::shared_ptr<Provider>(p);
}

} // namespace Credentials
} // namespace AlibabaCloud
