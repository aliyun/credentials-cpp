#include <gtest/gtest.h>
#include <alibabacloud/credential/Credential.hpp>
#include <alibabacloud/credential/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credential/provider/BearerTokenProvider.hpp>
#include <alibabacloud/credential/provider/StsProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>

using namespace AlibabaCloud::Credential;

// ==================== Model Tests ====================

TEST(CredentialModelTest, DefaultConstructor) {
  Models::CredentialModel model;
  EXPECT_TRUE(model.empty());
  EXPECT_FALSE(model.hasAccessKeyId());
  EXPECT_FALSE(model.hasAccessKeySecret());
  EXPECT_FALSE(model.hasBearerToken());
  EXPECT_FALSE(model.hasSecurityToken());
  EXPECT_FALSE(model.hasType());
  EXPECT_FALSE(model.hasProviderName());
}

TEST(CredentialModelTest, SetAndGetAccessKey) {
  Models::CredentialModel model;
  model.setAccessKeyId("test_ak_id");
  model.setAccessKeySecret("test_ak_secret");
  
  EXPECT_TRUE(model.hasAccessKeyId());
  EXPECT_TRUE(model.hasAccessKeySecret());
  EXPECT_EQ("test_ak_id", model.getAccessKeyId());
  EXPECT_EQ("test_ak_secret", model.getAccessKeySecret());
  EXPECT_FALSE(model.empty());
}

TEST(CredentialModelTest, SetAndGetBearerToken) {
  Models::CredentialModel model;
  model.setBearerToken("test_bearer_token");
  
  EXPECT_TRUE(model.hasBearerToken());
  EXPECT_EQ("test_bearer_token", model.getBearerToken());
  EXPECT_FALSE(model.empty());
}

TEST(CredentialModelTest, SetAndGetSecurityToken) {
  Models::CredentialModel model;
  model.setSecurityToken("test_security_token");
  
  EXPECT_TRUE(model.hasSecurityToken());
  EXPECT_EQ("test_security_token", model.getSecurityToken());
  EXPECT_FALSE(model.empty());
}

TEST(CredentialModelTest, SetAndGetType) {
  Models::CredentialModel model;
  model.setType("access_key");
  
  EXPECT_TRUE(model.hasType());
  EXPECT_EQ("access_key", model.getType());
}

TEST(CredentialModelTest, SetAndGetProviderName) {
  Models::CredentialModel model;
  model.setProviderName("test_provider");
  
  EXPECT_TRUE(model.hasProviderName());
  EXPECT_EQ("test_provider", model.getProviderName());
}

TEST(CredentialModelTest, RValueSet) {
  Models::CredentialModel model;
  std::string akId = "rvalue_ak_id";
  std::string akSecret = "rvalue_ak_secret";
  
  model.setAccessKeyId(std::move(akId));
  model.setAccessKeySecret(std::move(akSecret));
  
  EXPECT_EQ("rvalue_ak_id", model.getAccessKeyId());
  EXPECT_EQ("rvalue_ak_secret", model.getAccessKeySecret());
}

TEST(CredentialModelTest, ToMapAndFromMap) {
  Models::CredentialModel model;
  model.setAccessKeyId("test_id")
       .setAccessKeySecret("test_secret")
       .setType("test_type");
  
  auto json = model.toMap();
  
  Models::CredentialModel newModel;
  newModel.fromMap(json);
  
  EXPECT_EQ("test_id", newModel.getAccessKeyId());
  EXPECT_EQ("test_secret", newModel.getAccessKeySecret());
  EXPECT_EQ("test_type", newModel.getType());
}

// ==================== Config Tests ====================

TEST(ConfigTest, DefaultConstructor) {
  Models::Config config;
  EXPECT_FALSE(config.empty());
  EXPECT_EQ(3600, config.getDurationSeconds());
  EXPECT_EQ("cn-hangzhou", config.getRegionId());
  EXPECT_EQ("defaultSessionName", config.getRoleSessionName());
  EXPECT_EQ("sts.aliyuncs.com", config.getStsEndpoint());
}

TEST(ConfigTest, SetAndGetAccessKey) {
  Models::Config config;
  config.setAccessKeyId("config_ak_id")
        .setAccessKeySecret("config_ak_secret");
  
  EXPECT_TRUE(config.hasAccessKeyId());
  EXPECT_TRUE(config.hasAccessKeySecret());
  EXPECT_EQ("config_ak_id", config.getAccessKeyId());
  EXPECT_EQ("config_ak_secret", config.getAccessKeySecret());
}

TEST(ConfigTest, SetAndGetType) {
  Models::Config config;
  config.setType("access_key");
  
  EXPECT_TRUE(config.hasType());
  EXPECT_EQ("access_key", config.getType());
}

