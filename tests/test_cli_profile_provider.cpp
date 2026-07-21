#include <gtest/gtest.h>
#include <alibabacloud/credentials/provider/CLIProfileProvider.hpp>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
static inline int setenv(const char* name, const char* value, int /*overwrite*/) {
  return _putenv_s(name, value);
}
static inline int unsetenv(const char* name) {
  return _putenv_s(name, "");
}
#endif

#include <alibabacloud/credentials/Constant.hpp>
#include <fstream>

using namespace AlibabaCloud::Credentials;

// ==================== CLIProfileProvider Tests ====================

class CLIProfileProviderTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Save original environment variables
    saveEnv("ALIBABA_CLOUD_CLI_PROFILE_PATH");
    saveEnv("ALIBABA_CLOUD_PROFILE");
    saveEnv("HOME");
    saveEnv("USERPROFILE");
  }
  
  void TearDown() override {
    // Restore environment variables
    restoreEnv("ALIBABA_CLOUD_CLI_PROFILE_PATH");
    restoreEnv("ALIBABA_CLOUD_PROFILE");
    restoreEnv("HOME");
    restoreEnv("USERPROFILE");
  }
  
  void saveEnv(const std::string& name) {
    const char* value = std::getenv(name.c_str());
    if (value) {
      savedEnv_[name] = value;
    }
  }
  
  void restoreEnv(const std::string& name) {
    if (savedEnv_.find(name) != savedEnv_.end()) {
      setenv(name.c_str(), savedEnv_[name].c_str(), 1);
    } else {
      unsetenv(name.c_str());
    }
  }
  
  std::map<std::string, std::string> savedEnv_;
};

// Remove unused static functions - they are not used in any tests

TEST_F(CLIProfileProviderTest, DefaultConstructor) {
  // Should use default profile name "default"
  EXPECT_NO_THROW({
    CLIProfileProvider provider;
  });
}

TEST_F(CLIProfileProviderTest, ConstructorWithProfileName) {
  EXPECT_NO_THROW({
    CLIProfileProvider provider("production");
  });
}

TEST_F(CLIProfileProviderTest, ConstructorWithEmptyProfileName) {
  EXPECT_NO_THROW({
    CLIProfileProvider provider("");
  });
}

TEST_F(CLIProfileProviderTest, ConstructorWithSpecialProfileName) {
  EXPECT_NO_THROW({
    CLIProfileProvider provider("test-profile_123");
  });
}

TEST_F(CLIProfileProviderTest, GetCredentialWithoutValidConfig) {
  // Set to a non-existent path to ensure no config file is found
  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", "/tmp/nonexistent_cli_config_12345.json", 1);

  CLIProfileProvider provider;

  EXPECT_THROW({
    provider.getCredential();
  }, CredentialException);
}

TEST_F(CLIProfileProviderTest, MultipleProfileInstances) {
  EXPECT_NO_THROW({
    CLIProfileProvider provider1;
    CLIProfileProvider provider2("default");
    CLIProfileProvider provider3("production");
  });
}

TEST_F(CLIProfileProviderTest, ConstGetCredential) {
  // Set to a non-existent path to ensure no config file is found
  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", "/tmp/nonexistent_cli_config_67890.json", 1);

  const CLIProfileProvider provider;

  EXPECT_THROW({
    provider.getCredential();
  }, CredentialException);
}

TEST_F(CLIProfileProviderTest, ProfileNamePreserved) {
  // Create provider with specific profile name
  // The profile name should be used when reading config
  CLIProfileProvider provider("test_profile");
  
  // Even though it will fail, it should try to use the correct profile
  EXPECT_THROW({
    provider.getCredential();
  }, CredentialException);
}

TEST_F(CLIProfileProviderTest, UsesEnvironmentVariableForPath) {
  // Set custom CLI profile path
  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", "/tmp/nonexistent_cli_config.json", 1);
  
  CLIProfileProvider provider;
  
  // Should try to read from the specified path
  EXPECT_THROW({
    provider.getCredential();
  }, CredentialException);
}

