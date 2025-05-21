#include <alibabacloud/provider/DefaultProvider.hpp>
#include <alibabacloud/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/provider/EnvironmentVariableProvider.hpp>
#include <alibabacloud/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/provider/ProfileProvider.hpp>
#include <alibabacloud/provider/URLProvider.hpp>
namespace AlibabaCloud {
namespace Credential {

DefaultProvider::DefaultProvider() {

  providers_.emplace_back(new EnvironmentVariableProvider());

  auto oidcTokenFile = Darabonba::Env::getEnv("ALIBABA_CLOUD_OIDC_TOKEN_FILE"),
       roleArn = Darabonba::Env::getEnv("ALIBABA_CLOUD_ROLE_ARN"),
       oidcProviderArn =
           Darabonba::Env::getEnv("ALIBABA_CLOUD_OIDC_PROVIDER_ARN");
  if (!oidcTokenFile.empty() && !roleArn.empty() && !oidcProviderArn.empty()) {
    auto roleSessionName =
        Darabonba::Env::getEnv("ALIBABA_CLOUD_ROLE_SESSION_NAME");
    providers_.emplace_back(new OIDCRoleArnProvider(
        roleArn, oidcProviderArn, oidcTokenFile, roleSessionName));
  }

  providers_.emplace_back(new ProfileProvider());

  auto ecsMetaData = Darabonba::Env::getEnv("ALIBABA_CLOUD_ECS_METADATA");
  if (!ecsMetaData.empty()) {
    providers_.emplace_back(new EcsRamRoleProvider(ecsMetaData));
  }

  auto url = Darabonba::Env::getEnv("ALIBABA_CLOUD_CREDENTIALS_URI");
  if (!url.empty()) {
    providers_.emplace_back(new URLProvider(url));
  }
}
} // namespace Credential
} // namespace AlibabaCloud