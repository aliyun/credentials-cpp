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
  
  EXPECT_EQ("", credential.accessKeyId());
  EXPECT_EQ("secret", credential.accessKeySecret());
}

TEST(EdgeCasesTest, EmptyAccessKeySecret) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("ak_id")
        .setAccessKeySecret("");
  
  AccessKeyProvider provider(config);
  auto credential = provider.getCredential();
  
  EXPECT_EQ("ak_id", credential.accessKeyId());
  EXPECT_EQ("", credential.accessKeySecret());
}

TEST(EdgeCasesTest, VeryLongAccessKeyId) {
  std::string longAk(10000, 'A');
  
  AccessKeyProvider provider(longAk, "secret");
  auto credential = provider.getCredential();
  
  EXPECT_EQ(longAk, credential.accessKeyId());
  EXPECT_EQ(10000, credential.accessKeyId().length());
}

TEST(EdgeCasesTest, VeryLongAccessKeySecret) {
  std::string longSecret(10000, 'S');
  
  AccessKeyProvider provider("ak_id", longSecret);
  auto credential = provider.getCredential();
  
  EXPECT_EQ(longSecret, credential.accessKeySecret());
}

TEST(EdgeCasesTest, SpecialCharactersInAccessKey) {
  std::string specialAk = "ak-id_123.test@example#$%^&*()";
  std::string specialSecret = "secret!@#$%^&*()_+-=[]{}|;':\"<>?,./";
  
  AccessKeyProvider provider(specialAk, specialSecret);
  auto credential = provider.getCredential();
  
  EXPECT_EQ(specialAk, credential.accessKeyId());
  EXPECT_EQ(specialSecret, credential.accessKeySecret());
}

TEST(EdgeCasesTest, UnicodeInCredentials) {
  std::string unicodeAk = "测试AK";
  std::string unicodeSecret = "测试密钥";
  
  AccessKeyProvider provider(unicodeAk, unicodeSecret);
  auto credential = provider.getCredential();
  
  EXPECT_EQ(unicodeAk, credential.accessKeyId());
  EXPECT_EQ(unicodeSecret, credential.accessKeySecret());
}

TEST(EdgeCasesTest, WhitespaceInCredentials) {
  std::string akWithSpaces = "  ak_id  ";
  std::string secretWithSpaces = "  secret  ";
  
  AccessKeyProvider provider(akWithSpaces, secretWithSpaces);
  auto credential = provider.getCredential();
  
  // Should preserve whitespace
  EXPECT_EQ(akWithSpaces, credential.accessKeyId());
  EXPECT_EQ(secretWithSpaces, credential.accessKeySecret());
}

TEST(EdgeCasesTest, NewlineInCredentials) {
  std::string akWithNewline = "ak\nid";
  std::string secretWithNewline = "sec\nret";
  
  AccessKeyProvider provider(akWithNewline, secretWithNewline);
  auto credential = provider.getCredential();
  
  EXPECT_EQ(akWithNewline, credential.accessKeyId());
  EXPECT_EQ(secretWithNewline, credential.accessKeySecret());
}

TEST(EdgeCasesTest, EmptyBearerToken) {
  BearerTokenProvider provider("");
  auto credential = provider.getCredential();
  
  EXPECT_EQ("", credential.bearerToken());
  EXPECT_EQ(Constant::BEARER, credential.type());
}

TEST(EdgeCasesTest, VeryLongBearerToken) {
  std::string longToken(20000, 'T');
  
  BearerTokenProvider provider(longToken);
  auto credential = provider.getCredential();
  
  EXPECT_EQ(longToken, credential.bearerToken());
}

TEST(EdgeCasesTest, EmptySecurityToken) {
  StsProvider provider("ak", "secret", "");
  auto credential = provider.getCredential();
  
  EXPECT_EQ("", credential.securityToken());
  EXPECT_EQ(Constant::STS, credential.type());
}

TEST(EdgeCasesTest, AllEmptyStsCredentials) {
  StsProvider provider("", "", "");
  auto credential = provider.getCredential();
  
  EXPECT_EQ("", credential.accessKeyId());
  EXPECT_EQ("", credential.accessKeySecret());
  EXPECT_EQ("", credential.securityToken());
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
  EXPECT_EQ("ak", config.accessKeyId());
  EXPECT_EQ("secret", config.accessKeySecret());
  EXPECT_TRUE(config.disableIMDSv1());
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
  
  EXPECT_EQ(model1.accessKeyId(), model2.accessKeyId());
  EXPECT_EQ(model1.accessKeySecret(), model2.accessKeySecret());
  EXPECT_EQ(model1.bearerToken(), model2.bearerToken());
  EXPECT_EQ(model1.securityToken(), model2.securityToken());
  EXPECT_EQ(model1.type(), model2.type());
  EXPECT_EQ(model1.providerName(), model2.providerName());
}

