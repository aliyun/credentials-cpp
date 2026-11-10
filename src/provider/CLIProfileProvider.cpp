#include <alibabacloud/credentials/AuthUtil.hpp>
#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/Model.hpp>
#include <alibabacloud/credentials/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credentials/provider/CLIProfileProvider.hpp>
#include <alibabacloud/credentials/provider/CloudSSOCredentialsProvider.hpp>
#include <alibabacloud/credentials/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credentials/provider/OAuthCredentialsProvider.hpp>
#include <alibabacloud/credentials/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credentials/provider/RamRoleArnProvider.hpp>
#include <alibabacloud/credentials/provider/RsaKeyPairProvider.hpp>
#include <alibabacloud/credentials/provider/StsProvider.hpp>
#include <darabonba/Env.hpp>
#include <alibabacloud/credentials/Exception.hpp>
#include <darabonba/Ini.hpp>
#include <darabonba/Core.hpp>
#include <darabonba/Exception.hpp>
#include <darabonba/encode/SHA256.hpp>
#include <cstdlib>
#include <regex>
#include <fstream>

// JSON support is optional
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace AlibabaCloud {
namespace Credentials {

/**
 * @brief Map Alibaba Cloud CLI profile mode to internal credential type.
 *
 * CLI config.json uses modes such as "AK" / "RamRoleArn" / "EcsRamRole" / "OIDC",
 * while createProvider() compares Constant::* values ("access_key", "ram_role_arn", …).
 * Aligns with Java CLIProfileCredentialsProvider mode handling.
 */
static std::string mapCliModeToType(const std::string &mode) {
  if (mode.empty()) {
    return mode;
  }
  if (mode == "AK" || mode == Constant::ACCESS_KEY) {
    return Constant::ACCESS_KEY;
  }
  if (mode == "StsToken" || mode == Constant::STS) {
    return Constant::STS;
  }
  if (mode == "RamRoleArn" || mode == Constant::RAM_ROLE_ARN) {
    return Constant::RAM_ROLE_ARN;
  }
  if (mode == "EcsRamRole" || mode == Constant::ECS_RAM_ROLE) {
    return Constant::ECS_RAM_ROLE;
  }
  if (mode == "OIDC" || mode == Constant::OIDC_ROLE_ARN) {
    return Constant::OIDC_ROLE_ARN;
  }
  if (mode == "RsaKeyPair" || mode == Constant::RSA_KEY_PAIR) {
    return Constant::RSA_KEY_PAIR;
  }
  if (mode == "CloudSSO" || mode == Constant::CLOUD_SSO) {
    return Constant::CLOUD_SSO;
  }
  if (mode == "OAuth" || mode == Constant::OAUTH) {
    return Constant::OAUTH;
  }
  throw CredentialException("Unsupported profile mode '" + mode +
                            "' form CLI credentials file.");
}

/**
 * @brief 获取 CLI 配置文件路径（跨平台支持）
 *
 * 优先级：
 * 1. 环境变量 ALIBABA_CLOUD_CLI_PROFILE_PATH
 * 2. ~/.aliyun/config.json (新路径)
 * 3. ~/.alibabaclouds.ini (旧路径，回退)
 */
static std::string cliHomeDirectory() {
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
#else
  // Linux/macOS: 使用 HOME
  auto home = Darabonba::Env::getEnv("HOME");
  const char sep = '/';
#endif

  if (home.empty()) {
    return "";
  }

  // 确保 home 目录以分隔符结尾
  if (home.back() != sep && home.back() != '/' && home.back() != '\\') {
    home.push_back(sep);
  }

  return home;
}

std::string CLIProfileProvider::getCliProfilePath() {
  auto envPath = Darabonba::Env::getEnv("ALIBABA_CLOUD_CLI_PROFILE_PATH");
  if (!envPath.empty()) {
    return envPath;
  }

  auto home = cliHomeDirectory();
  if (home.empty()) {
    return "";
  }
  auto newPath = home + ".aliyun/config.json";
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
}

namespace {

std::string canonicalConfigPath(const std::string &path) {
  if (path.empty()) {
    return "";
  }
#ifdef _WIN32
  HANDLE file = CreateFileA(path.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    return "";
  }
  std::string resolved(32768, '\0');
  DWORD size = GetFinalPathNameByHandleA(file, &resolved[0], static_cast<DWORD>(resolved.size()), 0);
  CloseHandle(file);
  if (size == 0 || size >= resolved.size()) {
    return "";
  }
  resolved.resize(size);
  if (resolved.compare(0, 8, R"(\\?\UNC\)") == 0) {
    return R"(\\)" + resolved.substr(8);
  }
  return resolved.compare(0, 4, R"(\\?\)") == 0 ? resolved.substr(4) : resolved;
#else
  std::unique_ptr<char, decltype(&std::free)> resolved(realpath(path.c_str(), nullptr), &std::free);
  return resolved ? resolved.get() : "";
#endif
}

std::string sharedSessionPath(const std::string &filePath, const std::string &name) {
  static const std::regex validName("[a-z0-9_-]{1,64}");
  if (!std::regex_match(name, validName)) {
    throw CredentialException(std::string("Invalid CloudSSO session name."));
  }
  auto config = canonicalConfigPath(filePath);
  if (config.empty()) {
    throw CredentialException(std::string("Unable to resolve CloudSSO profile file."));
  }
  auto home = cliHomeDirectory();
  auto defaults = home.empty() ? "" : canonicalConfigPath(home + ".aliyun/config.json");
#ifdef _WIN32
  const auto separator = config.find_last_of("/\\");
#else
  const auto separator = config.find_last_of('/');
#endif
  std::string directory = config.substr(0, separator) + "/sso";
  if (config != defaults) {
    const auto hash = Darabonba::Encode::SHA256::hash(config.data(), config.size());
    const char digits[] = "0123456789abcdef";
    directory += '/';
    for (auto byte : hash) {
      directory += digits[byte >> 4];
      directory += digits[byte & 15];
    }
  }
  return directory + '/' + name + ".json";
}

class SharedCloudSSOProvider final : public NeedFreshProvider {
public:
  SharedCloudSSOProvider(const std::string &filePath, const std::string &name,
                         const std::string &account, const std::string &accessConfig)
      : path_(sharedSessionPath(filePath, name)), name_(name), account_(account),
        accessConfig_(accessConfig) {
    if (account_.empty() || accessConfig_.empty()) {
      throw CredentialException(std::string("CloudSSO account ID and access configuration are required."));
    }
  }

  std::string getProviderName() const override { return Constant::CLOUD_SSO; }

  bool matches(const SharedCloudSSOProvider &other) const {
    return path_ == other.path_ && account_ == other.account_ && accessConfig_ == other.accessConfig_;
  }

private:
  bool refreshCredential() const override {
    std::ifstream file(path_);
    if (!file) {
      throw CredentialException("Unable to read CloudSSO session " + name_);
    }
    auto session = json::parse(file, nullptr, false);
    try {
      if (!session.is_object() || session.value("name", "") != name_ ||
          session.value("sign_in_url", "").empty()) {
        throw CredentialException("Invalid CloudSSO session " + name_);
      }
      auto token = session.value("access_token", "");
      auto expiresAt = session.value("expires_at", int64_t{0});
      if (token.empty() || expiresAt <= time(nullptr)) {
        throw CredentialException(std::string("CloudSSO access token is empty or expired, please re-login with CLI."));
      }

      auto request = AuthUtil::getNewRequest(session.at("sign_in_url").get<std::string>());
      request.setPathname("/cloud-credentials");
      request.getQuery().clear();
      request.setMethod("POST");
      request.addHeader("Accept", "application/json");
      request.addHeader("Content-Type", "application/json");
      request.addHeader("Authorization", "Bearer " + token);
      json body = {{"AccountId", account_}, {"AccessConfigurationId", accessConfig_}};
      request.setBody(body.dump());
      Darabonba::RuntimeOptions runtime;
      runtime.setConnectTimeout(5000).setReadTimeout(10000);
      auto response = Darabonba::Core::doAction(request, runtime).get();
      if (response->getStatusCode() != 200) {
        throw CredentialException(std::string("CloudSSO credential exchange failed."));
      }
      const auto result = Darabonba::Stream::readAsJSON(response->getBody()).at("CloudCredential");
      auto keyId = result.value("AccessKeyId", "");
      auto secret = result.value("AccessKeySecret", "");
      auto securityToken = result.value("SecurityToken", "");
      auto expiration = strtotime(result.value("Expiration", ""));
      if (keyId.empty() || secret.empty() || securityToken.empty() || expiration <= time(nullptr)) {
        throw CredentialException(std::string("Invalid CloudSSO credential response."));
      }
      Models::CredentialModel credential;
      credential.setAccessKeyId(keyId).setAccessKeySecret(secret).setSecurityToken(securityToken)
          .setType(Constant::CLOUD_SSO);
      credential_ = std::move(credential);
      expiration_ = expiration;
      return true;
    } catch (const json::exception &) {
      throw CredentialException(std::string("Invalid CloudSSO session or credential response."));
    } catch (const Darabonba::DaraException &) {
      throw CredentialException(std::string("CloudSSO credential exchange failed."));
    }
  }

  const std::string path_;
  const std::string name_;
  const std::string account_;
  const std::string accessConfig_;
};

} // namespace

/**
 * @brief 解析 JSON 格式的配置文件
 */
std::shared_ptr<Models::Config>
CLIProfileProvider::parseJsonProfile(const std::string &filePath,
                                     const std::string &profileName) {
  return parseJsonProfile(filePath, profileName, nullptr);
}

std::shared_ptr<Models::Config> CLIProfileProvider::parseJsonProfile(
    const std::string &filePath, const std::string &profileName,
    std::unique_ptr<Provider> *sharedProvider) {
  std::ifstream ifs(filePath);
  if (!ifs.good()) {
    throw CredentialException("Can't open CLI profile file: " + filePath);
  }

  json profilesJson;
  try {
    ifs >> profilesJson;
  } catch (const json::exception &e) {
    throw CredentialException("Failed to parse JSON profile: " +
                               std::string(e.what()));
  }
  ifs.close();

  // 查找指定的 profile
  if (!profilesJson.contains("profiles")) {
    throw CredentialException(std::string("No 'profiles' section in CLI config file"));
  }

  json profiles = profilesJson["profiles"];
  if (!profiles.is_array()) {
    throw CredentialException(std::string("'profiles' must be an array"));
  }

  // 查找匹配的 profile
  auto selected = profileName.empty() ? std::string("default") : profileName;
  if (profileName.empty()) {
    auto active = Darabonba::Env::getEnv("ALIBABA_CLOUD_PROFILE");
    if (active.empty() && profilesJson.contains("current") && profilesJson["current"].is_string()) {
      active = profilesJson["current"].get<std::string>();
    }
    for (const auto &profile : profiles) {
      if (profile.contains("name") && profile["name"] == active &&
          profile.value("mode", "") == "CloudSSO" && profile.contains("sso_session") &&
          !profile["sso_session"].is_null() && profile["sso_session"] != "") {
        selected = profile["name"].get<std::string>();
        break;
      }
    }
  }
  json targetProfile;
  bool found = false;
  for (const auto &profile : profiles) {
    if (profile.contains("name") && profile["name"] == selected) {
      targetProfile = profile;
      found = true;
      break;
    }
  }

  if (!found) {
    throw CredentialException("Profile '" + profileName +
                               "' not found in CLI config");
  }

  // 解析 profile 配置
  auto config = std::make_shared<Models::Config>();

  if (targetProfile.contains("mode")) {
    // CLI mode (e.g. RamRoleArn) must be mapped to internal type (ram_role_arn)
    std::string mode = targetProfile["mode"].get<std::string>();
    config->setType(mapCliModeToType(mode));
  }

  if (sharedProvider && config->getType() == Constant::CLOUD_SSO && targetProfile.contains("sso_session")
      && !targetProfile["sso_session"].is_null()) {
    if (!targetProfile["sso_session"].is_string()) {
      throw CredentialException(std::string("Invalid CloudSSO session name."));
    }
    auto name = targetProfile["sso_session"].get<std::string>();
    if (!name.empty()) {
      sharedProvider->reset(new SharedCloudSSOProvider(filePath, name,
          targetProfile.value("cloud_sso_account_id", ""), targetProfile.value("cloud_sso_access_config", "")));
      return config;
    }
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

  // CLI uses ram_session_name; also accept role_session_name
  if (targetProfile.contains("ram_session_name")) {
    config->setRoleSessionName(targetProfile["ram_session_name"]);
  } else if (targetProfile.contains("role_session_name")) {
    config->setRoleSessionName(targetProfile["role_session_name"]);
  }

  if (targetProfile.contains("expired_seconds")) {
    config->setDurationSeconds(targetProfile["expired_seconds"].get<int64_t>());
    config->setRoleSessionExpiration(
        targetProfile["expired_seconds"].get<int64_t>());
  }

  if (targetProfile.contains("sts_region")) {
    config->setStsRegionId(targetProfile["sts_region"]);
    config->setRegionId(targetProfile["sts_region"]);
  }

  if (targetProfile.contains("policy")) {
    config->setPolicy(targetProfile["policy"]);
  }

  if (targetProfile.contains("external_id")) {
    config->setExternalId(targetProfile["external_id"]);
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
}

/**
 * @brief 解析 INI 格式的配置文件
 */
std::shared_ptr<Models::Config>
CLIProfileProvider::parseIniProfile(const std::string &filePath,
                                    const std::string &profileName) {

  std::ifstream ifs(filePath);
  if (!ifs.good()) {
    throw CredentialException("Can't open CLI profile file: " + filePath);
  }

  auto iniObj = Darabonba::Ini::parse(ifs);
  ifs.close();

  // 检查 enable 选项
  const auto &enable = iniObj.get(profileName, Constant::INI_ENABLE);
  if (enable != "true") {
    throw CredentialException("The enable option in '" + profileName +
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
    : profileName_(), provider_(nullptr) {
  // 检查是否禁用 CLI Profile
  std::string disabled =
      Darabonba::Env::getEnv(Constant::ENV_CLI_PROFILE_DISABLED);
  if (disabled == "true" || disabled == "TRUE" || disabled == "1") {
    throw CredentialException(std::string("CLI Profile is disabled by environment "
                               "variable ALIBABA_CLOUD_CLI_PROFILE_DISABLED"));
  }
}

CLIProfileProvider::CLIProfileProvider(const std::string &profileName)
    : profileName_(profileName), provider_(nullptr) {
  // 检查是否禁用 CLI Profile
  std::string disabled =
      Darabonba::Env::getEnv(Constant::ENV_CLI_PROFILE_DISABLED);
  if (disabled == "true" || disabled == "TRUE" || disabled == "1") {
    throw CredentialException(std::string("CLI Profile is disabled by environment "
                               "variable ALIBABA_CLOUD_CLI_PROFILE_DISABLED"));
  }
}

// getCredential 实现
Models::CredentialModel CLIProfileProvider::getCredential() const {
  auto provider = createProvider();
  if (provider == nullptr) {
    throw CredentialException(std::string("Can't create provider from CLI profile."));
  }
  auto shared = dynamic_cast<SharedCloudSSOProvider *>(provider.get());
  if (shared) {
    std::lock_guard<std::mutex> lock(sharedProviderMutex_);
    auto cached = dynamic_cast<SharedCloudSSOProvider *>(provider_.get());
    if (!cached || !cached->matches(*shared)) {
      provider_ = std::move(provider);
    }
    return provider_->getCredential();
  }
  return provider->getCredential();
}

/**
 * @brief Get provider name
 */
std::string CLIProfileProvider::getProviderName() const {
  auto provider = createProvider();
  if (provider == nullptr) {
    throw CredentialException(std::string("Can't create provider from CLI profile."));
  }
  return provider->getProviderName();
}

/**
 * @brief 创建凭据提供者
 */
std::unique_ptr<Provider> CLIProfileProvider::createProvider() const {
  // 获取配置文件路径
  std::string filePath = getCliProfilePath();
  if (filePath.empty()) {
    throw CredentialException(std::string("No CLI profile file found."));
  }

  // 解析配置文件
  std::shared_ptr<Models::Config> config;
  if (isJsonFile(filePath)) {
    std::unique_ptr<Provider> sharedProvider;
    config = parseJsonProfile(filePath, profileName_, &sharedProvider);
    if (sharedProvider) {
      return sharedProvider;
    }
  } else {
    config = parseIniProfile(filePath, profileName_.empty() ? "default" : profileName_);
  }

  // 根据类型创建对应的 Provider
  auto configType = config->getType();
  if (configType.empty()) {
    throw CredentialException(std::string("The configured client type is empty"));
  }
  if (configType == Constant::ACCESS_KEY) {
    const auto &accessKeyId = config->getAccessKeyId();
    const auto &accessKeySecret = config->getAccessKeySecret();
    if (accessKeyId.empty() || accessKeySecret.empty()) {
      throw CredentialException(
          std::string("AccessKeyId and AccessKeySecret are required"));
    }
    return std::unique_ptr<Provider>(new AccessKeyProvider(config));
  }
  if (configType == Constant::STS) {
    return std::unique_ptr<Provider>(new StsProvider(config));
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
  if (configType == Constant::CLOUD_SSO) {
    return std::unique_ptr<Provider>(new CloudSSOCredentialsProvider(config));
  }
  if (configType == Constant::OAUTH) {
    return std::unique_ptr<Provider>(new OAuthCredentialsProvider(config));
  }

  throw CredentialException("Unsupported profile type '" + configType +
                            "' form CLI credentials file.");
}

} // namespace Credentials
} // namespace AlibabaCloud