TEST_F(CLIProfileProviderTest, SupportsMultipleGetCredentialCalls) {
  // Set to a non-existent path to ensure no config file is found
  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", "/tmp/nonexistent_cli_config_multi.json", 1);

  CLIProfileProvider provider;

  // Multiple calls should behave consistently
  EXPECT_THROW({
    provider.getCredential();
  }, CredentialException);

  EXPECT_THROW({
    provider.getCredential();
  }, CredentialException);
}

// 获取跨平台的临时目录路径
static std::string getTempDir() {
#if defined(_WIN32) || defined(_WIN64)
  const char* temp = std::getenv("TEMP");
  if (!temp) temp = std::getenv("TMP");
  if (!temp) temp = "C:\\Windows\\Temp";
  return std::string(temp);
#else
  const char* tmpdir = std::getenv("TMPDIR");
  if (tmpdir) return std::string(tmpdir);
  return "/tmp";
#endif
}

// Escape a string for embedding inside a JSON string value (Windows paths
// contain backslashes that must be written as \\ in JSON).
static std::string escapeJsonString(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"':  out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:   out += c; break;
    }
  }
  return out;
}

TEST(CLIProfileProviderHelpers, EscapeJsonStringEscapesWindowsPath) {
  EXPECT_EQ(escapeJsonString("C:\\Users\\runner\\Temp\\token.txt"),
            "C:\\\\Users\\\\runner\\\\Temp\\\\token.txt");
  EXPECT_EQ(escapeJsonString("path\"with\"quotes"), "path\\\"with\\\"quotes");
}

// mode 字段缺失 -> type 为空 -> createProvider() 抛出 "The configured client type is empty"
TEST_F(CLIProfileProviderTest, EmptyModeThrowsCredentialException) {
  std::string tmpPath = getTempDir() + "/test_cli_empty_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"default\",\"profiles\":["
      << "{\"name\":\"default\",\"access_key_id\":\"test_id\",\"access_key_secret\":\"test_secret\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider;

  try {
    provider.getCredential();
    FAIL() << "Expected CredentialException";
  } catch (const CredentialException &e) {
    EXPECT_NE(std::string(e.what()).find("type"), std::string::npos)
        << "Expected error about empty type, got: " << e.what();
  }

  std::remove(tmpPath.c_str());
}

// mode=AK 且凭证齐全 -> 成功创建 AccessKeyProvider
TEST_F(CLIProfileProviderTest, AkModeCreatesAccessKeyProvider) {
  std::string tmpPath = getTempDir() + "/test_cli_ak_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"default\",\"profiles\":["
      << "{\"name\":\"default\",\"mode\":\"AK\","
      << "\"access_key_id\":\"test_ak_id\",\"access_key_secret\":\"test_ak_secret\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider;

  EXPECT_NO_THROW({
    EXPECT_EQ(Constant::ACCESS_KEY, provider.getProviderName());
  });

  std::remove(tmpPath.c_str());
}

// mode=RamRoleArn 须映射到 ram_role_arn，不能回落到 AccessKey
TEST_F(CLIProfileProviderTest, RamRoleArnModeCreatesRamRoleArnProvider) {
  std::string tmpPath = getTempDir() + "/test_cli_ram_role_arn_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"RamRoleArn\",\"profiles\":["
      << "{\"name\":\"RamRoleArn\",\"mode\":\"RamRoleArn\","
      << "\"access_key_id\":\"akid\",\"access_key_secret\":\"secret\","
      << "\"ram_role_arn\":\"acs:ram::123:role/test\","
      << "\"ram_session_name\":\"session\",\"expired_seconds\":3600,"
      << "\"sts_region\":\"cn-hangzhou\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider("RamRoleArn");

  EXPECT_EQ(Constant::RAM_ROLE_ARN, provider.getProviderName());

  std::remove(tmpPath.c_str());
}

// mode=EcsRamRole 须映射到 ecs_ram_role
TEST_F(CLIProfileProviderTest, EcsRamRoleModeCreatesEcsRamRoleProvider) {
  std::string tmpPath = getTempDir() + "/test_cli_ecs_ram_role_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"EcsRamRole\",\"profiles\":["
      << "{\"name\":\"EcsRamRole\",\"mode\":\"EcsRamRole\","
      << "\"ram_role_name\":\"test-ecs-role\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider("EcsRamRole");

  EXPECT_EQ(Constant::ECS_RAM_ROLE, provider.getProviderName());

  std::remove(tmpPath.c_str());
}

