#include <darabonba/Env.hpp>
#include <alibabacloud/credentials/Exception.hpp>

#include <alibabacloud/credentials/provider/DefaultProvider.hpp>
#include <alibabacloud/credentials/provider/CLIProfileProvider.hpp>
#include <alibabacloud/credentials/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credentials/provider/EnvironmentVariableProvider.hpp>
#include <alibabacloud/credentials/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credentials/provider/ProfileProvider.hpp>
#include <alibabacloud/credentials/provider/URLProvider.hpp>
namespace AlibabaCloud {
namespace Credentials {

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

  // Add CLIProfileProvider
  providers_.emplace_back(new CLIProfileProvider());

  providers_.emplace_back(new ProfileProvider());

  // Check if ECS metadata access is disabled
  auto ecsMetadataDisabled =
      Darabonba::Env::getEnv("ALIBABA_CLOUD_ECS_METADATA_DISABLED");
  if (ecsMetadataDisabled != "true" && ecsMetadataDisabled != "True" &&
      ecsMetadataDisabled != "TRUE") {
    auto ecsMetaData = Darabonba::Env::getEnv("ALIBABA_CLOUD_ECS_METADATA");
    if (!ecsMetaData.empty()) {
      providers_.emplace_back(new EcsRamRoleProvider(ecsMetaData));
    }
  }

  auto url = Darabonba::Env::getEnv("ALIBABA_CLOUD_CREDENTIALS_URI");
  if (!url.empty()) {
    providers_.emplace_back(new URLProvider(url));
  }
}

DefaultProvider::DefaultProvider(std::shared_ptr<Models::Config> config)
    : reuseLastProviderEnabled_(config->getReuseLastProviderEnabled()) {

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

  // Add CLIProfileProvider
  providers_.emplace_back(new CLIProfileProvider());

  providers_.emplace_back(new ProfileProvider());

  // Check if ECS metadata access is disabled
  auto ecsMetadataDisabled =
      Darabonba::Env::getEnv("ALIBABA_CLOUD_ECS_METADATA_DISABLED");
  if (ecsMetadataDisabled != "true" && ecsMetadataDisabled != "True" &&
      ecsMetadataDisabled != "TRUE") {
    auto ecsMetaData = Darabonba::Env::getEnv("ALIBABA_CLOUD_ECS_METADATA");
    if (!ecsMetaData.empty()) {
      providers_.emplace_back(new EcsRamRoleProvider(ecsMetaData));
    }
  }

  auto url = Darabonba::Env::getEnv("ALIBABA_CLOUD_CREDENTIALS_URI");
  if (!url.empty()) {
    providers_.emplace_back(new URLProvider(url));
  }
}

} // namespace Credentials
} // namespace AlibabaCloud