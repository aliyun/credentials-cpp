#include <fstream>

#include <darabonba/Env.hpp>
#include <darabonba/Exception.hpp>
#include <darabonba/Ini.hpp>

#include <alibabacloud/credential/AuthUtil.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credential/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credential/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/ProfileProvider.hpp>
#include <alibabacloud/credential/provider/RamRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/RsaKeyPairProvider.hpp>

static std::string getProfilePath() {
#ifdef _WIN32
  auto home = Darabonba::Env::getEnv("USERPROFILE");
  char sep = '\\';
#else
  auto home = Darabonba::Env::getEnv("HOME");
  constexpr char sep = '/';
#endif
  if (home.empty())
    return home;
  if (home.back() != sep) {
    home.push_back(sep);
  }
  // Support both .aliyun/config.json (new) and .alibabaclouds.ini (legacy)
  std::string newPath = home + ".aliyun" + sep + "config.json";
  std::ifstream testFile(newPath);
  if (testFile.good()) {
    return newPath;
  }
  return home + ".alibabaclouds.ini";
}

namespace AlibabaCloud {
namespace Credential {

std::unique_ptr<Provider> ProfileProvider::createProvider() {
  auto filePath = Darabonba::Env::getEnv("ALIBABA_CLOUD_CREDENTIALS_FILE",
                                         getProfilePath());
  if (filePath.empty()) {
    throw Darabonba::Exception("No credential profile.");
  }
  std::ifstream ifs(filePath);
  if (!ifs.good()) {
    throw Darabonba::Exception("Can't open credential profile: " + filePath);
  }
  auto iniObj = Darabonba::Ini::parse(ifs);
  // parse ini object into config
  std::string sectionName = AuthUtil::clientType();
  if (sectionName.empty()) {
    sectionName = Darabonba::Env::getEnv("ALIBABA_CLOUD_PROFILE", "default");
  }

  const auto &enable = iniObj.get(sectionName, Constant::INI_ENABLE);
  if (enable != "true") {
    throw Darabonba::Exception("The enable option in " + sectionName +
                               " is not equal to true.");
  }
  const auto &section = iniObj.get(sectionName);
  auto config = std::make_shared<Models::Config>();
  if (section.count(Constant::INI_ACCESS_KEY_ID)) {
    config->setAccessKeyId(section.get(Constant::INI_ACCESS_KEY_ID));
  }
  if (section.count(Constant::INI_ACCESS_KEY_SECRET)) {
    config->setAccessKeySecret(section.get(Constant::INI_ACCESS_KEY_SECRET));
  }
  if (section.count(Constant::INI_TYPE)) {
    config->setType(section.get(Constant::INI_TYPE));
  }
  if (section.count(Constant::INI_PRIVATE_KEY_FILE)) {
    config->setPrivateKeyFile(section.get(Constant::INI_PRIVATE_KEY_FILE));
  }
  if (section.count(Constant::INI_PUBLIC_KEY_ID)) {
    config->setPublicKeyId(section.get(Constant::INI_PUBLIC_KEY_ID));
  }
  if (section.count(Constant::INI_ROLE_ARN)) {
    config->setRoleArn(section.get(Constant::INI_ROLE_ARN));
  }
  if (section.count(Constant::INI_ROLE_SESSION_NAME)) {
    config->setRoleSessionName(section.get(Constant::INI_ROLE_SESSION_NAME));
  }
  if (section.count(Constant::INI_POLICY)) {
    config->setPolicy(section.get(Constant::INI_POLICY));
  }
  if (section.count(Constant::INI_ROLE_NAME)) {
    config->setRoleName(section.get(Constant::INI_ROLE_NAME));
  }
  if (section.count(Constant::INI_OIDC_PROVIDER_ARN)) {
    config->setOidcProviderArn(section.get(Constant::INI_OIDC_PROVIDER_ARN));
  }
  if (section.count(Constant::INI_OIDC_TOKEN_FILE_PATH)) {
    config->setOidcTokenFilePath(
        section.get(Constant::INI_OIDC_TOKEN_FILE_PATH));
  }

  auto configType = config->type();
  if (configType.empty()) {
    throw Darabonba::Exception("The configured client type is empty");
  }
  if (configType == Constant::ECS_RAM_ROLE) {
    return std::unique_ptr<Provider>(new EcsRamRoleProvider(config));
  }
  if (configType == Constant::RSA_KEY_PAIR) {
    return std::unique_ptr<Provider>(new RsaKeyPairProvider(config));
  }
  if (configType == Constant::RAM_ROLE_ARN) {
    return std::unique_ptr<Provider>(new RamRoleArnProvider(config));
  }
  if (configType == Constant::OIDC_ROLE_ARN) {
    return std::unique_ptr<Provider>(new OIDCRoleArnProvider(config));
  }
  const auto &accessKeyId = config->accessKeyId(),
             &accessKeySecret = config->accessKeySecret();
  if (accessKeyId.empty() || accessKeySecret.empty()) {
    return nullptr;
  }
  return std::unique_ptr<Provider>(new AccessKeyProvider(config));
}

} // namespace Credential
} // namespace AlibabaCloud