// mode=OIDC 须映射到 oidc_role_arn
TEST_F(CLIProfileProviderTest, OidcModeCreatesOidcRoleArnProvider) {
  std::string tokenPath = getTempDir() + "/test_cli_oidc_token.txt";
  {
    std::ofstream f(tokenPath);
    f << "fake-oidc-token";
  }

  std::string tmpPath = getTempDir() + "/test_cli_oidc_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"OIDC\",\"profiles\":["
      << "{\"name\":\"OIDC\",\"mode\":\"OIDC\","
      << "\"ram_role_arn\":\"acs:ram::123:role/oidc-role\","
      << "\"oidc_provider_arn\":\"acs:ram::123:oidc-provider/test\","
      << "\"oidc_token_file\":\"" << escapeJsonString(tokenPath) << "\","
      << "\"ram_session_name\":\"oidc-session\",\"expired_seconds\":3600}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider("OIDC");

  EXPECT_EQ(Constant::OIDC_ROLE_ARN, provider.getProviderName());

  std::remove(tmpPath.c_str());
  std::remove(tokenPath.c_str());
}

// mode=StsToken 须映射到 sts
TEST_F(CLIProfileProviderTest, StsTokenModeCreatesStsProvider) {
  std::string tmpPath = getTempDir() + "/test_cli_sts_token_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"default\",\"profiles\":["
      << "{\"name\":\"default\",\"mode\":\"StsToken\","
      << "\"access_key_id\":\"sts_ak\",\"access_key_secret\":\"sts_sk\","
      << "\"sts_token\":\"sts_token_value\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider;

  EXPECT_EQ(Constant::STS, provider.getProviderName());

  std::remove(tmpPath.c_str());
}

// 不支持的 mode 应抛错，而不是静默回落到 AccessKey
TEST_F(CLIProfileProviderTest, UnsupportedModeThrowsCredentialException) {
  std::string tmpPath = getTempDir() + "/test_cli_unsupported_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"default\",\"profiles\":["
      << "{\"name\":\"default\",\"mode\":\"ChainableRamRoleArn\","
      << "\"access_key_id\":\"ak\",\"access_key_secret\":\"sk\","
      << "\"ram_role_arn\":\"acs:ram::123:role/test\",\"source_profile\":\"AK\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider;

  try {
    provider.getProviderName();
    FAIL() << "Expected CredentialException";
  } catch (const CredentialException &e) {
    EXPECT_NE(std::string(e.what()).find("Unsupported profile mode"),
              std::string::npos)
        << "Expected unsupported mode error, got: " << e.what();
  }

  std::remove(tmpPath.c_str());
}

// mode="" -> mapCliModeToType 返回空 -> type empty 异常
TEST_F(CLIProfileProviderTest, EmptyStringModeThrowsCredentialException) {
  std::string tmpPath = getTempDir() + "/test_cli_empty_string_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"default\",\"profiles\":["
      << "{\"name\":\"default\",\"mode\":\"\","
      << "\"access_key_id\":\"ak\",\"access_key_secret\":\"sk\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider;

  try {
    provider.getProviderName();
    FAIL() << "Expected CredentialException";
  } catch (const CredentialException &e) {
    EXPECT_NE(std::string(e.what()).find("type"), std::string::npos)
        << "Expected empty type error, got: " << e.what();
  }

  std::remove(tmpPath.c_str());
}

// mode=AK 缺 AccessKeySecret -> 抛错
TEST_F(CLIProfileProviderTest, AkModeMissingSecretThrows) {
  std::string tmpPath = getTempDir() + "/test_cli_ak_missing_secret.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"default\",\"profiles\":["
      << "{\"name\":\"default\",\"mode\":\"AK\",\"access_key_id\":\"only_id\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider;

  try {
    provider.getProviderName();
    FAIL() << "Expected CredentialException";
  } catch (const CredentialException &e) {
    EXPECT_NE(std::string(e.what()).find("AccessKey"), std::string::npos)
        << "Expected AccessKey required error, got: " << e.what();
  }

  std::remove(tmpPath.c_str());
}

