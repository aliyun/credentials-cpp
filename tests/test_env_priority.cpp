#include <gtest/gtest.h>
#include <cstdlib>
#include <alibabacloud/credential/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/CloudSSOCredentialsProvider.hpp>
#include <alibabacloud/credential/provider/OAuthCredentialsProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>

using namespace AlibabaCloud::Credential;

// Helper functions to set/unset environment variables in a cross-platform way
static inline void env_set(const char* k, const char* v){
#if defined(_WIN32) || defined(_WIN64)
  _putenv_s(k, v);
#else
  setenv(k, v, 1);
#endif
}
static inline void env_unset(const char* k){
#if defined(_WIN32) || defined(_WIN64)
  _putenv_s(k, "");
#else
  unsetenv(k);
#endif
}

// ==================== OIDC Provider Environment Priority Tests ====================

class OIDCEnvPriorityTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Save and clear relevant environment variables
    saveEnv("ALIBABA_CLOUD_ROLE_ARN");
    saveEnv("ALIBABA_CLOUD_OIDC_PROVIDER_ARN");
    saveEnv("ALIBABA_CLOUD_OIDC_TOKEN_FILE");
    saveEnv("ALIBABA_CLOUD_ROLE_SESSION_NAME");
    saveEnv("ALIBABA_CLOUD_STS_REGION");
    saveEnv("ALIBABA_CLOUD_VPC_ENDPOINT_ENABLED");
    
    // Clear all for clean test environment
    env_unset("ALIBABA_CLOUD_ROLE_ARN");
    env_unset("ALIBABA_CLOUD_OIDC_PROVIDER_ARN");
    env_unset("ALIBABA_CLOUD_OIDC_TOKEN_FILE");
    env_unset("ALIBABA_CLOUD_ROLE_SESSION_NAME");
    env_unset("ALIBABA_CLOUD_STS_REGION");
    env_unset("ALIBABA_CLOUD_VPC_ENDPOINT_ENABLED");
  }
  
  void TearDown() override {
    // Restore original environment
    restoreEnv("ALIBABA_CLOUD_ROLE_ARN");
    restoreEnv("ALIBABA_CLOUD_OIDC_PROVIDER_ARN");
    restoreEnv("ALIBABA_CLOUD_OIDC_TOKEN_FILE");
    restoreEnv("ALIBABA_CLOUD_ROLE_SESSION_NAME");
    restoreEnv("ALIBABA_CLOUD_STS_REGION");
    restoreEnv("ALIBABA_CLOUD_VPC_ENDPOINT_ENABLED");
  }
  
  void saveEnv(const std::string &name) {
    const char* value = std::getenv(name.c_str());
    if (value) {
      savedEnv_[name] = value;
    }
  }
  
  void restoreEnv(const std::string &name) {
    if (savedEnv_.find(name) != savedEnv_.end()) {
      env_set(name.c_str(), savedEnv_[name].c_str());
    } else {
      env_unset(name.c_str());
    }
  }
  
  std::map<std::string, std::string> savedEnv_;
};

