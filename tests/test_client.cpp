#include <gtest/gtest.h>
#include <alibabacloud/credentials/Client.hpp>
#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credentials/provider/BearerTokenProvider.hpp>
#include <alibabacloud/credentials/provider/StsProvider.hpp>
#include <alibabacloud/credentials/provider/DefaultProvider.hpp>
#include <alibabacloud/credentials/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credentials/provider/RamRoleArnProvider.hpp>
#include <alibabacloud/credentials/provider/RsaKeyPairProvider.hpp>
#include <alibabacloud/credentials/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credentials/provider/URLProvider.hpp>
#include <alibabacloud/credentials/provider/CloudSSOCredentialsProvider.hpp>
#include <alibabacloud/credentials/provider/OAuthCredentialsProvider.hpp>
#include <cstdlib>
#include <fstream>
#include <memory>

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

// ==================== Client Constructor Tests ====================

class ClientTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ClientTest, DefaultConstructor) {
    // Test default constructor - should create DefaultProvider
    Client client;
    
    // Client should be created successfully
    // We can't directly access provider_ but we can test that it works
    EXPECT_NO_THROW({
        // Default constructor should work without throwing
        Client testClient;
    });
}

TEST_F(ClientTest, ConstructorWithConfig) {
    // Test constructor with Config object (copy)
    Models::Config config;
    config.setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setType(Constant::ACCESS_KEY);
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, ConstructorWithMoveConfig) {
    // Test constructor with Config object (move)
    Models::Config config;
    config.setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setType(Constant::ACCESS_KEY);
    
    EXPECT_NO_THROW({
        Client client(std::move(config));
    });
}

TEST_F(ClientTest, ConstructorWithSharedPtrConfig) {
    // Test constructor with shared_ptr<Config>
    auto config = std::make_shared<Models::Config>();
    config->setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setType(Constant::ACCESS_KEY);
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, ConstructorWithNullConfig) {
    // Test constructor with null config - should create DefaultProvider
    std::shared_ptr<Models::Config> nullConfig = nullptr;
    
    EXPECT_NO_THROW({
        Client client(nullConfig);
    });
}

TEST_F(ClientTest, ConstructorWithProvider) {
    // Test constructor with Provider
    auto provider = std::make_shared<AccessKeyProvider>("test_ak", "test_secret");
    
    EXPECT_NO_THROW({
        Client client(provider);
    });
}

// ==================== Provider Creation Tests ====================

TEST_F(ClientTest, AccessKeyClient) {
    Models::Config config;
    config.setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setType(Constant::ACCESS_KEY);
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, BearerTokenClient) {
    Models::Config config;
    config.setBearerToken("test_bearer_token")
          .setType(Constant::BEARER);
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, StsClient) {
    Models::Config config;
    config.setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setSecurityToken("test_security_token")
          .setType(Constant::STS);
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, EcsRamRoleClient) {
    Models::Config config;
    config.setRoleName("test_role_name")
          .setType(Constant::ECS_RAM_ROLE);
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, RamRoleArnClient) {
    Models::Config config;
    config.setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setRoleArn("acs:ram::123456789:role/test-role")
          .setRoleSessionName("test-session")
          .setType(Constant::RAM_ROLE_ARN);
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, RsaKeyPairClient) {
    // Create a temporary private key file
    std::string keyPath = getTempDir() + "/test_rsa_client_key.pem";
    std::ofstream keyFile(keyPath);
    keyFile << "-----BEGIN RSA PRIVATE KEY-----\ntest_private_key_content\n-----END RSA PRIVATE KEY-----";
    keyFile.close();

    Models::Config config;
    config.setPublicKeyId("test_public_key_id")
          .setPrivateKeyFile(keyPath)
          .setType(Constant::RSA_KEY_PAIR);

    EXPECT_NO_THROW({
        Client client(config);
    });

    std::remove(keyPath.c_str());
}

TEST_F(ClientTest, OIDCRoleArnClient) {
    // Create a temporary OIDC token file
    std::string tokenPath = getTempDir() + "/test_oidc_client_token.txt";
    std::ofstream tokenFile(tokenPath);
    tokenFile << "test_oidc_token_content";
    tokenFile.close();

    Models::Config config;
    config.setRoleArn("acs:ram::123456789:role/test-role")
          .setOidcProviderArn("acs:ram::123456789:oidc-provider/test")
          .setOidcTokenFilePath(tokenPath)
          .setRoleSessionName("test-session")
          .setType(Constant::OIDC_ROLE_ARN);

    EXPECT_NO_THROW({
        Client client(config);
    });

    std::remove(tokenPath.c_str());
}

