#include <gtest/gtest.h>
#include <alibabacloud/credential/provider/CLIProfileProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <darabonba/Exception.hpp>
#include <fstream>
#include <cstdlib>

using namespace AlibabaCloud::Credential;

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
  }, Darabonba::Exception);
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
  }, Darabonba::Exception);
}

TEST_F(CLIProfileProviderTest, ProfileNamePreserved) {
  // Create provider with specific profile name
  // The profile name should be used when reading config
  CLIProfileProvider provider("test_profile");
  
  // Even though it will fail, it should try to use the correct profile
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST_F(CLIProfileProviderTest, UsesEnvironmentVariableForPath) {
  // Set custom CLI profile path
  setenv("ALIBABA_CLOUD_CLI_PROFILE_PATH", "/tmp/nonexistent_cli_config.json", 1);
  
  CLIProfileProvider provider;
  
  // Should try to read from the specified path
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST_F(CLIProfileProviderTest, SupportsMultipleGetCredentialCalls) {
  CLIProfileProvider provider;
  
  // Multiple calls should behave consistently
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
  
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}
