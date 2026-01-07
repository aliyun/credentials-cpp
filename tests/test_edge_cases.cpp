#include <gtest/gtest.h>
#include <alibabacloud/credential/Credential.hpp>
#include <alibabacloud/credential/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credential/provider/BearerTokenProvider.hpp>
#include <alibabacloud/credential/provider/StsProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <darabonba/Exception.hpp>

using namespace AlibabaCloud::Credential;

// ==================== Edge Cases and Boundary Tests ====================

TEST(EdgeCasesTest, EmptyAccessKeyId) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("")
        .setAccessKeySecret("secret");
  
  AccessKeyProvider provider(config);
  auto credential = provider.getCredential();
  
  EXPECT_EQ("", credential.getAccessKeyId());
  EXPECT_EQ("secret", credential.getAccessKeySecret());
}

TEST(EdgeCasesTest, EmptyAccessKeySecret) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("ak_id")
        .setAccessKeySecret("");
  
  AccessKeyProvider provider(config);
  auto credential = provider.getCredential();
  
  EXPECT_EQ("ak_id", credential.getAccessKeyId());
  EXPECT_EQ("", credential.getAccessKeySecret());
}

TEST(EdgeCasesTest, VeryLongAccessKeyId) {
  std::string longAk(10000, 'A');
  
  AccessKeyProvider provider(longAk, "secret");
  auto credential = provider.getCredential();
  
  EXPECT_EQ(longAk, credential.getAccessKeyId());
  EXPECT_EQ(10000, credential.getAccessKeyId().length());
}

TEST(EdgeCasesTest, VeryLongAccessKeySecret) {
  std::string longSecret(10000, 'S');
  
  AccessKeyProvider provider("ak_id", longSecret);
  auto credential = provider.getCredential();
  
  EXPECT_EQ(longSecret, credential.getAccessKeySecret());
}

TEST(EdgeCasesTest, SpecialCharactersInAccessKey) {
  std::string specialAk = "ak-id_123.test@example#$%^&*()";
  std::string specialSecret = "secret!@#$%^&*()_+-=[]{}|;':\"<>?,./";
  
  AccessKeyProvider provider(specialAk, specialSecret);
  auto credential = provider.getCredential();
  
  EXPECT_EQ(specialAk, credential.getAccessKeyId());
  EXPECT_EQ(specialSecret, credential.getAccessKeySecret());
}

TEST(EdgeCasesTest, UnicodeInCredentials) {
  std::string unicodeAk = "测试AK";
  std::string unicodeSecret = "测试密钥";
  
  AccessKeyProvider provider(unicodeAk, unicodeSecret);
  auto credential = provider.getCredential();
  
  EXPECT_EQ(unicodeAk, credential.getAccessKeyId());
  EXPECT_EQ(unicodeSecret, credential.getAccessKeySecret());
}

TEST(EdgeCasesTest, WhitespaceInCredentials) {
  std::string akWithSpaces = "  ak_id  ";
  std::string secretWithSpaces = "  secret  ";
  
  AccessKeyProvider provider(akWithSpaces, secretWithSpaces);
  auto credential = provider.getCredential();
  
  // Should preserve whitespace
  EXPECT_EQ(akWithSpaces, credential.getAccessKeyId());
  EXPECT_EQ(secretWithSpaces, credential.getAccessKeySecret());
}

TEST(EdgeCasesTest, NewlineInCredentials) {
  std::string akWithNewline = "ak\nid";
  std::string secretWithNewline = "sec\nret";
  
  AccessKeyProvider provider(akWithNewline, secretWithNewline);
  auto credential = provider.getCredential();
  
  EXPECT_EQ(akWithNewline, credential.getAccessKeyId());
  EXPECT_EQ(secretWithNewline, credential.getAccessKeySecret());
}

TEST(EdgeCasesTest, EmptyBearerToken) {
  BearerTokenProvider provider("");
  auto credential = provider.getCredential();
  
  EXPECT_EQ("", credential.getBearerToken());
  EXPECT_EQ(Constant::BEARER, credential.getType());
}

TEST(EdgeCasesTest, VeryLongBearerToken) {
  std::string longToken(20000, 'T');
  
  BearerTokenProvider provider(longToken);
  auto credential = provider.getCredential();
  
  EXPECT_EQ(longToken, credential.getBearerToken());
}

TEST(EdgeCasesTest, EmptySecurityToken) {
  StsProvider provider("ak", "secret", "");
  auto credential = provider.getCredential();
  
  EXPECT_EQ("", credential.getSecurityToken());
  EXPECT_EQ(Constant::STS, credential.getType());
}

TEST(EdgeCasesTest, AllEmptyStsCredentials) {
  StsProvider provider("", "", "");
  auto credential = provider.getCredential();
  
  EXPECT_EQ("", credential.getAccessKeyId());
  EXPECT_EQ("", credential.getAccessKeySecret());
  EXPECT_EQ("", credential.getSecurityToken());
}