TEST(ConfigTest, SetAndGetBearerToken) {
  Models::Config config;
  config.setBearerToken("bearer_token_value");
  
  EXPECT_TRUE(config.hasBearerToken());
  EXPECT_EQ("bearer_token_value", config.getBearerToken());
}

TEST(ConfigTest, SetAndGetSecurityToken) {
  Models::Config config;
  config.setSecurityToken("security_token_value");
  
  EXPECT_TRUE(config.hasSecurityToken());
  EXPECT_EQ("security_token_value", config.getSecurityToken());
}

TEST(ConfigTest, SetAndGetRoleArn) {
  Models::Config config;
  config.setRoleArn("acs:ram::123456:role/testrole");
  
  EXPECT_TRUE(config.hasRoleArn());
  EXPECT_EQ("acs:ram::123456:role/testrole", config.getRoleArn());
}

TEST(ConfigTest, SetAndGetRoleName) {
  Models::Config config;
  config.setRoleName("test_role_name");
  
  EXPECT_TRUE(config.hasRoleName());
  EXPECT_EQ("test_role_name", config.getRoleName());
}

TEST(ConfigTest, SetAndGetRoleSessionName) {
  Models::Config config;
  config.setRoleSessionName("custom_session_name");
  
  EXPECT_EQ("custom_session_name", config.getRoleSessionName());
}

TEST(ConfigTest, SetAndGetDurationSeconds) {
  Models::Config config;
  config.setDurationSeconds(7200);
  
  EXPECT_TRUE(config.hasDurationSeconds());
  EXPECT_EQ(7200, config.getDurationSeconds());
}

TEST(ConfigTest, SetAndGetPolicy) {
  Models::Config config;
  config.setPolicy("{\"Version\":\"1\"}");
  
  EXPECT_TRUE(config.hasPolicy());
  EXPECT_EQ("{\"Version\":\"1\"}", config.getPolicy());
}

TEST(ConfigTest, SetAndGetPublicKeyId) {
  Models::Config config;
  config.setPublicKeyId("public_key_id_123");
  
  EXPECT_TRUE(config.hasPublicKeyId());
  EXPECT_EQ("public_key_id_123", config.getPublicKeyId());
}

TEST(ConfigTest, SetAndGetPrivateKeyFile) {
  Models::Config config;
  config.setPrivateKeyFile("/path/to/private_key.pem");
  
  EXPECT_TRUE(config.hasPrivateKeyFile());
  EXPECT_EQ("/path/to/private_key.pem", config.getPrivateKeyFile());
}

TEST(ConfigTest, SetAndGetRegionId) {
  Models::Config config;
  config.setRegionId("cn-beijing");
  
  EXPECT_TRUE(config.hasRegionId());
  EXPECT_EQ("cn-beijing", config.getRegionId());
}

TEST(ConfigTest, SetAndGetHost) {
  Models::Config config;
  config.setHost("ecs.aliyuncs.com");
  
  EXPECT_TRUE(config.hasHost());
  EXPECT_EQ("ecs.aliyuncs.com", config.getHost());
}

TEST(ConfigTest, SetAndGetProxy) {
  Models::Config config;
  config.setProxy("http://proxy.example.com:8080");
  
  EXPECT_TRUE(config.hasProxy());
  EXPECT_EQ("http://proxy.example.com:8080", config.getProxy());
}

TEST(ConfigTest, SetAndGetStsEndpoint) {
  Models::Config config;
  config.setStsEndpoint("sts.cn-beijing.aliyuncs.com");
  
  EXPECT_TRUE(config.hasStsEndpoint());
  EXPECT_EQ("sts.cn-beijing.aliyuncs.com", config.getStsEndpoint());
}

TEST(ConfigTest, SetAndGetOidcProviderArn) {
  Models::Config config;
  config.setOidcProviderArn("acs:ram::123456:oidc-provider/test");
  
  EXPECT_TRUE(config.hasOidcProviderArn());
  EXPECT_EQ("acs:ram::123456:oidc-provider/test", config.getOidcProviderArn());
}

TEST(ConfigTest, SetAndGetOidcTokenFilePath) {
  Models::Config config;
  config.setOidcTokenFilePath("/path/to/oidc/token");
  
  EXPECT_TRUE(config.hasOidcTokenFilePath());
  EXPECT_EQ("/path/to/oidc/token", config.getOidcTokenFilePath());
}

TEST(ConfigTest, SetAndGetCredentialsURL) {
  Models::Config config;
  config.setCredentialsURL("http://credentials.example.com");
  
  EXPECT_TRUE(config.hasCredentialsURL());
  EXPECT_EQ("http://credentials.example.com", config.getCredentialsURL());
}

