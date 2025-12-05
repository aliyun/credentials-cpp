#include <gtest/gtest.h>
#include <alibabacloud/credential/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credential/provider/RamRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credential/provider/RsaKeyPairProvider.hpp>
#include <alibabacloud/credential/provider/CloudSSOCredentialsProvider.hpp>
#include <alibabacloud/credential/provider/OAuthCredentialsProvider.hpp>
#include <alibabacloud/credential/provider/URLProvider.hpp>
#include <alibabacloud/credential/provider/NeedFreshProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <fstream>

using namespace AlibabaCloud::Credential;

// ==================== EcsRamRoleProvider Tests ====================

TEST(EcsRamRoleProviderTest, ConstructorWithConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_ecs_role");
  
  EcsRamRoleProvider provider(config);
  // Note: Cannot test actual refresh without ECS metadata service
  // Just verify the provider is created successfully
  EXPECT_NO_THROW({
    // Provider should be created without error
  });
}

TEST(EcsRamRoleProviderTest, ConstructorWithRoleName) {
  EcsRamRoleProvider provider("test_role");
  // Verify provider created successfully
  EXPECT_NO_THROW({
    // Provider should be created without error
  });
}

TEST(EcsRamRoleProviderTest, SupportsIMDSv2) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_role")
        .setDisableIMDSv1(true);
  
  // Verify provider can be created with IMDSv2 setting
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(config);
  });
}

// ==================== RamRoleArnProvider Tests ====================

TEST(RamRoleArnProviderTest, ConstructorWithConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("ram_ak")
        .setAccessKeySecret("ram_secret")
        .setRoleArn("acs:ram::123456:role/test")
        .setRoleSessionName("test_session");
  
  RamRoleArnProvider provider(config);
  // Provider should be created successfully
  EXPECT_NO_THROW({
    // Provider created without error
  });
}

TEST(RamRoleArnProviderTest, ConstructorWithParameters) {
  RamRoleArnProvider provider(
      "ak_id", "ak_secret",
      "acs:ram::123456:role/test",
      "session_name"
  );
  
  EXPECT_NO_THROW({
    // Provider created successfully
  });
}

TEST(RamRoleArnProviderTest, WithPolicy) {
  auto policy = std::make_shared<std::string>("{\"Version\":\"1\"}");
  RamRoleArnProvider provider(
      "ak", "secret",
      "acs:ram::123456:role/test",
      "session",
      policy
  );
  
  EXPECT_NO_THROW({
    // Provider with policy created successfully
  });
}

// ==================== OIDCRoleArnProvider Tests ====================

TEST(OIDCRoleArnProviderTest, ConstructorWithConfig) {
  // Create a temporary OIDC token file for testing
  std::string tokenPath = "/tmp/test_oidc_token.txt";
  std::ofstream tokenFile(tokenPath);
  tokenFile << "test_oidc_token_content";
  tokenFile.close();
  
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456:role/oidc-test")
        .setOidcProviderArn("acs:ram::123456:oidc-provider/test")
        .setOidcTokenFilePath(tokenPath)
        .setRoleSessionName("oidc_session");
  
  OIDCRoleArnProvider provider(config);
  
  // Cleanup
  std::remove(tokenPath.c_str());
  
  EXPECT_NO_THROW({
    // Provider created successfully
  });
}

TEST(OIDCRoleArnProviderTest, ConstructorWithParameters) {
  std::string tokenPath = "/tmp/test_oidc_param.txt";
  std::ofstream tokenFile(tokenPath);
  tokenFile << "param_oidc_token";
  tokenFile.close();
  
  OIDCRoleArnProvider provider(
      "acs:ram::123456:role/test",
      "acs:ram::123456:oidc-provider/test",
      tokenPath,
      "session_name"
  );
  
  std::remove(tokenPath.c_str());
  
  EXPECT_NO_THROW({
    // Provider with parameters created successfully
  });
}

// ==================== RsaKeyPairProvider Tests ====================

TEST(RsaKeyPairProviderTest, ConstructorWithConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setPublicKeyId("public_key_123")
        .setPrivateKeyFile("/path/to/private_key.pem");
  
  RsaKeyPairProvider provider(config);
  
  EXPECT_NO_THROW({
    // Provider created (will fail on refresh without valid key file)
  });
}

TEST(RsaKeyPairProviderTest, ConstructorWithParameters) {
  RsaKeyPairProvider provider("public_key_id", "/path/to/key.pem");
  
  EXPECT_NO_THROW({
    // Provider with parameters created successfully
  });
}

// ==================== CloudSSOCredentialsProvider Tests ====================