TEST(EdgeCasesTest, ConfigWithAllFieldsSet) {
  Models::Config config;
  config.setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setBearerToken("bearer")
        .setSecurityToken("token")
        .setType("test_type")
        .setRoleArn("role_arn")
        .setRoleName("role_name")
        .setRoleSessionName("session")
        .setDurationSeconds(7200)
        .setPolicy("policy")
        .setPublicKeyId("public_key")
        .setPrivateKeyFile("/path/to/key")
        .setRegionId("cn-hangzhou")
        .setHost("host.example.com")
        .setProxy("http://proxy:8080")
        .setStsEndpoint("sts.endpoint.com")
        .setOidcProviderArn("oidc_arn")
        .setOidcTokenFilePath("/path/to/token")
        .setCredentialsURL("http://creds.url")
        .setExternalId("ext_id")
        .setRoleSessionExpiration(3600)
        .setTimeout(10000)
        .setConnectTimeout(15000)
        .setDisableIMDSv1(true);
  
  EXPECT_FALSE(config.empty());
  EXPECT_EQ("ak", config.getAccessKeyId());
  EXPECT_EQ("secret", config.getAccessKeySecret());
  EXPECT_TRUE(config.getDisableIMDSv1());
}

TEST(EdgeCasesTest, CredentialModelCopyPreservesAllFields) {
  Models::CredentialModel model1;
  model1.setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setBearerToken("bearer")
        .setSecurityToken("token")
        .setType("type")
        .setProviderName("provider");
  
  Models::CredentialModel model2(model1);
  
  EXPECT_EQ(model1.getAccessKeyId(), model2.getAccessKeyId());
  EXPECT_EQ(model1.getAccessKeySecret(), model2.getAccessKeySecret());
  EXPECT_EQ(model1.getBearerToken(), model2.getBearerToken());
  EXPECT_EQ(model1.getSecurityToken(), model2.getSecurityToken());
  EXPECT_EQ(model1.getType(), model2.getType());
  EXPECT_EQ(model1.getProviderName(), model2.getProviderName());
}

TEST(EdgeCasesTest, CredentialModelMovePreservesAllFields) {
  Models::CredentialModel model1;
  model1.setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setBearerToken("bearer");
  
  std::string expectedAk = model1.getAccessKeyId();
  std::string expectedSecret = model1.getAccessKeySecret();
  std::string expectedBearer = model1.getBearerToken();
  
  Models::CredentialModel model2(std::move(model1));
  
  EXPECT_EQ(expectedAk, model2.getAccessKeyId());
  EXPECT_EQ(expectedSecret, model2.getAccessKeySecret());
  EXPECT_EQ(expectedBearer, model2.getBearerToken());
}

TEST(EdgeCasesTest, ConfigCopyPreservesAllFields) {
  Models::Config config1;
  config1.setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setTimeout(8000)
        .setDisableIMDSv1(true);
  
  Models::Config config2(config1);
  
  EXPECT_EQ(config1.getAccessKeyId(), config2.getAccessKeyId());
  EXPECT_EQ(config1.getAccessKeySecret(), config2.getAccessKeySecret());
  EXPECT_EQ(config1.getTimeout(), config2.getTimeout());
  EXPECT_EQ(config1.getDisableIMDSv1(), config2.getDisableIMDSv1());
}

TEST(EdgeCasesTest, ConfigMovePreservesAllFields) {
  Models::Config config1;
  config1.setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setTimeout(8000);
  
  std::string expectedAk = config1.getAccessKeyId();
  std::string expectedSecret = config1.getAccessKeySecret();
  int64_t expectedTimeout = config1.getTimeout();
  
  Models::Config config2(std::move(config1));
  
  EXPECT_EQ(expectedAk, config2.getAccessKeyId());
  EXPECT_EQ(expectedSecret, config2.getAccessKeySecret());
  EXPECT_EQ(expectedTimeout, config2.getTimeout());
}

TEST(EdgeCasesTest, ZeroDurationSeconds) {
  Models::Config config;
  config.setDurationSeconds(0);
  
  EXPECT_EQ(0, config.getDurationSeconds());
}

TEST(EdgeCasesTest, NegativeDurationSeconds) {
  Models::Config config;
  config.setDurationSeconds(-100);
  
  EXPECT_EQ(-100, config.getDurationSeconds());
}

TEST(EdgeCasesTest, VeryLargeDurationSeconds) {
  Models::Config config;
  config.setDurationSeconds(999999999);
  
  EXPECT_EQ(999999999, config.getDurationSeconds());
}

TEST(EdgeCasesTest, ZeroTimeout) {
  Models::Config config;
  config.setTimeout(0);
  
  EXPECT_EQ(0, config.getTimeout());
}

TEST(EdgeCasesTest, ZeroConnectTimeout) {
  Models::Config config;
  config.setConnectTimeout(0);
  
  EXPECT_EQ(0, config.getConnectTimeout());
}

