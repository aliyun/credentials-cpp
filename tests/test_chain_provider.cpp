#include <gtest/gtest.h>
#include <alibabacloud/credential/provider/ChainProvider.hpp>
#include <alibabacloud/credential/provider/AccessKeyProvider.hpp>
#include <alibabacloud/credential/provider/BearerTokenProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <darabonba/Exception.hpp>

using namespace AlibabaCloud::Credential;

// ==================== ChainProvider Tests ====================

TEST(ChainProviderTest, EmptyChainThrows) {
  std::vector<std::shared_ptr<CredentialProvider>> emptyProviders;
  ChainProvider provider(emptyProviders);
  
  // Empty chain should throw when getting credential
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

TEST(ChainProviderTest, SingleProviderChain) {
  std::vector<std::shared_ptr<CredentialProvider>> providers;
  providers.push_back(std::make_shared<AccessKeyProvider>("chain_ak", "chain_secret"));
  
  ChainProvider provider(providers);
  auto credential = provider.getCredential();
  
  EXPECT_EQ("chain_ak", credential.getAccessKeyId());
  EXPECT_EQ("chain_secret", credential.getAccessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
}

TEST(ChainProviderTest, MultipleProvidersChain) {
  std::vector<std::shared_ptr<CredentialProvider>> providers;
  
  // Add a provider that will succeed
  providers.push_back(std::make_shared<AccessKeyProvider>("first_ak", "first_secret"));
  providers.push_back(std::make_shared<BearerTokenProvider>("bearer_token"));
  
  ChainProvider provider(providers);
  auto credential = provider.getCredential();
  
  // Should get credential from first provider
  EXPECT_EQ("first_ak", credential.getAccessKeyId());
  EXPECT_EQ("first_secret", credential.getAccessKeySecret());
}

TEST(ChainProviderTest, FallbackToSecondProvider) {
  std::vector<std::shared_ptr<CredentialProvider>> providers;
  
  // First provider will fail (empty credentials)
  auto config1 = std::make_shared<Models::Config>();
  config1->setType(Constant::ACCESS_KEY);
  // Don't set access key - will fail
  
  // Second provider will succeed
  providers.push_back(std::make_shared<AccessKeyProvider>("fallback_ak", "fallback_secret"));
  
  ChainProvider provider(providers);
  auto credential = provider.getCredential();
  
  EXPECT_EQ("fallback_ak", credential.getAccessKeyId());
  EXPECT_EQ("fallback_secret", credential.getAccessKeySecret());
}

TEST(ChainProviderTest, ConstructorWithMoveSemantics) {
  std::vector<std::shared_ptr<CredentialProvider>> providers;
  providers.push_back(std::make_shared<AccessKeyProvider>("move_ak", "move_secret"));
  
  ChainProvider provider(std::move(providers));
  auto credential = provider.getCredential();
  
  EXPECT_EQ("move_ak", credential.getAccessKeyId());
}

TEST(ChainProviderTest, ChainWithDifferentProviderTypes) {
  std::vector<std::shared_ptr<CredentialProvider>> providers;
  
  // Mix different provider types
  providers.push_back(std::make_shared<AccessKeyProvider>("ak_id", "ak_secret"));
  providers.push_back(std::make_shared<BearerTokenProvider>("bearer"));
  
  ChainProvider provider(providers);
  
  // Should try providers in order
  EXPECT_NO_THROW({
    provider.getCredential();
  });
}

TEST(ChainProviderTest, ConstGetCredential) {
  std::vector<std::shared_ptr<CredentialProvider>> providers;
  providers.push_back(std::make_shared<AccessKeyProvider>("const_ak", "const_secret"));
  
  const ChainProvider provider(providers);
  const auto &credential = provider.getCredential();
  
  EXPECT_EQ("const_ak", credential.getAccessKeyId());
  EXPECT_EQ("const_secret", credential.getAccessKeySecret());
}

// Test chain priority - earlier providers should be tried first
TEST(ChainProviderTest, PriorityOrder) {
  std::vector<std::shared_ptr<CredentialProvider>> providers;
  
  // Add providers in specific order
  providers.push_back(std::make_shared<AccessKeyProvider>("first_provider", "secret1"));
  providers.push_back(std::make_shared<AccessKeyProvider>("second_provider", "secret2"));
  providers.push_back(std::make_shared<AccessKeyProvider>("third_provider", "secret3"));
  
  ChainProvider provider(providers);
  auto credential = provider.getCredential();
  
  // Should always get from first provider
  EXPECT_EQ("first_provider", credential.getAccessKeyId());
  EXPECT_EQ("secret1", credential.getAccessKeySecret());
}

// Test that chain stops at first successful provider
TEST(ChainProviderTest, StopsAtFirstSuccess) {
  std::vector<std::shared_ptr<CredentialProvider>> providers;
  
  providers.push_back(std::make_shared<AccessKeyProvider>("success_ak", "success_secret"));
  providers.push_back(std::make_shared<BearerTokenProvider>("should_not_reach"));
  
  ChainProvider provider(providers);
  auto credential = provider.getCredential();
  
  // Should get AccessKey credential, not Bearer
  EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
  EXPECT_FALSE(credential.hasBearerToken());
}