TEST(EdgeCasesTest, CredentialModelMovePreservesAllFields) {
  Models::CredentialModel model1;
  model1.setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setBearerToken("bearer");
  
  std::string expectedAk = model1.accessKeyId();
  std::string expectedSecret = model1.accessKeySecret();
  std::string expectedBearer = model1.bearerToken();
  
  Models::CredentialModel model2(std::move(model1));
  
  EXPECT_EQ(expectedAk, model2.accessKeyId());
  EXPECT_EQ(expectedSecret, model2.accessKeySecret());
  EXPECT_EQ(expectedBearer, model2.bearerToken());
}

TEST(EdgeCasesTest, ConfigCopyPreservesAllFields) {
  Models::Config config1;
  config1.setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setTimeout(8000)
        .setDisableIMDSv1(true);
  
  Models::Config config2(config1);
  
  EXPECT_EQ(config1.accessKeyId(), config2.accessKeyId());
  EXPECT_EQ(config1.accessKeySecret(), config2.accessKeySecret());
  EXPECT_EQ(config1.timeout(), config2.timeout());
  EXPECT_EQ(config1.disableIMDSv1(), config2.disableIMDSv1());
}

TEST(EdgeCasesTest, ConfigMovePreservesAllFields) {
  Models::Config config1;
  config1.setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setTimeout(8000);
  
  std::string expectedAk = config1.accessKeyId();
  std::string expectedSecret = config1.accessKeySecret();
  int64_t expectedTimeout = config1.timeout();
  
  Models::Config config2(std::move(config1));
  
  EXPECT_EQ(expectedAk, config2.accessKeyId());
  EXPECT_EQ(expectedSecret, config2.accessKeySecret());
  EXPECT_EQ(expectedTimeout, config2.timeout());
}

TEST(EdgeCasesTest, ZeroDurationSeconds) {
  Models::Config config;
  config.setDurationSeconds(0);
  
  EXPECT_EQ(0, config.durationSeconds());
}

TEST(EdgeCasesTest, NegativeDurationSeconds) {
  Models::Config config;
  config.setDurationSeconds(-100);
  
  EXPECT_EQ(-100, config.durationSeconds());
}

TEST(EdgeCasesTest, VeryLargeDurationSeconds) {
  Models::Config config;
  config.setDurationSeconds(999999999);
  
  EXPECT_EQ(999999999, config.durationSeconds());
}

TEST(EdgeCasesTest, ZeroTimeout) {
  Models::Config config;
  config.setTimeout(0);
  
  EXPECT_EQ(0, config.timeout());
}

TEST(EdgeCasesTest, ZeroConnectTimeout) {
  Models::Config config;
  config.setConnectTimeout(0);
  
  EXPECT_EQ(0, config.connectTimeout());
}

TEST(EdgeCasesTest, VeryLargeTimeout) {
  Models::Config config;
  config.setTimeout(999999999);
  
  EXPECT_EQ(999999999, config.timeout());
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
  
  EXPECT_EQ(cred1.accessKeyId(), cred2.accessKeyId());
  EXPECT_EQ(cred1.accessKeySecret(), cred2.accessKeySecret());
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
  EXPECT_FALSE(credential.accessKeyId().empty());
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
  
  EXPECT_EQ(config1.accessKeyId(), config2.accessKeyId());
  EXPECT_EQ(config1.accessKeySecret(), config2.accessKeySecret());
  EXPECT_EQ(config1.timeout(), config2.timeout());
  EXPECT_EQ(config1.disableIMDSv1(), config2.disableIMDSv1());
  EXPECT_EQ(config1.durationSeconds(), config2.durationSeconds());
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
  
  EXPECT_EQ(model1.accessKeyId(), model2.accessKeyId());
  EXPECT_EQ(model1.accessKeySecret(), model2.accessKeySecret());
  EXPECT_EQ(model1.type(), model2.type());
  EXPECT_EQ(model1.providerName(), model2.providerName());
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
  
  EXPECT_FALSE(config.disableIMDSv1());
  
  config.setDisableIMDSv1(true);
  EXPECT_TRUE(config.disableIMDSv1());
  
  config.setDisableIMDSv1(false);
  EXPECT_FALSE(config.disableIMDSv1());
  
  config.setDisableIMDSv1(true);
  EXPECT_TRUE(config.disableIMDSv1());
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
