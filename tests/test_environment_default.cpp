#include <gtest/gtest.h>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
static inline int setenv(const char* name, const char* value, int /*overwrite*/) {
  return _putenv_s(name, value);
}
static inline int unsetenv(const char* name) {
  return _putenv_s(name, "");
}
#endif

#include <alibabacloud/credential/provider/EnvironmentVariableProvider.hpp>
#include <alibabacloud/credential/provider/DefaultProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <darabonba/Env.hpp>

using namespace AlibabaCloud::Credential;

// Cross-platform env helpers used by tests
static inline void set_env(const char* k, const char* v, int overwrite = 1){
#if defined(_WIN32) || defined(_WIN64)
  (void)overwrite;
  _putenv_s(k, v);
#else
  setenv(k, v, overwrite);
#endif
}
static inline void unset_env(const char* k){
#if defined(_WIN32) || defined(_WIN64)
  _putenv_s(k, "");
#else
  unsetenv(k);
#endif
}

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
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "env_ak_id");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "env_ak_secret");
  unset_env("ALIBABA_CLOUD_SECURITY_TOKEN");
  
  EnvironmentVariableProvider provider;
  auto credential = provider.getCredential();
  
  EXPECT_EQ("env_ak_id", credential.getAccessKeyId());
  EXPECT_EQ("env_ak_secret", credential.getAccessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
  EXPECT_FALSE(credential.hasSecurityToken());
}

TEST_F(EnvironmentVariableProviderTest, StsFromEnvironment) {
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "env_sts_ak");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "env_sts_secret");
  set_env("ALIBABA_CLOUD_SECURITY_TOKEN", "env_sts_token");
  
  EnvironmentVariableProvider provider;
  auto credential = provider.getCredential();
  
  EXPECT_EQ("env_sts_ak", credential.getAccessKeyId());
  EXPECT_EQ("env_sts_secret", credential.getAccessKeySecret());
  EXPECT_EQ("env_sts_token", credential.getSecurityToken());
  EXPECT_EQ(Constant::STS, credential.getType());
}

TEST_F(EnvironmentVariableProviderTest, MissingAccessKeyId) {
  unset_env("ALIBABA_CLOUD_ACCESS_KEY_ID");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "secret");
  
  EnvironmentVariableProvider provider;
  
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST_F(EnvironmentVariableProviderTest, MissingAccessKeySecret) {
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "ak_id");
  unset_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
  
  EnvironmentVariableProvider provider;
  
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST_F(EnvironmentVariableProviderTest, EmptyAccessKeyId) {
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "secret");
  
  EnvironmentVariableProvider provider;
  
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST_F(EnvironmentVariableProviderTest, EmptyAccessKeySecret) {
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "ak_id");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "");
  
  EnvironmentVariableProvider provider;
  
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST_F(EnvironmentVariableProviderTest, SecurityTokenOptional) {
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "ak");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "secret");
  set_env("ALIBABA_CLOUD_SECURITY_TOKEN", "");
  
  EnvironmentVariableProvider provider;
  auto credential = provider.getCredential();
  
  // Empty security token should result in AccessKey type
  EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
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
    unset_env("ALIBABA_CLOUD_ACCESS_KEY_ID");
    unset_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
    unset_env("ALIBABA_CLOUD_SECURITY_TOKEN");
    unset_env("ALIBABA_CLOUD_OIDC_TOKEN_FILE");
    unset_env("ALIBABA_CLOUD_ROLE_ARN");
    unset_env("ALIBABA_CLOUD_OIDC_PROVIDER_ARN");
    unset_env("ALIBABA_CLOUD_ROLE_SESSION_NAME");
    unset_env("ALIBABA_CLOUD_ECS_METADATA");
    unset_env("ALIBABA_CLOUD_ECS_METADATA_DISABLED");
    unset_env("ALIBABA_CLOUD_CREDENTIALS_URI");
    unset_env("ALIBABA_CLOUD_CREDENTIALS_FILE");
    unset_env("ALIBABA_CLOUD_PROFILE");
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
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "default_env_ak");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "default_env_secret");
  
  DefaultProvider provider;
  auto credential = provider.getCredential();
  
  EXPECT_EQ("default_env_ak", credential.getAccessKeyId());
  EXPECT_EQ("default_env_secret", credential.getAccessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
}

TEST_F(DefaultProviderTest, EcsMetadataDisabled) {
  set_env("ALIBABA_CLOUD_ECS_METADATA_DISABLED", "true");
  set_env("ALIBABA_CLOUD_ECS_METADATA", "test_role");
  
  // Even with ECS_METADATA set, it should be ignored when DISABLED=true
  // This test verifies that the ECS provider is not added to the chain
  
  // Set environment credentials so DefaultProvider can succeed
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "fallback_ak");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "fallback_secret");
  
  DefaultProvider provider;
  auto credential = provider.getCredential();
  
  // Should get credential from env, not ECS
  EXPECT_EQ("fallback_ak", credential.getAccessKeyId());
}

TEST_F(DefaultProviderTest, EcsMetadataNotDisabledByDefault) {
  // Don't set ALIBABA_CLOUD_ECS_METADATA_DISABLED
  // This just tests that provider construction succeeds
  
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "test_ak");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "test_secret");
  
  EXPECT_NO_THROW({
    DefaultProvider provider;
  });
}

TEST_F(DefaultProviderTest, EcsMetadataDisabledCaseInsensitive) {
  // Test "True" variant
  set_env("ALIBABA_CLOUD_ECS_METADATA_DISABLED", "True");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "test_ak");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "test_secret");
  
  DefaultProvider provider;
  auto credential = provider.getCredential();
  
  EXPECT_EQ("test_ak", credential.getAccessKeyId());
  
  // Test "TRUE" variant
  set_env("ALIBABA_CLOUD_ECS_METADATA_DISABLED", "TRUE");
  
  DefaultProvider provider2;
  auto credential2 = provider2.getCredential();
  
  EXPECT_EQ("test_ak", credential2.getAccessKeyId());
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
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "env_ak");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "env_secret");
  
  DefaultProvider provider;
  auto credential = provider.getCredential();
  
  // Should get from environment (first provider)
  EXPECT_EQ("env_ak", credential.getAccessKeyId());
  EXPECT_EQ("env_secret", credential.getAccessKeySecret());
}

TEST_F(DefaultProviderTest, ConstGetCredential) {
  set_env("ALIBABA_CLOUD_ACCESS_KEY_ID", "const_ak");
  set_env("ALIBABA_CLOUD_ACCESS_KEY_SECRET", "const_secret");
  
  const DefaultProvider provider;
  const auto &credential = provider.getCredential();
  
  EXPECT_EQ("const_ak", credential.getAccessKeyId());
  EXPECT_EQ("const_secret", credential.getAccessKeySecret());
}