TEST(ConfigTest, SetAndGetExternalId) {
  Models::Config config;
  config.setExternalId("external_id_123");
  
  EXPECT_TRUE(config.hasExternalId());
  EXPECT_EQ("external_id_123", config.getExternalId());
}

TEST(ConfigTest, SetAndGetRoleSessionExpiration) {
  Models::Config config;
  config.setRoleSessionExpiration(7200);
  
  EXPECT_TRUE(config.hasRoleSessionExpiration());
  EXPECT_EQ(7200, config.getRoleSessionExpiration());
}

TEST(ConfigTest, CopyConstructor) {
  Models::Config config1;
  config1.setAccessKeyId("copy_test_id")
         .setAccessKeySecret("copy_test_secret");
  
  Models::Config config2(config1);
  
  EXPECT_EQ("copy_test_id", config2.getAccessKeyId());
  EXPECT_EQ("copy_test_secret", config2.getAccessKeySecret());
}

TEST(ConfigTest, MoveConstructor) {
  Models::Config config1;
  config1.setAccessKeyId("move_test_id")
         .setAccessKeySecret("move_test_secret");
  
  Models::Config config2(std::move(config1));
  
  EXPECT_EQ("move_test_id", config2.getAccessKeyId());
  EXPECT_EQ("move_test_secret", config2.getAccessKeySecret());
}

TEST(ConfigTest, ToMapAndFromMap) {
  Models::Config config;
  config.setAccessKeyId("json_test_id")
        .setAccessKeySecret("json_test_secret")
        .setType("access_key")
        .setDurationSeconds(5400);
  
  auto json = config.toMap();
  
  Models::Config newConfig;
  newConfig.fromMap(json);
  
  EXPECT_EQ("json_test_id", newConfig.getAccessKeyId());
  EXPECT_EQ("json_test_secret", newConfig.getAccessKeySecret());
  EXPECT_EQ("access_key", newConfig.getType());
  EXPECT_EQ(5400, newConfig.getDurationSeconds());
}

// ==================== Provider Tests ====================