TEST_F(ClientTest, URLProviderClient) {
    Models::Config config;
    config.setCredentialsUri("http://credentials.example.com")
          .setType(Constant::URL_STS);
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, CloudSSOClient) {
    Models::Config config;
    config.setType(Constant::CLOUD_SSO);
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, OAuthClient) {
    Models::Config config;
    config.setType(Constant::OAUTH);
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, UnknownTypeDefaultsToDefault) {
    Models::Config config;
    config.setType("unknown_type");
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

// ==================== Client Functionality Tests ====================

TEST_F(ClientTest, GetCredential) {
    // Test getting credential from AccessKey client
    Models::Config config;
    config.setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setType(Constant::ACCESS_KEY);
    
    Client client(config);
    
    EXPECT_NO_THROW({
        auto credential = client.getCredential();
        EXPECT_EQ("test_ak_id", credential.getAccessKeyId());
        EXPECT_EQ("test_ak_secret", credential.getAccessKeySecret());
        EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
    });
}

TEST_F(ClientTest, GetCredentialFromBearerToken) {
    Models::Config config;
    config.setBearerToken("test_bearer_token")
          .setType(Constant::BEARER);
    
    Client client(config);
    
    EXPECT_NO_THROW({
        auto credential = client.getCredential();
        EXPECT_EQ("test_bearer_token", credential.getBearerToken());
        EXPECT_EQ(Constant::BEARER, credential.getType());
    });
}

TEST_F(ClientTest, GetCredentialFromSts) {
    Models::Config config;
    config.setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setSecurityToken("test_security_token")
          .setType(Constant::STS);
    
    Client client(config);
    
    EXPECT_NO_THROW({
        auto credential = client.getCredential();
        EXPECT_EQ("test_ak_id", credential.getAccessKeyId());
        EXPECT_EQ("test_ak_secret", credential.getAccessKeySecret());
        EXPECT_EQ("test_security_token", credential.getSecurityToken());
        EXPECT_EQ(Constant::STS, credential.getType());
    });
}

// ==================== Edge Cases and Error Handling ====================

TEST_F(ClientTest, EmptyConfig) {
    Models::Config config;
    // Empty config should default to DefaultProvider
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, ConfigWithEmptyType) {
    Models::Config config;
    config.setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setType("");  // Empty type
    
    EXPECT_NO_THROW({
        Client client(config);
    });
}

TEST_F(ClientTest, MultipleConstructorCalls) {
    // Test creating multiple clients
    Models::Config config1;
    config1.setAccessKeyId("ak1").setAccessKeySecret("secret1").setType(Constant::ACCESS_KEY);
    
    Models::Config config2;
    config2.setBearerToken("token2").setType(Constant::BEARER);
    
    EXPECT_NO_THROW({
        Client client1(config1);
        Client client2(config2);
        Client client3;  // Default constructor
    });
}

// ==================== JSON Serialization Tests ====================

TEST_F(ClientTest, ToJsonSerialization) {
    Models::Config config;
    config.setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setType(Constant::ACCESS_KEY);
    
    Client client(config);
    
    EXPECT_NO_THROW({
        Darabonba::Json json;
        to_json(json, client);
        // Should contain config information
        EXPECT_TRUE(json.contains("config"));
    });
}

TEST_F(ClientTest, FromJsonDeserialization) {
    // Create JSON with config
    Darabonba::Json json;
    json["config"]["accessKeyId"] = "test_ak_id";
    json["config"]["accessKeySecret"] = "test_ak_secret";
    json["config"]["type"] = Constant::ACCESS_KEY;
    
    EXPECT_NO_THROW({
        Client client;
        from_json(json, client);
    });
}

// ==================== Memory Management Tests ====================

TEST_F(ClientTest, SharedPtrConfigLifetime) {
    auto config = std::make_shared<Models::Config>();
    config->setAccessKeyId("test_ak_id")
          .setAccessKeySecret("test_ak_secret")
          .setType(Constant::ACCESS_KEY);
    
    // Create client with shared_ptr config
    auto client = std::unique_ptr<Client>(new Client(config));
    
    // Config should still be valid
    EXPECT_EQ("test_ak_id", config->getAccessKeyId());
    
    // Client should work even after config goes out of scope
    EXPECT_NO_THROW({
        auto credential = client->getCredential();
        EXPECT_EQ("test_ak_id", credential.getAccessKeyId());
    });
}

TEST_F(ClientTest, ProviderLifetime) {
    auto provider = std::make_shared<AccessKeyProvider>("test_ak", "test_secret");
    
    // Create client with provider
    auto client = std::unique_ptr<Client>(new Client(provider));
    
    EXPECT_NO_THROW({
        auto credential = client->getCredential();
        EXPECT_EQ("test_ak", credential.getAccessKeyId());
        EXPECT_EQ("test_secret", credential.getAccessKeySecret());
    });
}

// ==================== Thread Safety Tests ====================

TEST_F(ClientTest, ConcurrentClientCreation) {
    std::vector<std::thread> threads;
    std::vector<std::unique_ptr<Client>> clients(10);
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&clients, i]() {
            Models::Config config;
            config.setAccessKeyId("ak_" + std::to_string(i))
                  .setAccessKeySecret("secret_" + std::to_string(i))
                  .setType(Constant::ACCESS_KEY);
            
            clients[i] = std::unique_ptr<Client>(new Client(config));
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // All clients should be created successfully
    for (int i = 0; i < 10; ++i) {
        EXPECT_NE(nullptr, clients[i]);
        auto credential = clients[i]->getCredential();
        EXPECT_EQ("ak_" + std::to_string(i), credential.getAccessKeyId());
    }
}

// ==================== Performance Tests ====================

TEST_F(ClientTest, ClientCreationPerformance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Create 1000 clients
    std::vector<std::unique_ptr<Client>> clients;
    clients.reserve(1000);
    
    for (int i = 0; i < 1000; ++i) {
        Models::Config config;
        config.setAccessKeyId("ak_" + std::to_string(i))
              .setAccessKeySecret("secret_" + std::to_string(i))
              .setType(Constant::ACCESS_KEY);
        
        clients.push_back(std::unique_ptr<Client>(new Client(config)));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete within reasonable time (less than 1 second)
    EXPECT_LT(duration.count(), 1000);
    
    // All clients should work
    EXPECT_EQ(1000U, clients.size());
    for (const auto& client : clients) {
        EXPECT_NE(nullptr, client);
    }
}

// ==================== Integration Tests ====================

TEST_F(ClientTest, EndToEndAccessKeyFlow) {
    // Complete flow: Config -> Client -> Credential
    Models::Config config;
    config.setAccessKeyId("integration_test_ak")
          .setAccessKeySecret("integration_test_secret")
          .setType(Constant::ACCESS_KEY);
    
    Client client(config);
    auto credential = client.getCredential();
    
    EXPECT_EQ("integration_test_ak", credential.getAccessKeyId());
    EXPECT_EQ("integration_test_secret", credential.getAccessKeySecret());
    EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
    EXPECT_FALSE(credential.empty());
}

TEST_F(ClientTest, EndToEndBearerTokenFlow) {
    Models::Config config;
    config.setBearerToken("integration_test_token")
          .setType(Constant::BEARER);
    
    Client client(config);
    auto credential = client.getCredential();
    
    EXPECT_EQ("integration_test_token", credential.getBearerToken());
    EXPECT_EQ(Constant::BEARER, credential.getType());
    EXPECT_FALSE(credential.empty());
}

TEST_F(ClientTest, EndToEndStsFlow) {
    Models::Config config;
    config.setAccessKeyId("integration_test_ak")
          .setAccessKeySecret("integration_test_secret")
          .setSecurityToken("integration_test_token")
          .setType(Constant::STS);
    
    Client client(config);
    auto credential = client.getCredential();
    
    EXPECT_EQ("integration_test_ak", credential.getAccessKeyId());
    EXPECT_EQ("integration_test_secret", credential.getAccessKeySecret());
    EXPECT_EQ("integration_test_token", credential.getSecurityToken());
    EXPECT_EQ(Constant::STS, credential.getType());
    EXPECT_FALSE(credential.empty());
}
