#include <gtest/gtest.h>
#include <alibabacloud/credentials/provider/RefreshableProvider.hpp>
#include <alibabacloud/credentials/provider/NeedFreshProvider.hpp>
#include <alibabacloud/credentials/Constant.hpp>
#include <thread>
#include <vector>
#include <atomic>

using namespace AlibabaCloud::Credentials;

/**
 * @brief Mock provider for testing RefreshableProvider thread safety
 *
 * This test verifies that the RefreshableProvider implementation follows
 * thread-safety best practices:
 * - Atomic replacement of entire RefreshResult object
 * - Lock-free reads, locked refreshes
 */
class MockRefreshableProvider : public RefreshableProvider {
public:
  MockRefreshableProvider() 
      : RefreshableProvider(StaleValueBehavior::ALLOW_, 
                            std::make_shared<OneCallerBlocksPrefetch>()),
        refreshCount_(0) {}

  std::string getProviderName() const override { return "mock_refreshable"; }

  int getRefreshCount() const { return refreshCount_.load(); }

protected:
  RefreshResult doRefresh() const override {
    // Simulate network delay
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    int count = ++refreshCount_;
    
    Models::CredentialModel cred;
    cred.setAccessKeyId("ak_" + std::to_string(count));
    cred.setAccessKeySecret("sk_" + std::to_string(count));
    cred.setType(Constant::ACCESS_KEY);
    
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // Set longer expiration to avoid too frequent refreshes
    return RefreshResult(cred, now + 300, now + 60);
  }

private:
  mutable std::atomic<int> refreshCount_;
};

/**
 * @brief Test concurrent access to RefreshableProvider
 * 
 * This test verifies that:
 * 1. Multiple threads can safely call getCredential() concurrently
 * 2. Each credential returned is internally consistent (ak_N matches sk_N)
 * 3. No crashes or data corruption occurs
 */
TEST(ThreadSafetyTest, RefreshableProviderConcurrentAccess) {
  MockRefreshableProvider provider;
  std::atomic<bool> stop{false};
  std::atomic<int> successCount{0};
  std::atomic<int> consistencyErrors{0};
  std::vector<std::thread> threads;
  
  // Create multiple reader threads
  for (int i = 0; i < 4; ++i) {
    threads.emplace_back([&]() {
      while (!stop) {
        try {
          const auto& cred = provider.getCredential();
          
          std::string ak = cred.getAccessKeyId();
          std::string sk = cred.getAccessKeySecret();
          
          // Verify consistency: ak_N and sk_N should have same N
          if (ak.length() > 3 && sk.length() > 3) {
            std::string akNum = ak.substr(3);
            std::string skNum = sk.substr(3);
            if (akNum != skNum) {
              consistencyErrors++;
            }
          }
          successCount++;
        } catch (const std::exception&) {
          // First call may throw if cache is not yet populated
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    });
  }
  
  // Run for 1 second
  std::this_thread::sleep_for(std::chrono::seconds(1));
  stop = true;
  
  for (auto& t : threads) {
    if (t.joinable()) t.join();
  }
  
  // Verify results
  EXPECT_GT(successCount.load(), 0) << "Should have successful reads";
  EXPECT_EQ(consistencyErrors.load(), 0) << "Should have no consistency errors";
  EXPECT_GT(provider.getRefreshCount(), 0) << "Should have refreshed at least once";
}

/**
 * @brief Mock provider for testing NeedFreshProvider basic functionality
 * 
 * Note: NeedFreshProvider does not have built-in thread safety.
 * This test only verifies single-threaded behavior.
 */
class MockNeedFreshProvider : public NeedFreshProvider {
public:
  MockNeedFreshProvider() : NeedFreshProvider() {
    credential_.setAccessKeyId("initial_ak");
    credential_.setAccessKeySecret("initial_sk");
    expiration_ = static_cast<int64_t>(time(nullptr)) - 100; // Expired
  }

  std::string getProviderName() const override { return "mock_need_fresh"; }

  int getRefreshCount() const { return refreshCount_; }

protected:
  bool refreshCredential() const override {
    int count = ++refreshCount_;
    
    credential_.setAccessKeyId("ak_" + std::to_string(count));
    credential_.setAccessKeySecret("sk_" + std::to_string(count));
    
    expiration_ = static_cast<int64_t>(time(nullptr)) + 300;
    return true;
  }

private:
  mutable int refreshCount_{0};
};

/**
 * @brief Test basic functionality of NeedFreshProvider (single-threaded)
 * 
 * NeedFreshProvider does not have built-in thread safety like RefreshableProvider.
 * This test verifies basic refresh behavior in a single-threaded context.
 */
TEST(ThreadSafetyTest, NeedFreshProviderBasicFunctionality) {
  MockNeedFreshProvider provider;
  
  // First call should trigger refresh since credential is expired
  const auto& cred1 = provider.getCredential();
  EXPECT_EQ(cred1.getAccessKeyId(), "ak_1");
  EXPECT_EQ(cred1.getAccessKeySecret(), "sk_1");
  EXPECT_EQ(provider.getRefreshCount(), 1);
  
  // Second call should not trigger refresh since expiration is far in future
  const auto& cred2 = provider.getCredential();
  EXPECT_EQ(cred2.getAccessKeyId(), "ak_1");
  EXPECT_EQ(cred2.getAccessKeySecret(), "sk_1");
  EXPECT_EQ(provider.getRefreshCount(), 1);
}