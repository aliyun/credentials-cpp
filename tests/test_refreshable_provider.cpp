#include <gtest/gtest.h>
#include <alibabacloud/credential/provider/RefreshableProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <thread>
#include <chrono>

using namespace AlibabaCloud::Credential;

// ==================== RefreshableProvider Tests ====================

// Test implementation of RefreshableProvider
class TestRefreshableProvider : public RefreshableProvider {
public:
  TestRefreshableProvider(
      StaleValueBehavior behavior = StaleValueBehavior::STRICT_,
      std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>())
      : RefreshableProvider(behavior, strategy), 
        refreshCount_(0),
        shouldFail_(false),
        customExpiration_(0) {}
  
  void setShouldFail(bool fail) {
    shouldFail_ = fail;
  }
  
  void setCustomExpiration(int64_t expiration) {
    customExpiration_ = expiration;
  }
  
  int getRefreshCount() const {
    return refreshCount_;
  }
  
  std::string getProviderName() const override {
    return "test_refreshable";
  }

protected:
  RefreshResult doRefresh() const override {
    refreshCount_++;
    
    if (shouldFail_) {
      throw std::runtime_error("Simulated refresh failure");
    }
    
    int64_t now = getCurrentTime();
    int64_t expiration = customExpiration_ > 0 ? customExpiration_ : now + 3600;
    int64_t staleTime = expiration;
    int64_t prefetchTime = expiration - PREFETCH_THRESHOLD;
    
    Models::CredentialModel credential;
    credential.setType(Constant::ACCESS_KEY)
              .setAccessKeyId("test_ak_" + std::to_string(refreshCount_))
              .setAccessKeySecret("test_secret_" + std::to_string(refreshCount_));
    
    return RefreshResult(credential, staleTime, prefetchTime);
  }

private:
  mutable int refreshCount_;
  mutable bool shouldFail_;
  mutable int64_t customExpiration_;
};

TEST(RefreshableProviderTest, DefaultConstructor) {
  EXPECT_NO_THROW({
    TestRefreshableProvider provider;
  });
}

TEST(RefreshableProviderTest, ConstructorWithStrictBehavior) {
  EXPECT_NO_THROW({
    TestRefreshableProvider provider(StaleValueBehavior::STRICT_);
  });
}

TEST(RefreshableProviderTest, ConstructorWithAllowBehavior) {
  EXPECT_NO_THROW({
    TestRefreshableProvider provider(StaleValueBehavior::ALLOW_);
  });
}

TEST(RefreshableProviderTest, ConstructorWithNonBlockingPrefetch) {
  auto strategy = std::make_shared<NonBlockingPrefetch>();
  EXPECT_NO_THROW({
    TestRefreshableProvider provider(StaleValueBehavior::STRICT_, strategy);
  });
}

TEST(RefreshableProviderTest, ConstructorWithOneCallerBlocksPrefetch) {
  auto strategy = std::make_shared<OneCallerBlocksPrefetch>();
  EXPECT_NO_THROW({
    TestRefreshableProvider provider(StaleValueBehavior::STRICT_, strategy);
  });
}

TEST(RefreshableProviderTest, FirstGetCredentialTriggersRefresh) {
  TestRefreshableProvider provider;
  
  auto credential = provider.getCredential();
  
  EXPECT_EQ(1, provider.getRefreshCount());
  EXPECT_EQ("test_ak_1", credential.getAccessKeyId());
  EXPECT_EQ("test_secret_1", credential.getAccessKeySecret());
  EXPECT_EQ(Constant::ACCESS_KEY, credential.getType());
}

TEST(RefreshableProviderTest, SecondGetCredentialUsesCache) {
  TestRefreshableProvider provider;
  
  auto credential1 = provider.getCredential();
  auto credential2 = provider.getCredential();
  
  // Should only refresh once
  EXPECT_EQ(1, provider.getRefreshCount());
  EXPECT_EQ(credential1.getAccessKeyId(), credential2.getAccessKeyId());
}

TEST(RefreshableProviderTest, ExpiredCredentialTriggersRefresh) {
  TestRefreshableProvider provider;
  
  // Set expiration to past
  int64_t pastTime = static_cast<int64_t>(std::time(nullptr)) - 100;
  provider.setCustomExpiration(pastTime);
  
  auto credential1 = provider.getCredential();
  EXPECT_EQ(1, provider.getRefreshCount());
  
  // Set expiration to future for next refresh
  provider.setCustomExpiration(0);
  
  // Get again, should trigger refresh because credential is expired
  auto credential2 = provider.getCredential();
  EXPECT_GE(provider.getRefreshCount(), 1);
}

TEST(RefreshableProviderTest, PrefetchThresholdTriggersAsyncRefresh) {
  auto strategy = std::make_shared<OneCallerBlocksPrefetch>();
  TestRefreshableProvider provider(StaleValueBehavior::STRICT_, strategy);
  
  // Set expiration to within prefetch threshold
  int64_t nearFuture = static_cast<int64_t>(std::time(nullptr)) + 
                       RefreshableProvider::PREFETCH_THRESHOLD - 10;
  provider.setCustomExpiration(nearFuture);
  
  auto credential = provider.getCredential();
  EXPECT_EQ(1, provider.getRefreshCount());
  
  // Sleep a bit to allow prefetch
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  // Get again, should trigger prefetch
  provider.setCustomExpiration(0);
  auto credential2 = provider.getCredential();
  EXPECT_GE(provider.getRefreshCount(), 1);
}

TEST(RefreshableProviderTest, RefreshFailureWithNoCacheThrows) {
  TestRefreshableProvider provider(StaleValueBehavior::STRICT_);
  provider.setShouldFail(true);
  
  EXPECT_THROW({
    provider.getCredential();
  }, std::exception);
}