TEST(AccessKeyProviderTest, ConstructorWithConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("test_ak_id")
        .setAccessKeySecret("test_ak_secret");
  
  AccessKeyProvider provider(config);
  auto credential = provider.getCredential();
  
  EXPECT_EQ("test_ak_id", credential.getAccessKeyId());
  EXPECT_EQ("test_ak_secret", credential.getAccessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
}

TEST(AccessKeyProviderTest, ConstructorWithParameters) {
  AccessKeyProvider provider("param_ak_id", "param_ak_secret");
  auto credential = provider.getCredential();
  
  EXPECT_EQ("param_ak_id", credential.getAccessKeyId());
  EXPECT_EQ("param_ak_secret", credential.getAccessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
}

TEST(AccessKeyProviderTest, GetCredentialConst) {
  const AccessKeyProvider provider("const_ak_id", "const_ak_secret");
  const auto &credential = provider.getCredential();
  
  EXPECT_EQ("const_ak_id", credential.getAccessKeyId());
  EXPECT_EQ("const_ak_secret", credential.getAccessKeySecret());
}

TEST(BearerTokenProviderTest, ConstructorWithConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setBearerToken("test_bearer_token");
  
  BearerTokenProvider provider(config);
  auto credential = provider.getCredential();
  
  EXPECT_EQ("test_bearer_token", credential.getBearerToken());
  EXPECT_EQ(Constant::BEARER, credential.getType());
}

TEST(BearerTokenProviderTest, ConstructorWithParameter) {
  BearerTokenProvider provider("param_bearer_token");
  auto credential = provider.getCredential();
  
  EXPECT_EQ("param_bearer_token", credential.getBearerToken());
  EXPECT_EQ(Constant::BEARER, credential.getType());
}

TEST(StsProviderTest, ConstructorWithConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setAccessKeyId("sts_ak_id")
        .setAccessKeySecret("sts_ak_secret")
        .setSecurityToken("sts_token");
  
  StsProvider provider(config);
  auto credential = provider.getCredential();
  
  EXPECT_EQ("sts_ak_id", credential.getAccessKeyId());
  EXPECT_EQ("sts_ak_secret", credential.getAccessKeySecret());
  EXPECT_EQ("sts_token", credential.getSecurityToken());
  EXPECT_EQ(Constant::STS, credential.getType());
}

TEST(StsProviderTest, ConstructorWithParameters) {
  StsProvider provider("param_sts_ak", "param_sts_secret", "param_sts_token");
  auto credential = provider.getCredential();
  
  EXPECT_EQ("param_sts_ak", credential.getAccessKeyId());
  EXPECT_EQ("param_sts_secret", credential.getAccessKeySecret());
  EXPECT_EQ("param_sts_token", credential.getSecurityToken());
  EXPECT_EQ(Constant::STS, credential.getType());
}

// ==================== Client Tests ====================

// Note: DefaultConstructor test is skipped because it triggers DefaultProvider
// which may require network access or environment configuration

TEST(ClientTest, AccessKeyClient) {
  Models::Config config;
  config.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("client_ak_id")
        .setAccessKeySecret("client_ak_secret");
  
  Client client(config);
  
  EXPECT_FALSE(client.empty());
  EXPECT_EQ("client_ak_id", client.getAccessKeyId());
  EXPECT_EQ("client_ak_secret", client.getAccessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, client.getType());
}

TEST(ClientTest, StsClient) {
  Models::Config config;
  config.setType(Constant::STS)
        .setAccessKeyId("sts_client_ak")
        .setAccessKeySecret("sts_client_secret")
        .setSecurityToken("sts_client_token");
  
  Client client(config);
  
  EXPECT_EQ("sts_client_ak", client.getAccessKeyId());
  EXPECT_EQ("sts_client_secret", client.getAccessKeySecret());
  EXPECT_EQ("sts_client_token", client.getSecurityToken());
  EXPECT_EQ(Constant::STS, client.getType());
}

TEST(ClientTest, BearerTokenClient) {
  Models::Config config;
  config.setType(Constant::BEARER)
        .setBearerToken("client_bearer_token");
  
  Client client(config);
  
  EXPECT_EQ("client_bearer_token", client.getBearerToken());
  EXPECT_EQ(Constant::BEARER, client.getType());
}

TEST(ClientTest, ConstructorWithSharedPtrConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setType(Constant::ACCESS_KEY)
        .setAccessKeyId("shared_ak_id")
        .setAccessKeySecret("shared_ak_secret");
  
  Client client(config);
  
  EXPECT_EQ("shared_ak_id", client.getAccessKeyId());
  EXPECT_EQ("shared_ak_secret", client.getAccessKeySecret());
}

TEST(ClientTest, ConstructorWithMoveConfig) {
  Models::Config config;
  config.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("move_ak_id")
        .setAccessKeySecret("move_ak_secret");
  
  Client client(std::move(config));
  
  EXPECT_EQ("move_ak_id", client.getAccessKeyId());
  EXPECT_EQ("move_ak_secret", client.getAccessKeySecret());
}

TEST(ClientTest, GetCredential) {
  Models::Config config;
  config.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("cred_ak_id")
        .setAccessKeySecret("cred_ak_secret");
  
  Client client(config);
  auto credential = client.getCredential();
  
  EXPECT_EQ("cred_ak_id", credential.getAccessKeyId());
  EXPECT_EQ("cred_ak_secret", credential.getAccessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
}

// ==================== Constant Tests ====================

TEST(ConstantTest, TypeConstants) {
  EXPECT_EQ("access_key", Constant::ACCESS_KEY);
  EXPECT_EQ("sts", Constant::STS);
  EXPECT_EQ("ecs_ram_role", Constant::ECS_RAM_ROLE);
  EXPECT_EQ("ram_role_arn", Constant::RAM_ROLE_ARN);
  EXPECT_EQ("rsa_key_pair", Constant::RSA_KEY_PAIR);
  EXPECT_EQ("bearer", Constant::BEARER);
  EXPECT_EQ("oidc_role_arn", Constant::OIDC_ROLE_ARN);
  EXPECT_EQ("credentials_uri", Constant::URL_STS);
}

TEST(ConstantTest, IniConstants) {
  EXPECT_EQ("access_key_id", Constant::INI_ACCESS_KEY_ID);
  EXPECT_EQ("access_key_secret", Constant::INI_ACCESS_KEY_SECRET);
  EXPECT_EQ("type", Constant::INI_TYPE);
  EXPECT_EQ("public_key_id", Constant::INI_PUBLIC_KEY_ID);
  EXPECT_EQ("private_key_file", Constant::INI_PRIVATE_KEY_FILE);
  EXPECT_EQ("private_key", Constant::INI_PRIVATE_KEY);
  EXPECT_EQ("role_name", Constant::INI_ROLE_NAME);
  EXPECT_EQ("role_session_name", Constant::INI_ROLE_SESSION_NAME);
  EXPECT_EQ("role_arn", Constant::INI_ROLE_ARN);
  EXPECT_EQ("policy", Constant::INI_POLICY);
  EXPECT_EQ("oidc_provider_arn", Constant::INI_OIDC_PROVIDER_ARN);
  EXPECT_EQ("oidc_token_file_path", Constant::INI_OIDC_TOKEN_FILE_PATH);
  EXPECT_EQ("enable", Constant::INI_ENABLE);
}