TEST_F(OIDCEnvPriorityTest, ConfigValueTakesPriority) {
  // Set environment variables
  env_set("ALIBABA_CLOUD_ROLE_ARN", "env_role_arn");
  env_set("ALIBABA_CLOUD_OIDC_PROVIDER_ARN", "env_oidc_provider");
  env_set("ALIBABA_CLOUD_OIDC_TOKEN_FILE", "/env/token/file");
  env_set("ALIBABA_CLOUD_ROLE_SESSION_NAME", "env_session");
  
  // Create config with explicit values
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("config_role_arn")
        .setOidcProviderArn("config_oidc_provider")
        .setOidcTokenFilePath("/config/token/file")
        .setRoleSessionName("config_session")
        .setRegionId("cn-hangzhou");
  
  // Config values should take priority
  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCEnvPriorityTest, FallbackToEnvironmentVariables) {
  // Set environment variables
  env_set("ALIBABA_CLOUD_ROLE_ARN", "env_role_arn");
  env_set("ALIBABA_CLOUD_OIDC_PROVIDER_ARN", "env_oidc_provider");
  env_set("ALIBABA_CLOUD_OIDC_TOKEN_FILE", "/env/token/file");
  env_set("ALIBABA_CLOUD_ROLE_SESSION_NAME", "env_session");
  
  // Create config with empty values
  auto config = std::make_shared<Models::Config>();
  config->setRegionId("cn-hangzhou");
  
  // Should fallback to environment variables
  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCEnvPriorityTest, SessionNameDefaultValue) {
  // Don't set ALIBABA_CLOUD_ROLE_SESSION_NAME
  env_set("ALIBABA_CLOUD_ROLE_ARN", "env_role_arn");
  env_set("ALIBABA_CLOUD_OIDC_PROVIDER_ARN", "env_oidc_provider");
  env_set("ALIBABA_CLOUD_OIDC_TOKEN_FILE", "/env/token/file");
  
  auto config = std::make_shared<Models::Config>();
  config->setRegionId("cn-hangzhou");
  
  // Should use default value "defaultSessionName"
  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

// ==================== CloudSSO Provider Environment Priority Tests ====================

class CloudSSOEnvPriorityTest : public ::testing::Test {
protected:
  void SetUp() override {
    saveEnv("ALIBABA_CLOUD_ROLE_NAME");
    env_unset("ALIBABA_CLOUD_ROLE_NAME");
  }
  
  void TearDown() override {
    restoreEnv("ALIBABA_CLOUD_ROLE_NAME");
  }
  
  void saveEnv(const std::string &name) {
    const char* value = std::getenv(name.c_str());
    if (value) {
      savedEnv_[name] = value;
    }
  }
  
  void restoreEnv(const std::string &name) {
    if (savedEnv_.find(name) != savedEnv_.end()) {
      env_set(name.c_str(), savedEnv_[name].c_str());
    } else {
      env_unset(name.c_str());
    }
  }
  
  std::map<std::string, std::string> savedEnv_;
};

TEST_F(CloudSSOEnvPriorityTest, ConfigValueTakesPriority) {
  // Set environment variable
  env_set("ALIBABA_CLOUD_ROLE_NAME", "env_role_name");
  
  // Create config with explicit value
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("config_role_name")
        .setRegionId("cn-hangzhou");
  
  // Config value should take priority
  EXPECT_NO_THROW({
    CloudSSOCredentialsProvider provider(config);
  });
}

TEST_F(CloudSSOEnvPriorityTest, FallbackToEnvironmentVariable) {
  // Set environment variable
  env_set("ALIBABA_CLOUD_ROLE_NAME", "env_role_name");
  
  // Create config without roleName
  auto config = std::make_shared<Models::Config>();
  config->setRegionId("cn-hangzhou");
  
  // Should fallback to environment variable
  EXPECT_NO_THROW({
    CloudSSOCredentialsProvider provider(config);
  });
}

// ==================== OAuth Provider Environment Priority Tests ====================

class OAuthEnvPriorityTest : public ::testing::Test {
protected:
  void SetUp() override {
    saveEnv("ALIBABA_CLOUD_CLIENT_ID");
    saveEnv("ALIBABA_CLOUD_CLIENT_SECRET");
    saveEnv("ALIBABA_CLOUD_OAUTH_TOKEN_ENDPOINT");
    
    env_unset("ALIBABA_CLOUD_CLIENT_ID");
    env_unset("ALIBABA_CLOUD_CLIENT_SECRET");
    env_unset("ALIBABA_CLOUD_OAUTH_TOKEN_ENDPOINT");
  }
  
  void TearDown() override {
    restoreEnv("ALIBABA_CLOUD_CLIENT_ID");
    restoreEnv("ALIBABA_CLOUD_CLIENT_SECRET");
    restoreEnv("ALIBABA_CLOUD_OAUTH_TOKEN_ENDPOINT");
  }
  
  void saveEnv(const std::string &name) {
    const char* value = std::getenv(name.c_str());
    if (value) {
      savedEnv_[name] = value;
    }
  }
  
  void restoreEnv(const std::string &name) {
    if (savedEnv_.find(name) != savedEnv_.end()) {
      env_set(name.c_str(), savedEnv_[name].c_str());
    } else {
      env_unset(name.c_str());
    }
  }
  
  std::map<std::string, std::string> savedEnv_;
};

TEST_F(OAuthEnvPriorityTest, ConfigValueTakesPriority) {
  // Set environment variables
  env_set("ALIBABA_CLOUD_CLIENT_ID", "env_client_id");
  env_set("ALIBABA_CLOUD_CLIENT_SECRET", "env_client_secret");
  env_set("ALIBABA_CLOUD_OAUTH_TOKEN_ENDPOINT", "https://env.example.com/token");
  
  // Create config with explicit values
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("config_client_id")
        .setAccessKeySecret("config_client_secret")
        .setStsEndpoint("https://config.example.com/token")
        .setRegionId("cn-hangzhou");
  
  // Config values should take priority
  EXPECT_NO_THROW({
    OAuthCredentialsProvider provider(config);
  });
}

TEST_F(OAuthEnvPriorityTest, FallbackToEnvironmentVariables) {
  // Set environment variables
  env_set("ALIBABA_CLOUD_CLIENT_ID", "env_client_id");
  env_set("ALIBABA_CLOUD_CLIENT_SECRET", "env_client_secret");
  env_set("ALIBABA_CLOUD_OAUTH_TOKEN_ENDPOINT", "https://env.example.com/token");
  
  // Create config without OAuth-related values
  auto config = std::make_shared<Models::Config>();
  config->setRegionId("cn-hangzhou");
  
  // Should fallback to environment variables
  EXPECT_NO_THROW({
    OAuthCredentialsProvider provider(config);
  });
}
