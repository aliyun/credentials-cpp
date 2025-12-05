#include <gtest/gtest.h>
#include <alibabacloud/credential/Credential.hpp>
#include <alibabacloud/credential/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credential/provider/BearerTokenProvider.hpp>
#include <alibabacloud/credential/provider/StsProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <vector>
#include <thread>

using namespace AlibabaCloud::Credential;

// ==================== Integration and Complex Scenario Tests ====================

TEST(IntegrationTest, MultipleClientsSameType) {
  Models::Config config1;
  config1.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("client1_ak")
        .setAccessKeySecret("client1_secret");
  
  Models::Config config2;
  config2.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("client2_ak")
        .setAccessKeySecret("client2_secret");
  
  Client client1(config1);
  Client client2(config2);
  
  EXPECT_EQ("client1_ak", client1.getAccessKeyId());
  EXPECT_EQ("client2_ak", client2.getAccessKeyId());
  EXPECT_NE(client1.getAccessKeyId(), client2.getAccessKeyId());
}

TEST(IntegrationTest, MultipleClientsDifferentTypes) {
  Models::Config akConfig;
  akConfig.setType(Constant::ACCESS_KEY)
          .setAccessKeyId("ak_id")
          .setAccessKeySecret("ak_secret");
  
  Models::Config bearerConfig;
  bearerConfig.setType(Constant::BEARER)
              .setBearerToken("bearer_token");
  
  Models::Config stsConfig;
  stsConfig.setType(Constant::STS)
           .setAccessKeyId("sts_ak")
           .setAccessKeySecret("sts_secret")
           .setSecurityToken("sts_token");
  
  Client akClient(akConfig);
  Client bearerClient(bearerConfig);
  Client stsClient(stsConfig);
  
  EXPECT_EQ(Constant::ACCESS_KEY, akClient.getType());
  EXPECT_EQ(Constant::BEARER, bearerClient.getType());
  EXPECT_EQ(Constant::STS, stsClient.getType());
}

TEST(IntegrationTest, ProviderToClientWorkflow) {
  // Create provider
  AccessKeyProvider provider("workflow_ak", "workflow_secret");
  
  // Get credential from provider
  auto credential = provider.getCredential();
  
  // Create config from credential
  Models::Config config;
  config.setType(credential.type())
        .setAccessKeyId(credential.accessKeyId())
        .setAccessKeySecret(credential.accessKeySecret());
  
  // Create client from config
  Client client(config);
  
  EXPECT_EQ("workflow_ak", client.getAccessKeyId());
  EXPECT_EQ("workflow_secret", client.getAccessKeySecret());
}

TEST(IntegrationTest, MultipleProvidersSequentialAccess) {
  std::vector<Provider*> providers;
  
  for (int i = 0; i < 10; ++i) {
    providers.push_back(new AccessKeyProvider(
        "ak_" + std::to_string(i),
        "secret_" + std::to_string(i)
    ));
  }
  
  for (int i = 0; i < 10; ++i) {
    auto credential = providers[i]->getCredential();
    EXPECT_EQ("ak_" + std::to_string(i), credential.accessKeyId());
  }
  
  // Cleanup
  for (auto p : providers) {
    delete p;
  }
}

TEST(IntegrationTest, ConcurrentProviderAccess) {
  AccessKeyProvider provider("concurrent_ak", "concurrent_secret");
  std::vector<std::thread> threads;
  std::atomic<int> successCount{0};
  
  for (int i = 0; i < 20; ++i) {
    threads.emplace_back([&provider, &successCount]() {
      try {
        auto credential = provider.getCredential();
        if (credential.accessKeyId() == "concurrent_ak") {
          successCount++;
        }
      } catch (...) {
        // Should not throw
      }
    });
  }
  
  for (auto& t : threads) {
    t.join();
  }
  
  EXPECT_EQ(20, successCount);
}

TEST(IntegrationTest, CredentialChaining) {
  // Simulate credential chaining scenario
  Models::CredentialModel cred1;
  cred1.setAccessKeyId("chain1_ak")
       .setAccessKeySecret("chain1_secret")
       .setType(Constant::ACCESS_KEY);
  
  // Pass credential data through multiple transformations
  Models::CredentialModel cred2;
  cred2.setAccessKeyId(cred1.accessKeyId())
       .setAccessKeySecret(cred1.accessKeySecret())
       .setType(cred1.type())
       .setProviderName("ChainedProvider");
  
  EXPECT_EQ(cred1.accessKeyId(), cred2.accessKeyId());
  EXPECT_EQ(cred1.accessKeySecret(), cred2.accessKeySecret());
  EXPECT_EQ("ChainedProvider", cred2.providerName());
}