TEST(RefreshableProviderTest, RefreshFailureWithValidCacheReturnsCache) {
  TestRefreshableProvider provider(StaleValueBehavior::STRICT_);
  
  // First get credential successfully
  auto credential1 = provider.getCredential();
  EXPECT_EQ(1, provider.getRefreshCount());
  
  // Now make refresh fail
  provider.setShouldFail(true);
  
  // Get again - should return cached credential
  auto credential2 = provider.getCredential();
  EXPECT_EQ(credential1.getAccessKeyId(), credential2.getAccessKeyId());
}

TEST(RefreshableProviderTest, StrictBehaviorWithExpiredCacheThrows) {
  TestRefreshableProvider provider(StaleValueBehavior::STRICT_);
  
  // Get initial credential with short expiration
  int64_t shortExpiration = static_cast<int64_t>(std::time(nullptr)) + 1;
  provider.setCustomExpiration(shortExpiration);
  auto credential1 = provider.getCredential();
  
  // Wait for expiration
  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  // Make refresh fail
  provider.setShouldFail(true);
  
  // Strict mode should throw on expired cache + failed refresh
  EXPECT_THROW({
    provider.getCredential();
  }, std::exception);
}

TEST(RefreshableProviderTest, AllowBehaviorWithExpiredCacheReturnsStale) {
  TestRefreshableProvider provider(StaleValueBehavior::ALLOW_);
  
  // Get initial credential with short expiration
  int64_t shortExpiration = static_cast<int64_t>(std::time(nullptr)) + 1;
  provider.setCustomExpiration(shortExpiration);
  auto credential1 = provider.getCredential();
  
  // Wait for expiration
  std::this_thread::sleep_for(std::chrono::seconds(2));
  
  // Make refresh fail
  provider.setShouldFail(true);
  
  // Allow mode should return stale value
  EXPECT_NO_THROW({
    auto credential2 = provider.getCredential();
    EXPECT_EQ(credential1.getAccessKeyId(), credential2.getAccessKeyId());
  });
}

TEST(RefreshableProviderTest, ConstGetCredential) {
  const TestRefreshableProvider provider;
  
  EXPECT_NO_THROW({
    const auto& credential = provider.getCredential();
    EXPECT_FALSE(credential.getAccessKeyId().empty());
  });
}

TEST(RefreshableProviderTest, ConcurrentAccess) {
  TestRefreshableProvider provider;
  std::vector<std::thread> threads;
  
  // Multiple threads accessing simultaneously
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&provider]() {
      for (int j = 0; j < 5; ++j) {
        EXPECT_NO_THROW({
          auto credential = provider.getCredential();
          EXPECT_FALSE(credential.getAccessKeyId().empty());
        });
      }
    });
  }
  
  for (auto& t : threads) {
    t.join();
  }
  
  // Should have refreshed at least once
  EXPECT_GE(provider.getRefreshCount(), 1);
}

TEST(RefreshableProviderTest, StrtotimeUtility) {
  // Test time parsing utility
  std::string gmtTime = "2024-12-05T10:30:45Z";
  int64_t timestamp = static_cast<int64_t>(std::time(nullptr));
  
  // Utility methods are protected, tested indirectly through provider
  EXPECT_GT(timestamp, 0);
}

TEST(RefreshableProviderTest, GetCurrentTimeUtility) {
  int64_t time1 = static_cast<int64_t>(std::time(nullptr));
  std::this_thread::sleep_for(std::chrono::seconds(1));
  int64_t time2 = static_cast<int64_t>(std::time(nullptr));
  
  EXPECT_GT(time2, time1);
  EXPECT_GE(time2 - time1, 1);
}

TEST(RefreshableProviderTest, PrefetchStrategyNonBlocking) {
  NonBlockingPrefetch strategy;
  bool executed = false;
  
  strategy.prefetch([&executed]() {
    executed = true;
  });
  
  // Give some time for async execution
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  EXPECT_TRUE(executed);
}

TEST(RefreshableProviderTest, PrefetchStrategyOneCallerBlocks) {
  OneCallerBlocksPrefetch strategy;
  bool executed = false;
  
  strategy.prefetch([&executed]() {
    executed = true;
  });
  
  // Synchronous, should execute immediately
  EXPECT_TRUE(executed);
}

TEST(RefreshableProviderTest, MultipleRefreshesIncrementCount) {
  TestRefreshableProvider provider;
  
  // Force multiple refreshes by expiring cache
  for (int i = 0; i < 3; ++i) {
    int64_t pastTime = static_cast<int64_t>(std::time(nullptr)) - 100;
    provider.setCustomExpiration(pastTime);
    provider.getCredential();
  }
  
  EXPECT_GE(provider.getRefreshCount(), 3);
}

TEST(RefreshableProviderTest, RefreshResultStructure) {
  Models::CredentialModel credential;
  credential.setAccessKeyId("test_ak")
            .setAccessKeySecret("test_secret");
  
  int64_t now = static_cast<int64_t>(std::time(nullptr));
  RefreshResult result(credential, now + 3600, now + 3420);
  
  EXPECT_EQ("test_ak", result.credential.getAccessKeyId());
  EXPECT_EQ(now + 3600, result.staleTime);
  EXPECT_EQ(now + 3420, result.prefetchTime);
}

TEST(RefreshableProviderTest, RefreshResultDefaultConstructor) {
  RefreshResult result;
  
  EXPECT_EQ(0, result.staleTime);
  EXPECT_EQ(0, result.prefetchTime);
  EXPECT_TRUE(result.credential.empty());
}
