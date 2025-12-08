#ifndef ALIBABACLOUD_CREDENTIAL_REFRESHABLEPROVIDER_HPP_
#define ALIBABACLOUD_CREDENTIAL_REFRESHABLEPROVIDER_HPP_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

#ifndef _WIN32
#include <time.h>
#include <cstring>
#endif

#include <alibabacloud/credential/provider/Provider.hpp>

namespace AlibabaCloud {
namespace Credential {

/**
 * @brief Refresh result wrapper class
 * 
 * Contains credential value, expiration time and prefetch time
 */
struct RefreshResult {
  Models::CredentialModel credential;
  int64_t staleTime = 0;     // Expiration time (seconds timestamp)
  int64_t prefetchTime = 0;  // Prefetch time (seconds timestamp)
  
  RefreshResult() = default;
  RefreshResult(const Models::CredentialModel& cred, int64_t stale, int64_t prefetch)
      : credential(cred), staleTime(stale), prefetchTime(prefetch) {}
};

/**
 * @brief Stale value behavior policy
 */

// Avoid Windows macro name conflicts (e.g., STRICT, ALLOW)
#ifdef STRICT_
#undef STRICT_
#endif
#ifdef ALLOW_
#undef ALLOW_
#endif
enum class StaleValueBehavior {
  STRICT_,  // Strict mode: never return stale cached values
  ALLOW_    // Allow mode: allow returning stale values to avoid service overload
};

/**
 * @brief Prefetch strategy base class
 */
class PrefetchStrategy {
public:
  virtual ~PrefetchStrategy() = default;
  virtual void prefetch(std::function<void()> action) = 0;
};

/**
 * @brief Non-blocking prefetch strategy (async refresh in background thread)
 */
class NonBlockingPrefetch : public PrefetchStrategy {
public:
  void prefetch(std::function<void()> action) override {
    // Execute refresh in detached thread
    std::thread([action]() {
      try {
        action();
      } catch (const std::exception& e) {
        // Log error but don't throw exception
        std::cerr << "NonBlockingPrefetch error: " << e.what() << std::endl;
      }
    }).detach();
  }
};

/**
 * @brief One caller blocks prefetch strategy (synchronous refresh)
 */
class OneCallerBlocksPrefetch : public PrefetchStrategy {
public:
  void prefetch(std::function<void()> action) override {
    action();  // Synchronous execution
  }
};

/**
 * @brief Refreshable credential provider base class
 * 
 * Reference Python SDK's RefreshCachedSupplier implementation
 * Provides async background refresh mechanism with:
 * - 180 seconds prefetch refresh
 * - 15 minutes expiration time window
 * - Failure retry and stale value handling
 * - Configurable refresh strategy (blocking/non-blocking)
 */
class RefreshableProvider : public Provider {
public:
  // Constants
  static constexpr int64_t STALE_TIME_WINDOW = 15 * 60;      // 15 minutes stale window
  static constexpr int64_t PREFETCH_THRESHOLD = 180;          // 180 seconds prefetch threshold
  static constexpr int64_t REFRESH_BLOCKING_MAX_WAIT_MS = 10000;  // Max wait 10 seconds (in milliseconds)

  /**
   * @brief 构造函数
   * 
   * @param staleValueBehavior 过期值处理策略
   * @param prefetchStrategy 预取策略（默认非阻塞）
   */
  explicit RefreshableProvider(
      StaleValueBehavior staleValueBehavior = StaleValueBehavior::STRICT_,
      std::shared_ptr<PrefetchStrategy> prefetchStrategy = 
          std::make_shared<NonBlockingPrefetch>())
      : staleValueBehavior_(staleValueBehavior),
        prefetchStrategy_(prefetchStrategy),
        consecutiveRefreshFailures_(0),
        cachedValue_(nullptr) {}

  virtual ~RefreshableProvider() {
    shutdown();
  }

  /**
   * @brief Get credential (thread safe)
   */
  virtual Models::CredentialModel& getCredential() override {
    return const_cast<Models::CredentialModel&>(
        static_cast<const RefreshableProvider*>(this)->getCredential());
  }

  virtual const Models::CredentialModel& getCredential() const override {
    std::lock_guard<std::mutex> lock(accessMutex_);
    
    if (cacheIsStale()) {
      // Cache expired, synchronous refresh
      refreshCache();
    } else if (shouldInitiateCachePrefetch()) {
      // About to expire, async prefetch refresh
      prefetchCache();
    }
    
    if (!cachedValue_) {
      throw std::runtime_error("No cached credential available");
    }
    
    return cachedValue_->credential;
  }

protected:
  /**
   * @brief Subclass implemented credential refresh logic
   * 
   * @return RefreshResult containing new credential and expiration time
   */
  virtual RefreshResult doRefresh() const = 0;

  /**
   * @brief Time utility: convert GMT time string to timestamp
   * 
   * Accepts format: "%Y-%m-%dT%H:%M:%SZ"
   */
  static int64_t strtotime(const std::string& gmt) {
    std::tm tm{};
#ifdef _WIN32
    std::istringstream ss(gmt);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
      throw std::runtime_error("Failed to parse GMT datetime: " + gmt);
    }
    // Convert to time_t in UTC
    time_t t = _mkgmtime(&tm);
    if (t == -1) {
      throw std::runtime_error("Failed to convert tm to time_t (UTC) for: " + gmt);
    }
    return static_cast<int64_t>(t);
#else
    // Use strptime + timegm on POSIX
    if (strptime(gmt.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm) == nullptr) {
      throw std::runtime_error("Failed to parse GMT datetime: " + gmt);
    }
    time_t t = timegm(&tm);
    if (t == -1) {
      throw std::runtime_error("Failed to convert tm to time_t (UTC) for: " + gmt);
    }
    return static_cast<int64_t>(t);
#endif
  }

