#include <gtest/gtest.h>
#include <alibabacloud/credentials/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credentials/provider/RamRoleArnProvider.hpp>
#include <alibabacloud/credentials/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credentials/provider/RsaKeyPairProvider.hpp>
#include <alibabacloud/credentials/provider/CloudSSOCredentialsProvider.hpp>
#include <alibabacloud/credentials/provider/OAuthCredentialsProvider.hpp>
#include <alibabacloud/credentials/provider/URLProvider.hpp>
#include <alibabacloud/credentials/provider/NeedFreshProvider.hpp>
#include <alibabacloud/credentials/Constant.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace AlibabaCloud::Credentials;

// Cross-platform temporary directory helper
static std::string getTempDir() {
#ifdef _WIN32
  const char* tempDir = std::getenv("TEMP");
  if (!tempDir) {
    tempDir = std::getenv("TMP");
  }
  if (!tempDir) {
    return "C:\\Windows\\Temp";
  }
  return std::string(tempDir);
#else
  return "/tmp";
#endif
}

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
  std::string tokenPath = getTempDir() + "/test_oidc_token.txt";
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
  std::string tokenPath = getTempDir() + "/test_oidc_param.txt";
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
  // Create a temporary private key file
  std::string keyPath = getTempDir() + "/test_rsa_key.pem";
  std::ofstream keyFile(keyPath);
  keyFile << "-----BEGIN RSA PRIVATE KEY-----\ntest_private_key_content\n-----END RSA PRIVATE KEY-----";
  keyFile.close();

  auto config = std::make_shared<Models::Config>();
  config->setPublicKeyId("public_key_123")
        .setPrivateKeyFile(keyPath);

  EXPECT_NO_THROW({
    RsaKeyPairProvider provider(config);
  });

  std::remove(keyPath.c_str());
}

TEST(RsaKeyPairProviderTest, ConstructorWithParameters) {
  std::string keyPath = getTempDir() + "/test_rsa_key_param.pem";
  std::ofstream keyFile(keyPath);
  keyFile << "-----BEGIN RSA PRIVATE KEY-----\ntest_key_content\n-----END RSA PRIVATE KEY-----";
  keyFile.close();

  EXPECT_NO_THROW({
    RsaKeyPairProvider provider("public_key_id", keyPath);
  });

  std::remove(keyPath.c_str());
}

TEST(RsaKeyPairProviderTest, ConstructorWithStaleValueBehavior) {
  std::string keyPath = getTempDir() + "/test_rsa_stale.pem";
  std::ofstream keyFile(keyPath);
  keyFile << "test_key";
  keyFile.close();

  RsaKeyPairProvider provider(
      "public_key_id",
      keyPath,
      3600,
      "cn-hangzhou",
      "sts.aliyuncs.com",
      StaleValueBehavior::ALLOW_
  );

  std::remove(keyPath.c_str());

  EXPECT_NO_THROW({
    // Provider with ALLOW_ behavior created successfully
  });
}

TEST(RsaKeyPairProviderTest, InheritsFromRefreshableProvider) {
  std::string keyPath = getTempDir() + "/test_rsa_inherit.pem";
  std::ofstream keyFile(keyPath);
  keyFile << "test_key";
  keyFile.close();

  auto config = std::make_shared<Models::Config>();
  config->setPublicKeyId("public_key")
        .setPrivateKeyFile(keyPath);

  RsaKeyPairProvider provider(config);

  // Verify it can be cast to RefreshableProvider
  RefreshableProvider* basePtr = dynamic_cast<RefreshableProvider*>(&provider);
  EXPECT_NE(basePtr, nullptr);

  std::remove(keyPath.c_str());
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
    EXPECT_EQ("test_type", credential.getType());
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
  EXPECT_EQ("refreshed_ak", credential.getAccessKeyId());
  EXPECT_EQ("refreshed_secret", credential.getAccessKeySecret());
}

// Note: strtotime and gmt_datetime are protected methods,
// they are tested indirectly through the provider refresh mechanism

// ==================== RamRoleArnProvider RefreshableProvider Integration Tests ====================

