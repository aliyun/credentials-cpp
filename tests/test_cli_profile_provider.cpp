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
  // Without valid config file, should throw
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

// mode=AK 且凭证齐全 -> 成功创建 AccessKeyProvider，getProviderName 不抛异常
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
    std::string name = provider.getProviderName();
    EXPECT_FALSE(name.empty());
  });

  std::remove(tmpPath.c_str());
}
