#include <alibabacloud/credential/Credential.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credential/provider/BearerTokenProvider.hpp>
#include <alibabacloud/credential/provider/CloudSSOCredentialsProvider.hpp>
#include <alibabacloud/credential/provider/DefaultProvider.hpp>
#include <alibabacloud/credential/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credential/provider/OAuthCredentialsProvider.hpp>
#include <alibabacloud/credential/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/RamRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/RsaKeyPairProvider.hpp>
#include <alibabacloud/credential/provider/StsProvider.hpp>
#include <alibabacloud/credential/provider/URLProvider.hpp>
#include <utility>

namespace AlibabaCloud {
namespace Credential {

// Constructor 1: Default constructor
// Equivalent to Java: public Client()
Client::Client() : provider_(std::make_shared<DefaultProvider>()) {}

// Constructor 2: Config-based constructors
// Equivalent to Java: public Client(Config config)
Client::Client(const Models::Config &obj)
    : config_(std::make_shared<Models::Config>(obj)),
      provider_(makeProvider(config_)) {}

Client::Client(Models::Config &&obj)
    : config_(std::make_shared<Models::Config>(std::move(obj))),
      provider_(makeProvider(config_)) {}

Client::Client(std::shared_ptr<Models::Config> config)
    : config_(std::move(config)), provider_(makeProvider(config_)) {}

// Constructor 3: Provider-based constructor
// Equivalent to Java: public Client(AlibabaCloudCredentialsProvider provider)
Client::Client(std::shared_ptr<Provider> provider)
    : provider_(std::move(provider)) {}

std::shared_ptr<Provider>
Client::makeProvider(std::shared_ptr<Models::Config> config) {
  if (config == nullptr) {
    return std::make_shared<DefaultProvider>();
  }

  const auto type = config->type();

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

} // namespace Credential
} // namespace AlibabaCloud