  /**
   * @brief Time utility: get current GMT time string in ISO8601 format
   * 
   * Returns: "%Y-%m-%dT%H:%M:%SZ"
   */
  static std::string gmt_datetime() {
    time_t now;
    time(&now);
#ifdef _WIN32
    std::tm tm{};
    if (gmtime_s(&tm, &now) != 0) {
      throw std::runtime_error("gmtime_s failed");
    }
    char buf[21]; // "YYYY-MM-DDTHH:MM:SSZ" + '\0'
    if (std::strftime(buf, sizeof(buf), "%FT%TZ", &tm) == 0) {
      throw std::runtime_error("strftime failed");
    }
    return std::string(buf);
#else
    char buf[21];
    std::tm* gmt = gmtime(&now);
    if (!gmt) {
      throw std::runtime_error("gmtime failed");
    }
    if (std::strftime(buf, sizeof(buf), "%FT%TZ", gmt) == 0) {
      throw std::runtime_error("strftime failed");
    }
    return std::string(buf);
#endif
  }

  /**
   * @brief Time utility: get current timestamp (seconds)
   */
  static int64_t getCurrentTime() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
  }

private:
  /**
   * @brief Check if cache is stale
   */
  bool cacheIsStale() const {
    if (!cachedValue_) {
      return true;
    }
    return getCurrentTime() >= cachedValue_->staleTime;
  }

  /**
   * @brief Check if prefetch should be initiated
   */
  bool shouldInitiateCachePrefetch() const {
    if (!cachedValue_) {
      return true;
    }
    return getCurrentTime() >= cachedValue_->prefetchTime;
  }

  /**
   * @brief Async prefetch refresh
   */
  void prefetchCache() const {
    prefetchStrategy_->prefetch([this]() {
      refreshCache();
    });
  }

  /**
   * @brief Synchronous refresh cache (with lock protection)
   */
  void refreshCache() const {
    std::unique_lock<std::timed_mutex> lock(refreshMutex_, std::defer_lock);
    
    // Try to acquire lock, wait max REFRESH_BLOCKING_MAX_WAIT_MS milliseconds
    if (!lock.try_lock_for(std::chrono::milliseconds(REFRESH_BLOCKING_MAX_WAIT_MS))) {
      // Lock timeout, return using existing cache
      return;
    }

    // Double check: another thread may have already refreshed
    if (!cacheIsStale() && !shouldInitiateCachePrefetch()) {
      return;
    }

    try {
      RefreshResult result = doRefresh();
      cachedValue_ = std::make_shared<RefreshResult>(
          handleFetchedSuccess(result));
    } catch (const std::exception& ex) {
      cachedValue_ = std::make_shared<RefreshResult>(
          handleFetchedFailure(ex));
    }
  }

  /**
   * @brief Handle successful refresh
   */
  RefreshResult handleFetchedSuccess(const RefreshResult& value) const {
    consecutiveRefreshFailures_ = 0;
    int64_t now = getCurrentTime();

    // Case 1: expiration time is more than 15 minutes away, normal case
    if (now < value.staleTime) {
      return value;
    }

    // Case 2: expiration within 15 minutes but not expired, will refresh next time
    if (now < value.staleTime + STALE_TIME_WINDOW) {
      return RefreshResult(value.credential, now, value.prefetchTime);
    }

    // Case 3: credential expired, try using cache
    if (!cachedValue_) {
      throw std::runtime_error("Retrieved expired credential and no cached value available");
    }

    if (now < cachedValue_->staleTime) {
      // Cache not expired, use cache
      return *cachedValue_;
    }

    // Decide how to handle expired cache based on policy
    if (staleValueBehavior_ == StaleValueBehavior::STRICT_) {
      // Strict mode: return cache but set very short expiration (1 second)
      return RefreshResult(cachedValue_->credential, now + 1, cachedValue_->prefetchTime);
    } else {
      // Allow mode: extend expiration time with random jitter
      int64_t jitter = (rand() % 20000 + 50000) / 1000;  // 50-70 seconds
      return RefreshResult(cachedValue_->credential, now + jitter, cachedValue_->prefetchTime);
    }
  }

  /**
   * @brief Handle refresh failure
   */
  RefreshResult handleFetchedFailure(const std::exception& ex) const {
    if (!cachedValue_) {
      throw ex;  // No cache, throw exception
    }

    int64_t now = getCurrentTime();
    if (now < cachedValue_->staleTime) {
      return *cachedValue_;  // Cache not expired, return cache
    }

    consecutiveRefreshFailures_++;

    if (staleValueBehavior_ == StaleValueBehavior::STRICT_) {
      throw ex;  // Strict mode: throw exception
    } else {
      // Allow mode: extend expiration time with exponential backoff
      int64_t backoffMillis = std::max(10000LL, (1LL << (consecutiveRefreshFailures_ - 1)) * 100);
      int64_t jitter = (rand() % (backoffMillis / 2)) + backoffMillis;
      int64_t newStaleTime = now + jitter / 1000;
      
      return RefreshResult(cachedValue_->credential, newStaleTime, cachedValue_->prefetchTime);
    }
  }

  /**
   * @brief Stop background refresh
   */
  void shutdown() {
    // Clean up resources
  }

private:
  // Member variables
  StaleValueBehavior staleValueBehavior_;
  std::shared_ptr<PrefetchStrategy> prefetchStrategy_;
  
  mutable std::atomic<int> consecutiveRefreshFailures_;
  mutable std::shared_ptr<RefreshResult> cachedValue_;
  
  mutable std::timed_mutex refreshMutex_;  // Refresh lock
  mutable std::mutex accessMutex_;   // Access lock
};

} // namespace Credential
} // namespace AlibabaCloud

#endif
