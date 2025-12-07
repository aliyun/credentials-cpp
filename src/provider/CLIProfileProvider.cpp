#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credential/provider/CLIProfileProvider.hpp>
#include <alibabacloud/credential/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credential/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/RamRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/RsaKeyPairProvider.hpp>
#include <darabonba/Env.hpp>
#include <darabonba/Exception.hpp>
#include <darabonba/Ini.hpp>
#include <fstream>

// JSON support is optional
#ifdef HAS_NLOHMANN_JSON
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

namespace AlibabaCloud {
namespace Credential {

/**
 * @brief 获取 CLI 配置文件路径（跨平台支持）
 *
 * 优先级：
 * 1. 环境变量 ALIBABA_CLOUD_CLI_PROFILE_PATH
 * 2. ~/.aliyun/config.json (新路径)
 * 3. ~/.alibabaclouds.ini (旧路径，回退)
 */
std::string CLIProfileProvider::getCliProfilePath() {
  // 优先使用环境变量指定的路径
  std::string envPath =
      Darabonba::Env::getEnv("ALIBABA_CLOUD_CLI_PROFILE_PATH");
  if (!envPath.empty()) {
    return envPath;
  }

#ifdef _WIN32
  // Windows: 使用 USERPROFILE
  auto home = Darabonba::Env::getEnv("USERPROFILE");
  if (home.empty()) {
    // 回退到 HOMEDRIVE + HOMEPATH
    auto homeDrive = Darabonba::Env::getEnv("HOMEDRIVE");
    auto homePath = Darabonba::Env::getEnv("HOMEPATH");
    if (!homeDrive.empty() && !homePath.empty()) {
      home = homeDrive + homePath;
    }
  }
  const char sep = '\\';
  const std::string sepStr = "\\";
#else
  // Linux/macOS: 使用 HOME
  auto home = Darabonba::Env::getEnv("HOME");
  const char sep = '/';
  const std::string sepStr = "/";
#endif

  if (home.empty()) {
    return "";
  }

  // 确保 home 目录以分隔符结尾
  if (home.back() != sep && home.back() != '/' && home.back() != '\\') {
    home.push_back(sep);
  }

  // 优先使用新路径：~/.aliyun/config.json
  std::string newPath = home + ".aliyun" + sepStr + "config.json";
  std::ifstream testFile(newPath);
  bool newPathExists = testFile.good();
  testFile.close();

  if (newPathExists) {
    return newPath;
  }

  // 回退到旧路径：~/.alibabaclouds.ini
  return home + ".alibabaclouds.ini";
}

/**
 * @brief 检查文件是否为 JSON 格式
 */
static bool isJsonFile(const std::string &filePath) {
#ifdef HAS_NLOHMANN_JSON
  // 通过扩展名判断
  if (filePath.size() >= 5 && filePath.substr(filePath.size() - 5) == ".json") {
    return true;
  }

  // 通过文件内容判断
  std::ifstream ifs(filePath);
  if (!ifs.good()) {
    return false;
  }

  char firstChar;
  ifs >> firstChar;
  ifs.close();

  return firstChar == '{' || firstChar == '[';
#else
  // 没有 JSON 支持，始终返回 false
  return false;
#endif
}

/**
 * @brief 解析 JSON 格式的配置文件
 */
std::shared_ptr<Models::Config>
CLIProfileProvider::parseJsonProfile(const std::string &filePath,
                                     const std::string &profileName) {

#ifdef HAS_NLOHMANN_JSON
  std::ifstream ifs(filePath);
  if (!ifs.good()) {
    throw Darabonba::Exception("Can't open CLI profile file: " + filePath);
  }

  json profilesJson;
  try {
    ifs >> profilesJson;
  } catch (const json::exception &e) {
    throw Darabonba::Exception("Failed to parse JSON profile: " +
                               std::string(e.what()));
  }
  ifs.close();

  // 查找指定的 profile
  if (!profilesJson.contains("profiles")) {
    throw Darabonba::Exception("No 'profiles' section in CLI config file");
  }

  json profiles = profilesJson["profiles"];
  if (!profiles.is_array()) {
    throw Darabonba::Exception("'profiles' must be an array");
  }

  // 查找匹配的 profile
  json targetProfile;
  bool found = false;
  for (const auto &profile : profiles) {
    if (profile.contains("name") && profile["name"] == profileName) {
      targetProfile = profile;
      found = true;
      break;
    }
  }

  if (!found) {
    throw Darabonba::Exception("Profile '" + profileName +
                               "' not found in CLI config");
  }

  // 解析 profile 配置
  auto config = std::make_shared<Models::Config>();

  if (targetProfile.contains("mode")) {
    config->setType(targetProfile["mode"]);
  }

  if (targetProfile.contains("access_key_id")) {
    config->setAccessKeyId(targetProfile["access_key_id"]);
  }

  if (targetProfile.contains("access_key_secret")) {
    config->setAccessKeySecret(targetProfile["access_key_secret"]);
  }

  if (targetProfile.contains("sts_token")) {
    config->setSecurityToken(targetProfile["sts_token"]);
  }

  if (targetProfile.contains("ram_role_name")) {
    config->setRoleName(targetProfile["ram_role_name"]);
  }

  if (targetProfile.contains("ram_role_arn")) {
    config->setRoleArn(targetProfile["ram_role_arn"]);
  }

  if (targetProfile.contains("role_session_name")) {
    config->setRoleSessionName(targetProfile["role_session_name"]);
  }

  if (targetProfile.contains("public_key_id")) {
    config->setPublicKeyId(targetProfile["public_key_id"]);
  }

  if (targetProfile.contains("private_key_file")) {
    config->setPrivateKeyFile(targetProfile["private_key_file"]);
  }

  if (targetProfile.contains("oidc_provider_arn")) {
    config->setOidcProviderArn(targetProfile["oidc_provider_arn"]);
  }

  if (targetProfile.contains("oidc_token_file")) {
    config->setOidcTokenFilePath(targetProfile["oidc_token_file"]);
  }

  if (targetProfile.contains("region_id")) {
    config->setRegionId(targetProfile["region_id"]);
  }

  return config;
#else
  throw Darabonba::Exception("JSON format is not supported. Please install "
                             "nlohmann_json library or use INI format.");
#endif
}

/**
 * @brief 解析 INI 格式的配置文件
 */
std::shared_ptr<Models::Config>
CLIProfileProvider::parseIniProfile(const std::string &filePath,
                                    const std::string &profileName) {

  std::ifstream ifs(filePath);
  if (!ifs.good()) {
    throw Darabonba::Exception("Can't open CLI profile file: " + filePath);
  }

  auto iniObj = Darabonba::Ini::parse(ifs);
  ifs.close();

  // 检查 enable 选项
  const auto &enable = iniObj.get(profileName, Constant::INI_ENABLE);
  if (enable != "true") {
    throw Darabonba::Exception("The enable option in '" + profileName +
                               "' is not equal to true.");
  }

  // 获取 section
  const auto &section = iniObj.get(profileName);
  auto config = std::make_shared<Models::Config>();

  if (section.count(Constant::INI_TYPE)) {
    config->setType(section.get(Constant::INI_TYPE));
  }

  if (section.count(Constant::INI_ACCESS_KEY_ID)) {
    config->setAccessKeyId(section.get(Constant::INI_ACCESS_KEY_ID));
  }

  if (section.count(Constant::INI_ACCESS_KEY_SECRET)) {
    config->setAccessKeySecret(section.get(Constant::INI_ACCESS_KEY_SECRET));
  }

  if (section.count(Constant::INI_ROLE_NAME)) {
    config->setRoleName(section.get(Constant::INI_ROLE_NAME));
  }

  if (section.count(Constant::INI_ROLE_ARN)) {
    config->setRoleArn(section.get(Constant::INI_ROLE_ARN));
  }

  if (section.count(Constant::INI_ROLE_SESSION_NAME)) {
    config->setRoleSessionName(section.get(Constant::INI_ROLE_SESSION_NAME));
  }

  if (section.count(Constant::INI_PUBLIC_KEY_ID)) {
    config->setPublicKeyId(section.get(Constant::INI_PUBLIC_KEY_ID));
  }

  if (section.count(Constant::INI_PRIVATE_KEY_FILE)) {
    config->setPrivateKeyFile(section.get(Constant::INI_PRIVATE_KEY_FILE));
  }

  if (section.count(Constant::INI_OIDC_PROVIDER_ARN)) {
    config->setOidcProviderArn(section.get(Constant::INI_OIDC_PROVIDER_ARN));
  }

  if (section.count(Constant::INI_OIDC_TOKEN_FILE_PATH)) {
    config->setOidcTokenFilePath(
        section.get(Constant::INI_OIDC_TOKEN_FILE_PATH));
  }

  if (section.count(Constant::INI_POLICY)) {
    config->setPolicy(section.get(Constant::INI_POLICY));
  }

  return config;
}

// 构造函数实现
CLIProfileProvider::CLIProfileProvider()
    : profileName_("default"), provider_(nullptr) {
  // 检查是否禁用 CLI Profile
  std::string disabled =
      Darabonba::Env::getEnv(Constant::ENV_CLI_PROFILE_DISABLED);
  if (disabled == "true" || disabled == "TRUE" || disabled == "1") {
    throw Darabonba::Exception("CLI Profile is disabled by environment "
                               "variable ALIBABA_CLOUD_CLI_PROFILE_DISABLED");
  }
}

CLIProfileProvider::CLIProfileProvider(const std::string &profileName)
    : profileName_(profileName), provider_(nullptr) {
  // 检查是否禁用 CLI Profile
  std::string disabled =
      Darabonba::Env::getEnv(Constant::ENV_CLI_PROFILE_DISABLED);
  if (disabled == "true" || disabled == "TRUE" || disabled == "1") {
    throw Darabonba::Exception("CLI Profile is disabled by environment "
                               "variable ALIBABA_CLOUD_CLI_PROFILE_DISABLED");
  }
}

// getCredential 实现
Models::CredentialModel &CLIProfileProvider::getCredential() {
  provider_ = createProvider();
  if (provider_ == nullptr) {
    throw Darabonba::Exception("Can't create provider from CLI profile.");
  }
  return provider_->getCredential();
}

const Models::CredentialModel &CLIProfileProvider::getCredential() const {
  provider_ = createProvider();
  if (provider_ == nullptr) {
    throw Darabonba::Exception("Can't create provider from CLI profile.");
  }
  return provider_->getCredential();
}

/**
 * @brief Get provider name
 */
std::string CLIProfileProvider::getProviderName() const {
  provider_ = createProvider();
  if (provider_ == nullptr) {
    throw Darabonba::Exception("Can't create provider from CLI profile.");
  }
  return provider_->getProviderName();
}

/**
 * @brief 创建凭据提供者
 */
std::unique_ptr<Provider> CLIProfileProvider::createProvider() const {
  // 获取配置文件路径
  std::string filePath = getCliProfilePath();
  if (filePath.empty()) {
    throw Darabonba::Exception("No CLI profile file found.");
  }

  // 解析配置文件
  std::shared_ptr<Models::Config> config;
  if (isJsonFile(filePath)) {
    config = parseJsonProfile(filePath, profileName_);
  } else {
    config = parseIniProfile(filePath, profileName_);
  }

  // 根据类型创建对应的 Provider
  auto configType = config->type();
  if (configType.empty()) {
    throw Darabonba::Exception("The configured client type is empty");
  }

  if (configType == Constant::ECS_RAM_ROLE) {
    return std::unique_ptr<Provider>(new EcsRamRoleProvider(config));
  } else if (configType == Constant::RSA_KEY_PAIR) {
    return std::unique_ptr<Provider>(new RsaKeyPairProvider(config));
  } else if (configType == Constant::RAM_ROLE_ARN) {
    return std::unique_ptr<Provider>(new RamRoleArnProvider(config));
  } else if (configType == Constant::OIDC_ROLE_ARN) {
    return std::unique_ptr<Provider>(new OIDCRoleArnProvider(config));
  }

  // 默认使用 AccessKey
  const auto &accessKeyId = config->accessKeyId();
  const auto &accessKeySecret = config->accessKeySecret();
  if (accessKeyId.empty() || accessKeySecret.empty()) {
    throw Darabonba::Exception("AccessKeyId and AccessKeySecret are required");
  }

  return std::unique_ptr<Provider>(new AccessKeyProvider(config));
}

} // namespace Credential
} // namespace AlibabaCloud
