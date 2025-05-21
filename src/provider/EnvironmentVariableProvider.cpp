#include <alibabacloud/provider/AccessKeyProvider.hpp>
#include <alibabacloud/provider/EnvironmentVariableProvider.hpp>
#include <alibabacloud/provider/StsProvider.hpp>
#include <darabonba/Env.hpp>
#include <memory>

namespace AlibabaCloud {
namespace Credential {

std::unique_ptr<Provider> EnvironmentVariableProvider::createProvider() {
  auto accessKeyId = Darabonba::Env::getEnv("ALIBABA_CLOUD_ACCESS_KEY_ID");
  auto accessKeySecret =
      Darabonba::Env::getEnv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
  if (!accessKeyId.empty() && !accessKeySecret.empty()) {
    auto securityToken = Darabonba::Env::getEnv("ALIBABA_CLOUD_SECURITY_TOKEN");
    if (securityToken.empty()) {
      return std::unique_ptr<Provider>(
          new AccessKeyProvider(accessKeyId, accessKeySecret));
    } else {
      return std::unique_ptr<Provider>(new StsProvider(accessKeyId, accessKeySecret, securityToken));
    }
  }
  return nullptr;
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