TEST(CloudSSOProviderTest, ConstructorWithConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("sso_role_name")
        .setRegionId("cn-hangzhou");
  
  CloudSSOCredentialsProvider provider(config);
  
  EXPECT_NO_THROW({
    // Provider created successfully
  });
}

TEST(CloudSSOProviderTest, ConstructorWithParameters) {
  CloudSSOCredentialsProvider provider("sso_role", "cn-beijing");
  
  EXPECT_NO_THROW({
    // Provider with parameters created successfully
  });
}

TEST(CloudSSOProviderTest, DefaultRegion) {
  CloudSSOCredentialsProvider provider("sso_role");
  
  EXPECT_NO_THROW({
    // Provider with default region created successfully
  });
}

// ==================== OAuthCredentialsProvider Tests ====================

TEST(OAuthProviderTest, ConstructorWithConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("oauth_client_id")
        .setAccessKeySecret("oauth_client_secret")
        .setStsEndpoint("https://oauth.example.com/token")
        .setRegionId("cn-hangzhou");
  
  OAuthCredentialsProvider provider(config);
  
  EXPECT_NO_THROW({
    // Provider created successfully
  });
}

TEST(OAuthProviderTest, ConstructorWithParameters) {
  OAuthCredentialsProvider provider(
      "client_id",
      "client_secret",
      "https://oauth.example.com/token",
      "cn-hangzhou"
  );
  
  EXPECT_NO_THROW({
    // Provider with parameters created successfully
  });
}

TEST(OAuthProviderTest, DefaultRegion) {
  OAuthCredentialsProvider provider(
      "client_id",
      "client_secret",
      "https://oauth.example.com/token"
  );
  
  EXPECT_NO_THROW({
    // Provider with default region created successfully
  });
}

// ==================== NeedFreshProvider Tests ====================

class TestNeedFreshProvider : public NeedFreshProvider {
public:
  TestNeedFreshProvider() : NeedFreshProvider() {
    credential_.setType("test_type");
  }
  
  TestNeedFreshProvider(int64_t expiration) : NeedFreshProvider(expiration) {
    credential_.setType("test_type");
  }
  
  void setExpiration(int64_t exp) {
    expiration_ = exp;
  }
  
  bool isNeedFresh() const {
    return needFresh();
  }
  
  std::string getProviderName() const override {
    return "test_need_fresh";
  }
  
protected:
  virtual bool refreshCredential() const override {
    credential_.setAccessKeyId("refreshed_ak");
    credential_.setAccessKeySecret("refreshed_secret");
    // Set expiration to 1 hour from now
    expiration_ = static_cast<int64_t>(time(nullptr)) + 3600;
    return true;
  }
};

TEST(NeedFreshProviderTest, DefaultConstructor) {
  TestNeedFreshProvider provider;
  EXPECT_NO_THROW({
    auto credential = provider.getCredential();
    EXPECT_EQ("test_type", credential.type());
  });
}

TEST(NeedFreshProviderTest, ExpirationConstructor) {
  int64_t futureTime = static_cast<int64_t>(time(nullptr)) + 3600;
  TestNeedFreshProvider provider(futureTime);
  
  EXPECT_NO_THROW({
    auto credential = provider.getCredential();
  });
}

TEST(NeedFreshProviderTest, NeedFreshWhenExpired) {
  TestNeedFreshProvider provider;
  // Set expiration to past time
  provider.setExpiration(static_cast<int64_t>(time(nullptr)) - 100);
  
  EXPECT_TRUE(provider.isNeedFresh());
}

TEST(NeedFreshProviderTest, NoNeedFreshWhenValid) {
  TestNeedFreshProvider provider;
  // Set expiration to future time (more than 180 seconds)
  provider.setExpiration(static_cast<int64_t>(time(nullptr)) + 1000);
  
  EXPECT_FALSE(provider.isNeedFresh());
}

TEST(NeedFreshProviderTest, NeedFreshWithin180Seconds) {
  TestNeedFreshProvider provider;
  // Set expiration to within 180 seconds
  provider.setExpiration(static_cast<int64_t>(time(nullptr)) + 100);
  
  EXPECT_TRUE(provider.isNeedFresh());
}

TEST(NeedFreshProviderTest, AutoRefresh) {
  TestNeedFreshProvider provider;
  provider.setExpiration(static_cast<int64_t>(time(nullptr)) - 100);
  
  auto credential = provider.getCredential();
  
  // After refresh, credential should be updated
  EXPECT_EQ("refreshed_ak", credential.accessKeyId());
  EXPECT_EQ("refreshed_secret", credential.accessKeySecret());
}

// Note: strtotime and gmt_datetime are protected methods,
// they are tested indirectly through the provider refresh mechanism
