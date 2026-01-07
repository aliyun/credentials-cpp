#include <gtest/gtest.h>
#include <alibabacloud/credential/Credential.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/provider/AccessKeyProvider.hpp>
#include <fstream>
#include <cstdio>

using namespace AlibabaCloud::Credential;

// ==================== Client Tests ====================

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

// Test ECS RAM Role client
TEST(ClientTest, EcsRamRoleClient) {
  Models::Config config;
  config.setType(Constant::ECS_RAM_ROLE)
        .setRoleName("test_ecs_role");
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::ECS_RAM_ROLE, client.getType());
  });
}

// Test RAM Role ARN client
TEST(ClientTest, RamRoleArnClient) {
  Models::Config config;
  config.setType(Constant::RAM_ROLE_ARN)
        .setAccessKeyId("ram_ak")
        .setAccessKeySecret("ram_secret")
        .setRoleArn("acs:ram::123456:role/test")
        .setRoleSessionName("session");
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::RAM_ROLE_ARN, client.getType());
  });
}

// Test RSA Key Pair client
TEST(ClientTest, RsaKeyPairClient) {
  Models::Config config;
  config.setType(Constant::RSA_KEY_PAIR)
        .setPublicKeyId("public_key_id")
        .setPrivateKeyFile("/path/to/key.pem");
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::RSA_KEY_PAIR, client.getType());
  });
}

// Test OIDC Role ARN client
TEST(ClientTest, OIDCRoleArnClient) {
  // Create temporary OIDC token file
  std::string tokenPath = "/tmp/test_oidc_client_token.txt";
  std::ofstream tokenFile(tokenPath);
  tokenFile << "test_oidc_token";
  tokenFile.close();
  
  Models::Config config;
  config.setType(Constant::OIDC_ROLE_ARN)
        .setRoleArn("acs:ram::123456:role/test")
        .setOidcProviderArn("acs:ram::123456:oidc-provider/test")
        .setOidcTokenFilePath(tokenPath)
        .setRoleSessionName("session");
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::OIDC_ROLE_ARN, client.getType());
  });
  
  std::remove(tokenPath.c_str());
}

// Test URL STS client
TEST(ClientTest, URLSTSClient) {
  Models::Config config;
  config.setType(Constant::URL_STS)
        .setCredentialsURL("http://localhost:8080/credentials");
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::URL_STS, client.getType());
  });
}

// Test Cloud SSO client
TEST(ClientTest, CloudSSOClient) {
  Models::Config config;
  config.setType(Constant::CLOUD_SSO)
        .setRoleName("sso_role")
        .setRegionId("cn-hangzhou");
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::CLOUD_SSO, client.getType());
  });
}

// Test OAuth client
TEST(ClientTest, OAuthClient) {
  Models::Config config;
  config.setType(Constant::OAUTH)
        .setAccessKeyId("oauth_client_id")
        .setAccessKeySecret("oauth_client_secret")
        .setStsEndpoint("https://oauth.example.com/token");
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::OAUTH, client.getType());
  });
}

// Test client with timeout configuration
TEST(ClientTest, ClientWithTimeout) {
  Models::Config config;
  config.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setTimeout(8000)
        .setConnectTimeout(12000);
  
  Client client(config);
  
  EXPECT_EQ("ak", client.getAccessKeyId());
}

// Test client with IMDSv1 disabled
TEST(ClientTest, ClientWithDisableIMDSv1) {
  Models::Config config;
  config.setType(Constant::ECS_RAM_ROLE)
        .setRoleName("test_role")
        .setDisableIMDSv1(true);
  
  EXPECT_NO_THROW({
    Client client(config);
  });
}

// Test client with proxy
TEST(ClientTest, ClientWithProxy) {
  Models::Config config;
  config.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setProxy("http://proxy.example.com:8080");
  
  EXPECT_NO_THROW({
    Client client(config);
  });
}

// ==================== Additional Comprehensive Tests ====================

// Test constructor with null config
TEST(ClientTest, ConstructorWithNullConfig) {
  std::shared_ptr<Models::Config> nullConfig = nullptr;
  
  EXPECT_NO_THROW({
    Client client(nullConfig);
    EXPECT_FALSE(client.empty());
  });
}

// Test constructor with provider
TEST(ClientTest, ConstructorWithProvider) {
  auto provider = std::make_shared<AccessKeyProvider>("provider_ak", "provider_secret");
  
  Client client(provider);
  
  EXPECT_FALSE(client.empty());
  EXPECT_EQ("provider_ak", client.getAccessKeyId());
  EXPECT_EQ("provider_secret", client.getAccessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, client.getType());
}

// Test empty state with null provider
TEST(ClientTest, EmptyWithNullProvider) {
  auto provider = std::shared_ptr<Provider>(nullptr);
  Client client(provider);
  
  EXPECT_TRUE(client.empty());
}