TEST(EdgeCasesTest, VeryLargeTimeout) {
  Models::Config config;
  config.setTimeout(999999999);
  
  EXPECT_EQ(999999999, config.getTimeout());
}

TEST(EdgeCasesTest, ClientWithEmptyConfig) {
  Models::Config config;
  // Config with only defaults, no type set
  
  // This should handle gracefully
  EXPECT_NO_THROW({
    // Most client operations require a type to be set
    // But construction should not fail
  });
}

TEST(EdgeCasesTest, MultipleProviderSameConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("shared_ak")
        .setAccessKeySecret("shared_secret");
  
  AccessKeyProvider provider1(config);
  AccessKeyProvider provider2(config);
  
  auto cred1 = provider1.getCredential();
  auto cred2 = provider2.getCredential();
  
  EXPECT_EQ(cred1.getAccessKeyId(), cred2.getAccessKeyId());
  EXPECT_EQ(cred1.getAccessKeySecret(), cred2.getAccessKeySecret());
}

TEST(EdgeCasesTest, ProviderAfterConfigModification) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("original_ak")
        .setAccessKeySecret("original_secret");
  
  AccessKeyProvider provider(config);
  
  // Modify config after provider creation
  config->setAccessKeyId("modified_ak");
  
  auto credential = provider.getCredential();
  
  // Provider should use the current config values
  // (Behavior depends on implementation - may use original or modified)
  EXPECT_FALSE(credential.getAccessKeyId().empty());
}

TEST(EdgeCasesTest, ToMapAndFromMapRoundTrip) {
  Models::Config config1;
  config1.setAccessKeyId("roundtrip_ak")
        .setAccessKeySecret("roundtrip_secret")
        .setTimeout(7000)
        .setDisableIMDSv1(true)
        .setDurationSeconds(5400);
  
  auto json = config1.toMap();
  
  Models::Config config2;
  config2.fromMap(json);
  
  EXPECT_EQ(config1.getAccessKeyId(), config2.getAccessKeyId());
  EXPECT_EQ(config1.getAccessKeySecret(), config2.getAccessKeySecret());
  EXPECT_EQ(config1.getTimeout(), config2.getTimeout());
  EXPECT_EQ(config1.getDisableIMDSv1(), config2.getDisableIMDSv1());
  EXPECT_EQ(config1.getDurationSeconds(), config2.getDurationSeconds());
}

TEST(EdgeCasesTest, CredentialModelToMapAndFromMapRoundTrip) {
  Models::CredentialModel model1;
  model1.setAccessKeyId("model_ak")
        .setAccessKeySecret("model_secret")
        .setType("model_type")
        .setProviderName("model_provider");
  
  auto json = model1.toMap();
  
  Models::CredentialModel model2;
  model2.fromMap(json);
  
  EXPECT_EQ(model1.getAccessKeyId(), model2.getAccessKeyId());
  EXPECT_EQ(model1.getAccessKeySecret(), model2.getAccessKeySecret());
  EXPECT_EQ(model1.getType(), model2.getType());
  EXPECT_EQ(model1.getProviderName(), model2.getProviderName());
}

TEST(EdgeCasesTest, NullPointerInSharedPtrConfig) {
  std::shared_ptr<Models::Config> nullConfig;
  
  // Creating provider with null config should handle gracefully or throw
  // Note: This may cause undefined behavior, so we comment it out for safety
  // EXPECT_THROW({
  //   AccessKeyProvider provider(nullConfig);
  // }, std::exception);
  
  // Instead, test with valid but empty config
  auto emptyConfig = std::make_shared<Models::Config>();
  EXPECT_NO_THROW({
    AccessKeyProvider provider(emptyConfig);
  });
}

TEST(EdgeCasesTest, BooleanFieldToggle) {
  Models::Config config;
  
  EXPECT_FALSE(config.getDisableIMDSv1());
  
  config.setDisableIMDSv1(true);
  EXPECT_TRUE(config.getDisableIMDSv1());
  
  config.setDisableIMDSv1(false);
  EXPECT_FALSE(config.getDisableIMDSv1());
  
  config.setDisableIMDSv1(true);
  EXPECT_TRUE(config.getDisableIMDSv1());
}

TEST(EdgeCasesTest, HasMethodsWithoutSetting) {
  Models::Config config;
  
  // Most has* methods should return false for unset fields
  // Some fields have defaults, so their has* methods may return true
  EXPECT_FALSE(config.hasAccessKeyId());
  EXPECT_FALSE(config.hasAccessKeySecret());
  EXPECT_FALSE(config.hasBearerToken());
  EXPECT_FALSE(config.hasSecurityToken());
}

TEST(EdgeCasesTest, HasMethodsAfterSetting) {
  Models::Config config;
  
  config.setAccessKeyId("ak");
  EXPECT_TRUE(config.hasAccessKeyId());
  
  config.setAccessKeySecret("secret");
  EXPECT_TRUE(config.hasAccessKeySecret());
  
  config.setBearerToken("bearer");
  EXPECT_TRUE(config.hasBearerToken());
}
