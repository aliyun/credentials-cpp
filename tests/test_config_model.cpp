#include <gtest/gtest.h>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/Constant.hpp>

using namespace AlibabaCloud::Credential;

// ==================== Config Timeout and IMDSv1 Tests ====================

TEST(ConfigTest, SetAndGetTimeout) {
  Models::Config config;
  config.setTimeout(10000);
  
  EXPECT_TRUE(config.hasTimeout());
  EXPECT_EQ(10000, config.getTimeout());
}

TEST(ConfigTest, DefaultTimeout) {
  Models::Config config;
  // Default timeout should be 5000ms
  EXPECT_EQ(5000, config.getTimeout());
}

TEST(ConfigTest, SetAndGetConnectTimeout) {
  Models::Config config;
  config.setConnectTimeout(15000);
  
  EXPECT_TRUE(config.hasConnectTimeout());
  EXPECT_EQ(15000, config.getConnectTimeout());
}

TEST(ConfigTest, DefaultConnectTimeout) {
  Models::Config config;
  // Default connect timeout should be 10000ms
  EXPECT_EQ(10000, config.getConnectTimeout());
}

TEST(ConfigTest, SetAndGetDisableIMDSv1) {
  Models::Config config;
  config.setDisableIMDSv1(true);
  
  EXPECT_TRUE(config.hasDisableIMDSv1());
  EXPECT_TRUE(config.getDisableIMDSv1());
}

TEST(ConfigTest, DefaultDisableIMDSv1) {
  Models::Config config;
  // Default should be false (IMDSv1 enabled)
  EXPECT_FALSE(config.getDisableIMDSv1());
}

TEST(ConfigTest, SetDisableIMDSv1False) {
  Models::Config config;
  config.setDisableIMDSv1(false);
  
  EXPECT_TRUE(config.hasDisableIMDSv1());
  EXPECT_FALSE(config.getDisableIMDSv1());
}

TEST(ConfigTest, AllNewFieldsInToMap) {
  Models::Config config;
  config.setTimeout(8000)
        .setConnectTimeout(12000)
        .setDisableIMDSv1(true)
        .setAccessKeyId("test_ak")
        .setType("test_type");
  
  auto json = config.toMap();
  
  EXPECT_TRUE(json.contains("timeout"));
  EXPECT_TRUE(json.contains("connectTimeout"));
  EXPECT_TRUE(json.contains("disableIMDSv1"));
  EXPECT_EQ(8000, json["timeout"].get<int64_t>());
  EXPECT_EQ(12000, json["connectTimeout"].get<int64_t>());
  EXPECT_TRUE(json["disableIMDSv1"].get<bool>());
}

TEST(ConfigTest, AllNewFieldsFromMap) {
  Darabonba::Json json;
  json["timeout"] = 7000;
  json["connectTimeout"] = 14000;
  json["disableIMDSv1"] = true;
  json["accessKeyId"] = "from_json_ak";
  json["type"] = "from_json_type";
  
  Models::Config config;
  config.fromMap(json);
  
  EXPECT_EQ(7000, config.getTimeout());
  EXPECT_EQ(14000, config.getConnectTimeout());
  EXPECT_TRUE(config.getDisableIMDSv1());
  EXPECT_EQ("from_json_ak", config.getAccessKeyId());
  EXPECT_EQ("from_json_type", config.getType());
}

TEST(ConfigTest, EmptyCheckWithNewFields) {
  Models::Config config;
  // Config has default values, so it's not empty
  EXPECT_FALSE(config.empty());
  
  // Even with only new fields set
  Models::Config config2;
  config2.setTimeout(1000);
  // Still not empty because default values exist
  EXPECT_FALSE(config2.empty());
}

// ==================== Constant Tests for New Types ====================

TEST(ConstantTest, CloudSSOType) {
  EXPECT_EQ("sso", Constant::CLOUD_SSO);
}

TEST(ConstantTest, OAuthType) {
  EXPECT_EQ("oauth", Constant::OAUTH);
}