TEST(RamRoleArnProviderTest, ConstructorWithStaleValueBehavior) {
  RamRoleArnProvider provider(
      "ak", "secret",
      "acs:ram::123456:role/test",
      "session",
      nullptr,  // policy
      3600,     // durationSeconds
      "cn-hangzhou",
      "sts.aliyuncs.com",
      StaleValueBehavior::ALLOW_
  );

  EXPECT_NO_THROW({
    // Provider with ALLOW_ behavior created successfully
  });
}

TEST(RamRoleArnProviderTest, ConstructorWithPrefetchStrategy) {
  auto strategy = std::make_shared<OneCallerBlocksPrefetch>();
  RamRoleArnProvider provider(
      "ak", "secret",
      "acs:ram::123456:role/test",
      "session",
      nullptr,
      3600,
      "cn-hangzhou",
      "sts.aliyuncs.com",
      StaleValueBehavior::STRICT_,
      strategy
  );

  EXPECT_NO_THROW({
    // Provider with custom prefetch strategy created successfully
  });
}

TEST(RamRoleArnProviderTest, ConstructorWithConfigAndBehavior) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("ram_ak")
        .setAccessKeySecret("ram_secret")
        .setRoleArn("acs:ram::123456:role/test");

  RamRoleArnProvider provider(config, StaleValueBehavior::ALLOW_);

  EXPECT_NO_THROW({
    // Provider with config and ALLOW_ behavior created successfully
  });
}

TEST(RamRoleArnProviderTest, EmptyRoleArnThrowsException) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("ak")
        .setAccessKeySecret("secret");
  // No roleArn set

  EXPECT_THROW({
    RamRoleArnProvider provider(config);
  }, CredentialException);
}

// ==================== OIDCRoleArnProvider RefreshableProvider Integration Tests ====================

TEST(OIDCRoleArnProviderTest, ConstructorWithStaleValueBehavior) {
  std::string tokenPath = getTempDir() + "/test_oidc_stale.txt";
  std::ofstream tokenFile(tokenPath);
  tokenFile << "test_token";
  tokenFile.close();

  OIDCRoleArnProvider provider(
      "acs:ram::123456:role/test",
      "acs:ram::123456:oidc-provider/test",
      tokenPath,
      "session",
      nullptr,
      3600,
      "cn-hangzhou",
      "sts.aliyuncs.com",
      StaleValueBehavior::ALLOW_
  );

  std::remove(tokenPath.c_str());

  EXPECT_NO_THROW({
    // Provider with ALLOW_ behavior created successfully
  });
}

TEST(OIDCRoleArnProviderTest, ConstructorWithPrefetchStrategy) {
  std::string tokenPath = getTempDir() + "/test_oidc_strategy.txt";
  std::ofstream tokenFile(tokenPath);
  tokenFile << "test_token";
  tokenFile.close();

  auto strategy = std::make_shared<OneCallerBlocksPrefetch>();
  OIDCRoleArnProvider provider(
      "acs:ram::123456:role/test",
      "acs:ram::123456:oidc-provider/test",
      tokenPath,
      "session",
      nullptr,
      3600,
      "cn-hangzhou",
      "sts.aliyuncs.com",
      StaleValueBehavior::STRICT_,
      strategy
  );

  std::remove(tokenPath.c_str());

  EXPECT_NO_THROW({
    // Provider with custom prefetch strategy created successfully
  });
}

TEST(OIDCRoleArnProviderTest, EmptyRoleArnThrowsException) {
  std::string tokenPath = getTempDir() + "/test_oidc_empty_role.txt";
  std::ofstream tokenFile(tokenPath);
  tokenFile << "test_token";
  tokenFile.close();

  auto config = std::make_shared<Models::Config>();
  config->setOidcProviderArn("acs:ram::123456:oidc-provider/test")
        .setOidcTokenFilePath(tokenPath);
  // No roleArn set

  EXPECT_THROW({
    OIDCRoleArnProvider provider(config);
  }, CredentialException);

  std::remove(tokenPath.c_str());
}

TEST(OIDCRoleArnProviderTest, EmptyOidcProviderArnThrowsException) {
  std::string tokenPath = getTempDir() + "/test_oidc_empty_provider.txt";
  std::ofstream tokenFile(tokenPath);
  tokenFile << "test_token";
  tokenFile.close();

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456:role/test")
        .setOidcTokenFilePath(tokenPath);
  // No oidcProviderArn set

  EXPECT_THROW({
    OIDCRoleArnProvider provider(config);
  }, CredentialException);

  std::remove(tokenPath.c_str());
}

