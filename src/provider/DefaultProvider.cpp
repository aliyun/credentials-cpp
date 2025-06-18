#include <alibabacloud/credential/provider/DefaultProvider.hpp>
#include <alibabacloud/credential/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credential/provider/EnvironmentVariableProvider.hpp>
#include <alibabacloud/credential/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/ProfileProvider.hpp>
#include <alibabacloud/credential/provider/URLProvider.hpp>
#include <darabonba/Env.hpp>
#include <darabonba/Exception.hpp>
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