TEST(ConstantTest, AllTypeConstants) {
  EXPECT_EQ("access_key", Constant::ACCESS_KEY);
  EXPECT_EQ("sts", Constant::STS);
  EXPECT_EQ("ecs_ram_role", Constant::ECS_RAM_ROLE);
  EXPECT_EQ("ram_role_arn", Constant::RAM_ROLE_ARN);
  EXPECT_EQ("rsa_key_pair", Constant::RSA_KEY_PAIR);
  EXPECT_EQ("bearer", Constant::BEARER);
  EXPECT_EQ("oidc_role_arn", Constant::OIDC_ROLE_ARN);
  EXPECT_EQ("credentials_uri", Constant::URL_STS);
  EXPECT_EQ("sso", Constant::CLOUD_SSO);
  EXPECT_EQ("oauth", Constant::OAUTH);
}

// ==================== CredentialModel Extended Tests ====================

TEST(CredentialModelTest, ChainedSetter) {
  Models::CredentialModel model;
  model.setAccessKeyId("chain_ak")
       .setAccessKeySecret("chain_secret")
       .setSecurityToken("chain_token")
       .setBearerToken("chain_bearer")
       .setType("chain_type")
       .setProviderName("chain_provider");
  
  EXPECT_EQ("chain_ak", model.getAccessKeyId());
  EXPECT_EQ("chain_secret", model.getAccessKeySecret());
  EXPECT_EQ("chain_token", model.getSecurityToken());
  EXPECT_EQ("chain_bearer", model.getBearerToken());
  EXPECT_EQ("chain_type", model.getType());
  EXPECT_EQ("chain_provider", model.getProviderName());
}

TEST(CredentialModelTest, CopyConstructor) {
  Models::CredentialModel model1;
  model1.setAccessKeyId("copy_ak")
        .setAccessKeySecret("copy_secret");
  
  Models::CredentialModel model2(model1);
  
  EXPECT_EQ("copy_ak", model2.getAccessKeyId());
  EXPECT_EQ("copy_secret", model2.getAccessKeySecret());
}

TEST(CredentialModelTest, MoveConstructor) {
  Models::CredentialModel model1;
  model1.setAccessKeyId("move_ak")
        .setAccessKeySecret("move_secret");
  
  Models::CredentialModel model2(std::move(model1));
  
  EXPECT_EQ("move_ak", model2.getAccessKeyId());
  EXPECT_EQ("move_secret", model2.getAccessKeySecret());
}

TEST(CredentialModelTest, ConstructorFromJson) {
  Darabonba::Json json;
  json["accessKeyId"] = "json_ak";
  json["accessKeySecret"] = "json_secret";
  json["type"] = "json_type";
  
  Models::CredentialModel model(json);
  
  EXPECT_EQ("json_ak", model.getAccessKeyId());
  EXPECT_EQ("json_secret", model.getAccessKeySecret());
  EXPECT_EQ("json_type", model.getType());
}

TEST(CredentialModelTest, ValidateMethod) {
  Models::CredentialModel model;
  model.setAccessKeyId("validate_ak");
  
  // validate() should not throw for valid model
  EXPECT_NO_THROW(model.validate());
}

TEST(CredentialModelTest, AllFieldsInToMap) {
  Models::CredentialModel model;
  model.setAccessKeyId("all_ak")
       .setAccessKeySecret("all_secret")
       .setBearerToken("all_bearer")
       .setSecurityToken("all_token")
       .setType("all_type")
       .setProviderName("all_provider");
  
  auto json = model.toMap();
  
  EXPECT_EQ("all_ak", json["accessKeyId"].get<std::string>());
  EXPECT_EQ("all_secret", json["accessKeySecret"].get<std::string>());
  EXPECT_EQ("all_bearer", json["bearerToken"].get<std::string>());
  EXPECT_EQ("all_token", json["securityToken"].get<std::string>());
  EXPECT_EQ("all_type", json["type"].get<std::string>());
  EXPECT_EQ("all_provider", json["providerName"].get<std::string>());
}

TEST(CredentialModelTest, PartialFieldsInToMap) {
  Models::CredentialModel model;
  model.setAccessKeyId("partial_ak")
       .setType("partial_type");
  
  auto json = model.toMap();
  
  EXPECT_TRUE(json.contains("accessKeyId"));
  EXPECT_TRUE(json.contains("type"));
  // Fields not set should not appear or be null
  EXPECT_FALSE(json["accessKeySecret"].is_string());
}

