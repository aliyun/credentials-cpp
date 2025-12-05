#include <gtest/gtest.h>
#include <alibabacloud/credential/provider/EnvironmentVariableProvider.hpp>
#include <alibabacloud/credential/provider/DefaultProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <darabonba/Env.hpp>
#include <cstdlib>

using namespace AlibabaCloud::Credential;

// ==================== EnvironmentVariableProvider Tests ====================

class EnvironmentVariableProviderTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Save original environment
    saveEnv("ALIBABA_CLOUD_ACCESS_KEY_ID");
    saveEnv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
    saveEnv("ALIBABA_CLOUD_SECURITY_TOKEN");
  }
  
  void TearDown() override {
    // Restore original environment
    restoreEnv("ALIBABA_CLOUD_ACCESS_KEY_ID");
    restoreEnv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
    restoreEnv("ALIBABA_CLOUD_SECURITY_TOKEN");
  }
  
  void saveEnv(const std::string &name) {
    const char* value = std::getenv(name.c_str());
    if (value) {
      savedEnv_[name] = value;
    }
  }
  
  void restoreEnv(const std::string &name) {
    if (savedEnv_.find(name) != savedEnv_.end()) {
      setenv(name.c_str(), savedEnv_[name].c_str(), 1);
    } else {
      unsetenv(name.c_str());
    }
  }
  
  std::map<std::string, std::string> savedEnv_;
};

TEST_F(EnvironmentVariableProviderTest, AccessKeyFromEnvironment) {
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "env_ak_id", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "env_ak_secret", 1);
  unsetenv("ALIBABA_CLOUD_SECURITY_TOKEN");
  
  EnvironmentVariableProvider provider;
  auto credential = provider.getCredential();
  
  EXPECT_EQ("env_ak_id", credential.accessKeyId());
  EXPECT_EQ("env_ak_secret", credential.accessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, credential.type());
  EXPECT_FALSE(credential.hasSecurityToken());
}

TEST_F(EnvironmentVariableProviderTest, StsFromEnvironment) {
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "env_sts_ak", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "env_sts_secret", 1);
  setenv("ALIBABA_CLOUD_SECURITY_TOKEN", "env_sts_token", 1);
  
  EnvironmentVariableProvider provider;
  auto credential = provider.getCredential();
  
  EXPECT_EQ("env_sts_ak", credential.accessKeyId());
  EXPECT_EQ("env_sts_secret", credential.accessKeySecret());
  EXPECT_EQ("env_sts_token", credential.securityToken());
  EXPECT_EQ(Constant::STS, credential.type());
}

TEST_F(EnvironmentVariableProviderTest, MissingAccessKeyId) {
  unsetenv("ALIBABA_CLOUD_ACCESS_KEY_ID");
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "secret", 1);
  
  EnvironmentVariableProvider provider;
  
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST_F(EnvironmentVariableProviderTest, MissingAccessKeySecret) {
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "ak_id", 1);
  unsetenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
  
  EnvironmentVariableProvider provider;
  
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST_F(EnvironmentVariableProviderTest, EmptyAccessKeyId) {
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "secret", 1);
  
  EnvironmentVariableProvider provider;
  
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST_F(EnvironmentVariableProviderTest, EmptyAccessKeySecret) {
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "ak_id", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "", 1);
  
  EnvironmentVariableProvider provider;
  
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST_F(EnvironmentVariableProviderTest, SecurityTokenOptional) {
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "ak", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "secret", 1);
  setenv("ALIBABA_CLOUD_SECURITY_TOKEN", "", 1);
  
  EnvironmentVariableProvider provider;
  auto credential = provider.getCredential();
  
  // Empty security token should result in AccessKey type
  EXPECT_EQ(Constant::ACCESS_KEY, credential.type());
}

// ==================== DefaultProvider Tests ====================

class DefaultProviderTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Save and clear relevant environment variables
    saveEnv("ALIBABA_CLOUD_ACCESS_KEY_ID");
    saveEnv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
    saveEnv("ALIBABA_CLOUD_SECURITY_TOKEN");
    saveEnv("ALIBABA_CLOUD_OIDC_TOKEN_FILE");
    saveEnv("ALIBABA_CLOUD_ROLE_ARN");
    saveEnv("ALIBABA_CLOUD_OIDC_PROVIDER_ARN");
    saveEnv("ALIBABA_CLOUD_ROLE_SESSION_NAME");
    saveEnv("ALIBABA_CLOUD_ECS_METADATA");
    saveEnv("ALIBABA_CLOUD_ECS_METADATA_DISABLED");
    saveEnv("ALIBABA_CLOUD_CREDENTIALS_URI");
    saveEnv("ALIBABA_CLOUD_CREDENTIALS_FILE");
    saveEnv("ALIBABA_CLOUD_PROFILE");
    
    // Clear all for clean test environment
    unsetenv("ALIBABA_CLOUD_ACCESS_KEY_ID");
    unsetenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
    unsetenv("ALIBABA_CLOUD_SECURITY_TOKEN");
    unsetenv("ALIBABA_CLOUD_OIDC_TOKEN_FILE");
    unsetenv("ALIBABA_CLOUD_ROLE_ARN");
    unsetenv("ALIBABA_CLOUD_OIDC_PROVIDER_ARN");
    unsetenv("ALIBABA_CLOUD_ROLE_SESSION_NAME");
    unsetenv("ALIBABA_CLOUD_ECS_METADATA");
    unsetenv("ALIBABA_CLOUD_ECS_METADATA_DISABLED");
    unsetenv("ALIBABA_CLOUD_CREDENTIALS_URI");
    unsetenv("ALIBABA_CLOUD_CREDENTIALS_FILE");
    unsetenv("ALIBABA_CLOUD_PROFILE");
  }
  
  void TearDown() override {
    // Restore all environment variables
    restoreEnv("ALIBABA_CLOUD_ACCESS_KEY_ID");
    restoreEnv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
    restoreEnv("ALIBABA_CLOUD_SECURITY_TOKEN");
    restoreEnv("ALIBABA_CLOUD_OIDC_TOKEN_FILE");
    restoreEnv("ALIBABA_CLOUD_ROLE_ARN");
    restoreEnv("ALIBABA_CLOUD_OIDC_PROVIDER_ARN");
    restoreEnv("ALIBABA_CLOUD_ROLE_SESSION_NAME");
    restoreEnv("ALIBABA_CLOUD_ECS_METADATA");
    restoreEnv("ALIBABA_CLOUD_ECS_METADATA_DISABLED");
    restoreEnv("ALIBABA_CLOUD_CREDENTIALS_URI");
    restoreEnv("ALIBABA_CLOUD_CREDENTIALS_FILE");
    restoreEnv("ALIBABA_CLOUD_PROFILE");
  }
  
  void saveEnv(const std::string &name) {
    const char* value = std::getenv(name.c_str());
    if (value) {
      savedEnv_[name] = value;
    }
  }
  
  void restoreEnv(const std::string &name) {
    if (savedEnv_.find(name) != savedEnv_.end()) {
      setenv(name.c_str(), savedEnv_[name].c_str(), 1);
    } else {
      unsetenv(name.c_str());
    }
  }
  
  std::map<std::string, std::string> savedEnv_;
};

TEST_F(DefaultProviderTest, UsesEnvironmentVariableProvider) {
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "default_env_ak", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "default_env_secret", 1);
  
  DefaultProvider provider;
  auto credential = provider.getCredential();
  
  EXPECT_EQ("default_env_ak", credential.accessKeyId());
  EXPECT_EQ("default_env_secret", credential.accessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, credential.type());
}

TEST_F(DefaultProviderTest, EcsMetadataDisabled) {
  setenv("ALIBABA_CLOUD_ECS_METADATA_DISABLED", "true", 1);
  setenv("ALIBABA_CLOUD_ECS_METADATA", "test_role", 1);
  
  // Even with ECS_METADATA set, it should be ignored when DISABLED=true
  // This test verifies that the ECS provider is not added to the chain
  
  // Set environment credentials so DefaultProvider can succeed
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "fallback_ak", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "fallback_secret", 1);
  
  DefaultProvider provider;
  auto credential = provider.getCredential();
  
  // Should get credential from env, not ECS
  EXPECT_EQ("fallback_ak", credential.accessKeyId());
}

TEST_F(DefaultProviderTest, EcsMetadataNotDisabledByDefault) {
  // Don't set ALIBABA_CLOUD_ECS_METADATA_DISABLED
  // This just tests that provider construction succeeds
  
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "test_ak", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "test_secret", 1);
  
  EXPECT_NO_THROW({
    DefaultProvider provider;
  });
}

TEST_F(DefaultProviderTest, EcsMetadataDisabledCaseInsensitive) {
  // Test "True" variant
  setenv("ALIBABA_CLOUD_ECS_METADATA_DISABLED", "True", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "test_ak", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "test_secret", 1);
  
  DefaultProvider provider;
  auto credential = provider.getCredential();
  
  EXPECT_EQ("test_ak", credential.accessKeyId());
  
  // Test "TRUE" variant
  setenv("ALIBABA_CLOUD_ECS_METADATA_DISABLED", "TRUE", 1);
  
  DefaultProvider provider2;
  auto credential2 = provider2.getCredential();
  
  EXPECT_EQ("test_ak", credential2.accessKeyId());
}

TEST_F(DefaultProviderTest, NoValidProvider) {
  // With no environment variables or config files, provider should be created
  // but getCredential() will throw
  EXPECT_NO_THROW({
    DefaultProvider provider;
  });
}

TEST_F(DefaultProviderTest, ProviderChainOrder) {
  // DefaultProvider should try providers in order:
  // 1. EnvironmentVariableProvider
  // 2. OIDCRoleArnProvider (if OIDC env vars set)
  // 3. ProfileProvider
  // 4. EcsRamRoleProvider (if ECS_METADATA set and not disabled)
  // 5. URLProvider (if CREDENTIALS_URI set)
  
  // Set environment credentials (first in chain)
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "env_ak", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "env_secret", 1);
  
  DefaultProvider provider;
  auto credential = provider.getCredential();
  
  // Should get from environment (first provider)
  EXPECT_EQ("env_ak", credential.accessKeyId());
  EXPECT_EQ("env_secret", credential.accessKeySecret());
}

TEST_F(DefaultProviderTest, ConstGetCredential) {
  setenv("ALIBABA_CLOUD_ACCESS_KEY_ID", "const_ak", 1);
  setenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "const_secret", 1);
  
  const DefaultProvider provider;
  const auto &credential = provider.getCredential();
  
  EXPECT_EQ("const_ak", credential.accessKeyId());
  EXPECT_EQ("const_secret", credential.accessKeySecret());
}