TEST(IntegrationTest, ConfigCloning) {
  Models::Config original;
  original.setAccessKeyId("original_ak")
          .setAccessKeySecret("original_secret")
          .setTimeout(8000)
          .setConnectTimeout(12000)
          .setDisableIMDSv1(true);
  
  // Create multiple clones
  std::vector<Models::Config> clones;
  for (int i = 0; i < 5; ++i) {
    clones.push_back(original);
  }
  
  // Verify all clones have same values
  for (const auto& clone : clones) {
    EXPECT_EQ(original.accessKeyId(), clone.accessKeyId());
    EXPECT_EQ(original.timeout(), clone.timeout());
    EXPECT_EQ(original.disableIMDSv1(), clone.disableIMDSv1());
  }
}

TEST(IntegrationTest, CredentialRotation) {
  // Simulate credential rotation scenario
  std::vector<std::string> credentials = {
    "ak_v1", "ak_v2", "ak_v3", "ak_v4"
  };
  
  int currentVersion = 0;
  
  for (int i = 0; i < 10; ++i) {
    AccessKeyProvider provider(
        credentials[currentVersion],
        "secret_v" + std::to_string(currentVersion)
    );
    
    auto credential = provider.getCredential();
    EXPECT_EQ(credentials[currentVersion], credential.accessKeyId());
    
    // Rotate every 3 iterations
    if ((i + 1) % 3 == 0 && currentVersion < 3) {
      currentVersion++;
    }
  }
}

TEST(IntegrationTest, TypeConversion) {
  // Test converting between credential types
  Models::Config config;
  
  // Start as ACCESS_KEY
  config.setType(Constant::ACCESS_KEY)
        .setAccessKeyId("ak")
        .setAccessKeySecret("secret");
  
  EXPECT_EQ(Constant::ACCESS_KEY, config.type());
  
  // Convert to STS
  config.setType(Constant::STS)
        .setSecurityToken("token");
  
  EXPECT_EQ(Constant::STS, config.type());
  
  // Convert to BEARER
  config.setType(Constant::BEARER)
        .setBearerToken("bearer");
  
  EXPECT_EQ(Constant::BEARER, config.type());
}

TEST(IntegrationTest, PartialConfigUpdate) {
  Models::Config config;
  config.setAccessKeyId("initial_ak")
        .setAccessKeySecret("initial_secret")
        .setTimeout(5000);
  
  // Update only some fields
  config.setAccessKeyId("updated_ak")
        .setTimeout(8000);
  
  EXPECT_EQ("updated_ak", config.accessKeyId());
  EXPECT_EQ("initial_secret", config.accessKeySecret());
  EXPECT_EQ(8000, config.timeout());
}

TEST(IntegrationTest, JsonSerializationWorkflow) {
  // Create config
  Models::Config config1;
  config1.setAccessKeyId("json_ak")
        .setAccessKeySecret("json_secret")
        .setType(Constant::ACCESS_KEY)
        .setTimeout(7000)
        .setDisableIMDSv1(true);
  
  // Serialize to JSON
  auto json = config1.toMap();
  
  // Transfer JSON (simulate network/storage)
  auto transferredJson = json;
  
  // Deserialize from JSON
  Models::Config config2;
  config2.fromMap(transferredJson);
  
  // Verify all fields preserved
  EXPECT_EQ(config1.accessKeyId(), config2.accessKeyId());
  EXPECT_EQ(config1.accessKeySecret(), config2.accessKeySecret());
  EXPECT_EQ(config1.type(), config2.type());
  EXPECT_EQ(config1.timeout(), config2.timeout());
  EXPECT_EQ(config1.disableIMDSv1(), config2.disableIMDSv1());
}

TEST(IntegrationTest, ProviderChainFallback) {
  // Simulate fallback scenario with multiple providers
  std::vector<Provider*> providers;
  
  // Add different types of providers
  providers.push_back(new AccessKeyProvider("primary_ak", "primary_secret"));
  providers.push_back(new BearerTokenProvider("fallback_token"));
  providers.push_back(new StsProvider("sts_ak", "sts_secret", "sts_token"));
  
  // Try each provider
  for (size_t i = 0; i < providers.size(); ++i) {
    auto credential = providers[i]->getCredential();
    EXPECT_FALSE(credential.empty());
    EXPECT_FALSE(credential.type().empty());
  }
  
  // Cleanup
  for (auto p : providers) {
    delete p;
  }
}

TEST(IntegrationTest, ClientLifecycle) {
  // Test complete client lifecycle
  {
    Models::Config config;
    config.setType(Constant::ACCESS_KEY)
          .setAccessKeyId("lifecycle_ak")
          .setAccessKeySecret("lifecycle_secret");
    
    Client client(config);
    
    // Use client
    EXPECT_EQ("lifecycle_ak", client.getAccessKeyId());
    EXPECT_FALSE(client.empty());
    
    // Get credential multiple times
    for (int i = 0; i < 5; ++i) {
      auto credential = client.getCredential();
      EXPECT_EQ("lifecycle_ak", credential.accessKeyId());
    }
  }
  // Client destroyed, no leaks
}