// Test getCredential returns a copy
TEST(ClientTest, GetCredentialReturnsACopy) {
  Models::Config config;
  config.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("copy_ak")
        .setAccessKeySecret("copy_secret");
  
  Client client(config);
  
  auto credential1 = client.getCredential();
  auto credential2 = client.getCredential();
  
  EXPECT_EQ(credential1.getAccessKeyId(), credential2.getAccessKeyId());
  EXPECT_EQ(credential1.getAccessKeySecret(), credential2.getAccessKeySecret());
}

// Test unknown type defaults to DefaultProvider
TEST(ClientTest, UnknownTypeDefaultsToDefaultProvider) {
  Models::Config config;
  config.setType("unknown_type");
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_FALSE(client.empty());
  });
}

// Test empty credentials
TEST(ClientTest, EmptyCredentials) {
  Models::Config config;
  config.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("")
        .setAccessKeySecret("");
  
  Client client(config);
  
  EXPECT_EQ("", client.getAccessKeyId());
  EXPECT_EQ("", client.getAccessKeySecret());
}

// Test special characters in credentials
TEST(ClientTest, SpecialCharactersInCredentials) {
  std::string specialAk = "AK+/=!@#$%^&*()";
  std::string specialSecret = "Secret+/=!@#$%^&*()";
  
  Models::Config config;
  config.setType(Constant::ACCESS_KEY)
        .setAccessKeyId(specialAk)
        .setAccessKeySecret(specialSecret);
  
  Client client(config);
  
  EXPECT_EQ(specialAk, client.getAccessKeyId());
  EXPECT_EQ(specialSecret, client.getAccessKeySecret());
}

// Test multiple clients with different types
TEST(ClientTest, MultipleClientsDifferentTypes) {
  Models::Config config1;
  config1.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("client1_ak")
        .setAccessKeySecret("client1_secret");
  
  Models::Config config2;
  config2.setType(Constant::BEARER)
        .setBearerToken("client2_token");
  
  Models::Config config3;
  config3.setType(Constant::STS)
        .setAccessKeyId("client3_ak")
        .setAccessKeySecret("client3_secret")
        .setSecurityToken("client3_token");
  
  Client client1(config1);
  Client client2(config2);
  Client client3(config3);
  
  EXPECT_EQ(Constant::ACCESS_KEY, client1.getType());
  EXPECT_EQ(Constant::BEARER, client2.getType());
  EXPECT_EQ(Constant::STS, client3.getType());
  
  EXPECT_EQ("client1_ak", client1.getAccessKeyId());
  EXPECT_EQ("client2_token", client2.getBearerToken());
  EXPECT_EQ("client3_token", client3.getSecurityToken());
}

// Test client with all timeout options
TEST(ClientTest, ClientWithAllTimeoutOptions) {
  Models::Config config;
  config.setType(Constant::RAM_ROLE_ARN)
        .setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setRoleArn("acs:ram::123456789:role/test")
        .setRoleSessionName("session")
        .setTimeout(8000)
        .setConnectTimeout(12000)
        .setDurationSeconds(7200);
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::RAM_ROLE_ARN, client.getType());
  });
}

// Test client with policy
TEST(ClientTest, ClientWithPolicy) {
  Models::Config config;
  config.setType(Constant::RAM_ROLE_ARN)
        .setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setRoleArn("acs:ram::123456789:role/test")
        .setRoleSessionName("session")
        .setPolicy("{\"Version\":\"1\",\"Statement\":[]}");
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::RAM_ROLE_ARN, client.getType());
  });
}

// Test client with custom STS endpoint
TEST(ClientTest, ClientWithCustomStsEndpoint) {
  Models::Config config;
  config.setType(Constant::RAM_ROLE_ARN)
        .setAccessKeyId("ak")
        .setAccessKeySecret("secret")
        .setRoleArn("acs:ram::123456789:role/test")
        .setRoleSessionName("session")
        .setStsEndpoint("sts.cn-beijing.aliyuncs.com");
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::RAM_ROLE_ARN, client.getType());
  });
}

// Test client with VPC enabled
TEST(ClientTest, ClientWithVpcEnabled) {
  Models::Config config;
  config.setType(Constant::ECS_RAM_ROLE)
        .setRoleName("test_role")
        .setEnableVpc(true);
  
  EXPECT_NO_THROW({
    Client client(config);
    EXPECT_EQ(Constant::ECS_RAM_ROLE, client.getType());
  });
}

// Test copy constructor
TEST(ClientTest, CopyConstructor) {
  Models::Config config;
  config.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("copy_ak")
        .setAccessKeySecret("copy_secret");
  
  Client client1(config);
  Client client2(client1);
  
  EXPECT_EQ(client1.getAccessKeyId(), client2.getAccessKeyId());
  EXPECT_EQ(client1.getAccessKeySecret(), client2.getAccessKeySecret());
  EXPECT_EQ(client1.getType(), client2.getType());
}
