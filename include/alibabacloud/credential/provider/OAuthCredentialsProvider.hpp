#ifndef ALIBABACLOUD_CREDENTIAL_OAUTHCREDENTIALSPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_OAUTHCREDENTIALSPROVIDER_HPP_

#include <memory>
#include <string>

#include <darabonba/Env.hpp>

#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/NeedFreshProvider.hpp>

namespace AlibabaCloud {
namespace Credential {

class OAuthCredentialsProvider : public NeedFreshProvider,
                                  std::enable_shared_from_this<OAuthCredentialsProvider> {
public:
  OAuthCredentialsProvider(std::shared_ptr<Models::Config> config)
      : clientId_(config->hasAccessKeyId() && !config->getAccessKeyId().empty()
                      ? config->getAccessKeyId()
                      : Darabonba::Env::getEnv(Constant::ENV_OAUTH_CLIENT_ID)),
        clientSecret_(config->hasAccessKeySecret() && !config->getAccessKeySecret().empty()
                          ? config->getAccessKeySecret()
                          : Darabonba::Env::getEnv(Constant::ENV_OAUTH_CLIENT_SECRET)),
        tokenEndpoint_(config->hasStsEndpoint() && !config->getStsEndpoint().empty()
                           ? config->getStsEndpoint()
                           : Darabonba::Env::getEnv(Constant::ENV_OAUTH_TOKEN_ENDPOINT)),
        regionId_(config->getRegionId()),
        connectTimeout_(config->hasConnectTimeout() ? config->getConnectTimeout() : 10000),
        readTimeout_(config->hasTimeout() ? config->getTimeout() : 5000) {
    credential_.setType(Constant::OAUTH);
  }

  OAuthCredentialsProvider(const std::string &clientId,
                            const std::string &clientSecret,
                            const std::string &tokenEndpoint,
                            const std::string &regionId = "cn-hangzhou")
      : clientId_(clientId), clientSecret_(clientSecret),
        tokenEndpoint_(tokenEndpoint), regionId_(regionId) {
    credential_.setType(Constant::OAUTH);
  }

  virtual ~OAuthCredentialsProvider() {}
  
  /**
   * @brief Get provider name
   */
  std::string getProviderName() const override { return Constant::OAUTH; }

private:
  virtual bool refreshCredential() const override;

  static const std::string OAUTH_FETCH_ERROR_MSG;

  mutable Models::CredentialModel credential_;
  std::string clientId_;
  std::string clientSecret_;
  std::string tokenEndpoint_;
  std::string regionId_ = "cn-hangzhou";
  int64_t connectTimeout_ = 10000;  // Connection timeout in milliseconds
  int64_t readTimeout_ = 5000;      // Read timeout in milliseconds
};

} // namespace Credential
} // namespace AlibabaCloud

#endif