TEST(IntegrationTest, ConfigDefaultValues) {
  Models::Config config;
  
  // Verify default values are set correctly
  EXPECT_EQ(5000, config.timeout());
  EXPECT_EQ(10000, config.connectTimeout());
  EXPECT_FALSE(config.disableIMDSv1());
  EXPECT_EQ(3600, config.durationSeconds());
  EXPECT_EQ("cn-hangzhou", config.regionId());
  EXPECT_EQ("sts.aliyuncs.com", config.stsEndpoint());
}

TEST(IntegrationTest, ConfigDefaultValuesPreservedAfterPartialUpdate) {
  Models::Config config;
  
  // Update only access key
  config.setAccessKeyId("test_ak");
  
  // Default values should still be there
  EXPECT_EQ(5000, config.timeout());
  EXPECT_EQ(10000, config.connectTimeout());
  EXPECT_EQ(3600, config.durationSeconds());
}

TEST(IntegrationTest, MultipleCredentialTypesInSequence) {
  // Test using different credential types in sequence
  
  // 1. Access Key
  {
    AccessKeyProvider provider("ak1", "secret1");
    auto cred = provider.getCredential();
    EXPECT_EQ(Constant::ACCESS_KEY, cred.type());
  }
  
  // 2. Bearer Token
  {
    BearerTokenProvider provider("token1");
    auto cred = provider.getCredential();
    EXPECT_EQ(Constant::BEARER, cred.type());
  }
  
  // 3. STS
  {
    StsProvider provider("sts_ak", "sts_secret", "sts_token");
    auto cred = provider.getCredential();
    EXPECT_EQ(Constant::STS, cred.type());
  }
}

TEST(IntegrationTest, ConstantsAreConsistent) {
  // Verify all constants are properly defined and consistent
  EXPECT_FALSE(Constant::ACCESS_KEY.empty());
  EXPECT_FALSE(Constant::STS.empty());
  EXPECT_FALSE(Constant::BEARER.empty());
  EXPECT_FALSE(Constant::ECS_RAM_ROLE.empty());
  EXPECT_FALSE(Constant::RAM_ROLE_ARN.empty());
  EXPECT_FALSE(Constant::RSA_KEY_PAIR.empty());
  EXPECT_FALSE(Constant::OIDC_ROLE_ARN.empty());
  EXPECT_FALSE(Constant::URL_STS.empty());
  EXPECT_FALSE(Constant::CLOUD_SSO.empty());
  EXPECT_FALSE(Constant::OAUTH.empty());
}

TEST(IntegrationTest, ProviderPolymorphism) {
  // Test polymorphic behavior
  std::vector<Provider*> providers;
  
  providers.push_back(new AccessKeyProvider("poly_ak", "poly_secret"));
  providers.push_back(new BearerTokenProvider("poly_token"));
  providers.push_back(new StsProvider("sts_ak", "sts_secret", "token"));
  
  // Access all providers polymorphically
  for (auto provider : providers) {
    auto credential = provider->getCredential();
    EXPECT_FALSE(credential.empty());
  }
  
  // Cleanup
  for (auto p : providers) {
    delete p;
  }
}

TEST(IntegrationTest, CredentialModelFieldValidation) {
  Models::CredentialModel model;
  
  // Initially empty
  EXPECT_TRUE(model.empty());
  
  // Set some fields
  model.setAccessKeyId("validation_ak");
  EXPECT_FALSE(model.empty());
  EXPECT_TRUE(model.hasAccessKeyId());
  
  model.setAccessKeySecret("validation_secret");
  EXPECT_TRUE(model.hasAccessKeySecret());
  
  model.setType("validation_type");
  EXPECT_TRUE(model.hasType());
}

TEST(IntegrationTest, ConfigChainedSetters) {
  // Test that all setters return reference for chaining
  Models::Config config;
  
  auto& result = config.setAccessKeyId("chain_ak")
                       .setAccessKeySecret("chain_secret")
                       .setTimeout(8000)
                       .setConnectTimeout(12000)
                       .setDisableIMDSv1(true)
                       .setType(Constant::ACCESS_KEY)
                       .setDurationSeconds(7200);
  
  // Result should be the same object
  EXPECT_EQ("chain_ak", result.accessKeyId());
  EXPECT_EQ(8000, result.timeout());
}

TEST(IntegrationTest, CredentialModelChainedSetters) {
  Models::CredentialModel model;
  
  auto& result = model.setAccessKeyId("model_ak")
                      .setAccessKeySecret("model_secret")
                      .setType("model_type")
                      .setProviderName("model_provider")
                      .setBearerToken("model_token")
                      .setSecurityToken("model_sec_token");
  
  EXPECT_EQ("model_ak", result.accessKeyId());
  EXPECT_EQ("model_provider", result.providerName());
}