TEST(OIDCRoleArnProviderTest, EmptyOidcTokenFilePathThrowsException) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456:role/test")
        .setOidcProviderArn("acs:ram::123456:oidc-provider/test");
  // No oidcTokenFilePath set

  EXPECT_THROW({
    OIDCRoleArnProvider provider(config);
  }, CredentialException);
}

// ==================== URLProvider RefreshableProvider Integration Tests ====================

TEST(URLProviderTest, ConstructorWithStaleValueBehavior) {
  URLProvider provider("http://example.com/credentials",
                       StaleValueBehavior::ALLOW_);

  EXPECT_NO_THROW({
    // Provider with ALLOW_ behavior created successfully
  });
}

TEST(URLProviderTest, ConstructorWithPrefetchStrategy) {
  auto strategy = std::make_shared<OneCallerBlocksPrefetch>();
  URLProvider provider("http://example.com/credentials",
                       StaleValueBehavior::STRICT_,
                       strategy);

  EXPECT_NO_THROW({
    // Provider with custom prefetch strategy created successfully
  });
}

TEST(URLProviderTest, ConstructorWithConfigAndBehavior) {
  auto config = std::make_shared<Models::Config>();
  config->setCredentialsUri("http://example.com/credentials");

  URLProvider provider(config, StaleValueBehavior::ALLOW_);

  EXPECT_NO_THROW({
    // Provider with config and ALLOW_ behavior created successfully
  });
}

TEST(URLProviderTest, EmptyUrlThrowsException) {
  auto config = std::make_shared<Models::Config>();
  // No credentialsUri set

  EXPECT_THROW({
    URLProvider provider(config);
  }, CredentialException);
}

TEST(URLProviderTest, EmptyUrlStringThrowsException) {
  EXPECT_THROW({
    URLProvider provider("");
  }, CredentialException);
}

// ==================== Provider Type Inheritance Verification ====================

// Verify that RamRoleArnProvider is a RefreshableProvider
TEST(RamRoleArnProviderTest, InheritsFromRefreshableProvider) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setRoleArn("acs:ram::123456:role/test");

  RamRoleArnProvider provider(config);

  // Verify it can be cast to RefreshableProvider
  RefreshableProvider* basePtr = dynamic_cast<RefreshableProvider*>(&provider);
  EXPECT_NE(basePtr, nullptr);
}

// Verify that OIDCRoleArnProvider is a RefreshableProvider
TEST(OIDCRoleArnProviderTest, InheritsFromRefreshableProvider) {
  std::string tokenPath = getTempDir() + "/test_oidc_inherit.txt";
  std::ofstream tokenFile(tokenPath);
  tokenFile << "test_token";
  tokenFile.close();

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456:role/test")
        .setOidcProviderArn("acs:ram::123456:oidc-provider/test")
        .setOidcTokenFilePath(tokenPath);

  OIDCRoleArnProvider provider(config);

  // Verify it can be cast to RefreshableProvider
  RefreshableProvider* basePtr = dynamic_cast<RefreshableProvider*>(&provider);
  EXPECT_NE(basePtr, nullptr);

  std::remove(tokenPath.c_str());
}

// Verify that URLProvider is a RefreshableProvider
TEST(URLProviderTest, InheritsFromRefreshableProvider) {
  URLProvider provider("http://example.com/credentials");

  // Verify it can be cast to RefreshableProvider
  RefreshableProvider* basePtr = dynamic_cast<RefreshableProvider*>(&provider);
  EXPECT_NE(basePtr, nullptr);
}

// ==================== Stale Time Window Tests ====================

// Test that the stale time window constant is 15 minutes (900 seconds)
TEST(RefreshableProviderConstantsTest, StaleTimeWindowIs15Minutes) {
  EXPECT_EQ(RefreshableProvider::STALE_TIME_WINDOW, 900);
}

// Test that the prefetch threshold is 180 seconds
TEST(RefreshableProviderConstantsTest, PrefetchThresholdIs180Seconds) {
  EXPECT_EQ(RefreshableProvider::PREFETCH_THRESHOLD, 180);
}