// mode=RsaKeyPair 映射到 rsa_key_pair
TEST_F(CLIProfileProviderTest, RsaKeyPairModeCreatesRsaKeyPairProvider) {
  std::string keyPath = getTempDir() + "/test_cli_rsa_private.pem";
  {
    std::ofstream f(keyPath);
    f << "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC0\n-----END PRIVATE KEY-----\n";
  }

  std::string tmpPath = getTempDir() + "/test_cli_rsa_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"default\",\"profiles\":["
      << "{\"name\":\"default\",\"mode\":\"RsaKeyPair\","
      << "\"public_key_id\":\"pk-id\",\"private_key_file\":\""
      << escapeJsonString(keyPath) << "\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider;

  EXPECT_EQ(Constant::RSA_KEY_PAIR, provider.getProviderName());

  std::remove(tmpPath.c_str());
  std::remove(keyPath.c_str());
}

// mode=CloudSSO 映射到 sso
TEST_F(CLIProfileProviderTest, CloudSsoModeCreatesCloudSsoProvider) {
  std::string tmpPath = getTempDir() + "/test_cli_cloud_sso_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"default\",\"profiles\":["
      << "{\"name\":\"default\",\"mode\":\"CloudSSO\",\"ram_role_name\":\"sso-role\","
      << "\"region_id\":\"cn-hangzhou\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider;

  EXPECT_EQ(Constant::CLOUD_SSO, provider.getProviderName());

  std::remove(tmpPath.c_str());
}

// mode=OAuth 映射到 oauth
TEST_F(CLIProfileProviderTest, OAuthModeCreatesOAuthProvider) {
  std::string tmpPath = getTempDir() + "/test_cli_oauth_mode.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"default\",\"profiles\":["
      << "{\"name\":\"default\",\"mode\":\"OAuth\","
      << "\"access_key_id\":\"client-id\",\"access_key_secret\":\"client-secret\","
      << "\"region_id\":\"cn-hangzhou\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider;

  EXPECT_EQ(Constant::OAUTH, provider.getProviderName());

  std::remove(tmpPath.c_str());
}

// RamRoleArn 解析 policy / external_id / ram_session_name
TEST_F(CLIProfileProviderTest, RamRoleArnParsesPolicyAndExternalId) {
  std::string tmpPath = getTempDir() + "/test_cli_ram_policy.json";
  {
    std::ofstream f(tmpPath);
    f << "{\"current\":\"RamRoleArn\",\"profiles\":["
      << "{\"name\":\"RamRoleArn\",\"mode\":\"RamRoleArn\","
      << "\"access_key_id\":\"akid\",\"access_key_secret\":\"secret\","
      << "\"ram_role_arn\":\"acs:ram::123:role/test\","
      << "\"ram_session_name\":\"session\",\"policy\":\"{\\\"Version\\\":\\\"1\\\"}\","
      << "\"external_id\":\"ext-1\",\"expired_seconds\":7200,\"sts_region\":\"cn-beijing\"}"
      << "]}";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider("RamRoleArn");

  EXPECT_EQ(Constant::RAM_ROLE_ARN, provider.getProviderName());

  std::remove(tmpPath.c_str());
}

// INI 未知 type：createProvider 抛 Unsupported profile type
// 注意：isJsonFile 会把以 '[' 开头的文件当成 JSON，故先放非 '[' 行
TEST_F(CLIProfileProviderTest, IniUnknownTypeThrows) {
  std::string tmpPath = getTempDir() + "/test_cli_unknown_type.ini";
  {
    std::ofstream f(tmpPath);
    f << "; aliyun cli ini\n"
      << "[default]\n"
      << "enable = true\n"
      << "type = unknown_cli_type\n"
      << "access_key_id = ak\n"
      << "access_key_secret = sk\n";
  }

  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", tmpPath.c_str(), 1);
  CLIProfileProvider provider;

  try {
    provider.getProviderName();
    FAIL() << "Expected CredentialException";
  } catch (const CredentialException &e) {
    EXPECT_NE(std::string(e.what()).find("Unsupported profile type"),
              std::string::npos)
        << "Expected unsupported type error, got: " << e.what();
  }

  std::remove(tmpPath.c_str());
}
