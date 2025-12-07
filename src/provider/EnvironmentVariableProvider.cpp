#include <memory>

#include <darabonba/Env.hpp>

#include <alibabacloud/credential/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credential/provider/EnvironmentVariableProvider.hpp>
#include <alibabacloud/credential/provider/StsProvider.hpp>

namespace AlibabaCloud {
namespace Credential {

std::unique_ptr<Provider> EnvironmentVariableProvider::createProvider() {
  const auto accessKeyId =
      Darabonba::Env::getEnv("ALIBABA_CLOUD_ACCESS_KEY_ID");
  const auto accessKeySecret =
      Darabonba::Env::getEnv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
  if (!accessKeyId.empty() && !accessKeySecret.empty()) {
    const auto securityToken =
        Darabonba::Env::getEnv("ALIBABA_CLOUD_SECURITY_TOKEN");
    if (securityToken.empty()) {
      return std::unique_ptr<Provider>(
          new AccessKeyProvider(accessKeyId, accessKeySecret));
    }
    return std::unique_ptr<Provider>(
        new StsProvider(accessKeyId, accessKeySecret, securityToken));
  }
  if (accessKeyId.empty()) {
    throw Darabonba::Exception(
        "Environment variable accessKeyId cannot be empty");
  }
  if (accessKeySecret.empty()) {
    throw Darabonba::Exception(
        "Environment variable accessKeySecret cannot be empty");
  }
  return nullptr;
}
} // namespace Credential
} // namespace AlibabaCloud