// ==================== Config Extended Tests ====================

TEST(ConfigTest, AllFieldsChainedSetter) {
  Models::Config config;
  config.setAccessKeyId("full_ak")
        .setAccessKeySecret("full_secret")
        .setBearerToken("full_bearer")
        .setSecurityToken("full_token")
        .setType("full_type")
        .setRoleArn("acs:ram::123:role/test")
        .setRoleName("full_role")
        .setRoleSessionName("full_session")
        .setDurationSeconds(7200)
        .setPolicy("{}")
        .setRegionId("cn-beijing")
        .setStsEndpoint("sts.cn-beijing.aliyuncs.com")
        .setPublicKeyId("full_public")
        .setPrivateKeyFile("/path/to/key")
        .setOidcProviderArn("acs:ram::123:oidc")
        .setOidcTokenFilePath("/path/to/token")
        .setCredentialsURL("http://example.com")
        .setExternalId("ext_id")
        .setHost("host.example.com")
        .setProxy("http://proxy:8080")
        .setRoleSessionExpiration(3600)
        .setTimeout(6000)
        .setConnectTimeout(11000)
        .setDisableIMDSv1(true);
  
  EXPECT_EQ("full_ak", config.getAccessKeyId());
  EXPECT_EQ("full_secret", config.getAccessKeySecret());
  EXPECT_EQ("full_bearer", config.getBearerToken());
  EXPECT_EQ("full_token", config.getSecurityToken());
  EXPECT_EQ("full_type", config.getType());
  EXPECT_EQ("acs:ram::123:role/test", config.getRoleArn());
  EXPECT_EQ("full_role", config.getRoleName());
  EXPECT_EQ("full_session", config.getRoleSessionName());
  EXPECT_EQ(7200, config.getDurationSeconds());
  EXPECT_EQ("{}", config.getPolicy());
  EXPECT_EQ("cn-beijing", config.getRegionId());
  EXPECT_EQ("sts.cn-beijing.aliyuncs.com", config.getStsEndpoint());
  EXPECT_EQ("full_public", config.getPublicKeyId());
  EXPECT_EQ("/path/to/key", config.getPrivateKeyFile());
  EXPECT_EQ("acs:ram::123:oidc", config.getOidcProviderArn());
  EXPECT_EQ("/path/to/token", config.getOidcTokenFilePath());
  EXPECT_EQ("http://example.com", config.getCredentialsURL());
  EXPECT_EQ("ext_id", config.getExternalId());
  EXPECT_EQ("host.example.com", config.getHost());
  EXPECT_EQ("http://proxy:8080", config.getProxy());
  EXPECT_EQ(3600, config.getRoleSessionExpiration());
  EXPECT_EQ(6000, config.getTimeout());
  EXPECT_EQ(11000, config.getConnectTimeout());
  EXPECT_TRUE(config.getDisableIMDSv1());
}

TEST(ConfigTest, CopyAssignment) {
  Models::Config config1;
  config1.setAccessKeyId("assign_ak")
        .setAccessKeySecret("assign_secret");
  
  Models::Config config2;
  config2 = config1;
  
  EXPECT_EQ("assign_ak", config2.getAccessKeyId());
  EXPECT_EQ("assign_secret", config2.getAccessKeySecret());
}

TEST(ConfigTest, MoveAssignment) {
  Models::Config config1;
  config1.setAccessKeyId("move_assign_ak")
        .setAccessKeySecret("move_assign_secret");
  
  Models::Config config2;
  config2 = std::move(config1);
  
  EXPECT_EQ("move_assign_ak", config2.getAccessKeyId());
  EXPECT_EQ("move_assign_secret", config2.getAccessKeySecret());
}

TEST(ConfigTest, RValueSetters) {
  Models::Config config;
  std::string ak = "rvalue_ak";
  std::string secret = "rvalue_secret";
  std::string type = "rvalue_type";
  
  config.setAccessKeyId(std::move(ak))
        .setAccessKeySecret(std::move(secret))
        .setType(std::move(type));
  
  EXPECT_EQ("rvalue_ak", config.getAccessKeyId());
  EXPECT_EQ("rvalue_secret", config.getAccessKeySecret());
  EXPECT_EQ("rvalue_type", config.getType());
}
