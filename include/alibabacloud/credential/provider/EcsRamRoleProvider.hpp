#ifndef ALIBABACLOUD_CREDENTIAL_EcsRamRoleProvider_HPP_
#define ALIBABACLOUD_CREDENTIAL_EcsRamRoleProvider_HPP_

#include <atomic>
#include <memory>
#include <string>

#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>
#include <alibabacloud/credential/provider/RefreshableProvider.hpp>

namespace AlibabaCloud {
namespace Credential {

/**
 * @brief ECS RAM role credential provider (async refresh version)
 * 
 * Fully corresponds to Python SDK's EcsRamRoleCredentialsProvider implementation:
 * https://github.com/aliyun/credentials-python/blob/master/alibabacloud_credentials/provider/ecs_ram_role.py
 * 
 * Key features:
 * - Background async refresh (NonBlocking strategy)
 * - Scheduled refresh check (should_refresh flag)
 * - IMDSv2/IMDSv1 auto-switch
 * - Custom stale_time and prefetch_time calculation
 * - ALLOW mode tolerates refresh failures
 */
class EcsRamRoleProvider : public RefreshableProvider {
public:
  // Default configuration constants (from Python SDK)
  static constexpr int DEFAULT_METADATA_TOKEN_DURATION = 21600;  // 6 hours
  static constexpr int DEFAULT_CONNECT_TIMEOUT = 1000;            // 1 second
  static constexpr int DEFAULT_READ_TIMEOUT = 1000;               // 1 second

  /**
   * @brief Construct with config
   * 
   * @param config Configuration object
   * @param asyncUpdateEnabled Enable async update (default true, corresponds to Python async_update_enabled)
   * @param behavior Stale value policy (default ALLOW, corresponds to Python)
   * @param strategy Refresh strategy (default NonBlocking, corresponds to Python)
   */
  EcsRamRoleProvider(
      std::shared_ptr<Models::Config> config,
      bool asyncUpdateEnabled = true,
      StaleValueBehavior behavior = StaleValueBehavior::ALLOW_,
      std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>());

  /**
   * @brief Construct with role name
   * 
   * @param roleName Role name
   * @param disableIMDSv1 Disable IMDSv1
   * @param asyncUpdateEnabled Enable async update
   * @param behavior Stale value policy
   * @param strategy Refresh strategy
   */
  EcsRamRoleProvider(
      const std::string& roleName = "",
      bool disableIMDSv1 = false,
      bool asyncUpdateEnabled = true,
      StaleValueBehavior behavior = StaleValueBehavior::ALLOW_,
      std::shared_ptr<PrefetchStrategy> strategy = std::make_shared<NonBlockingPrefetch>());

  virtual ~EcsRamRoleProvider() = default;

  /**
   * @brief Get provider name (corresponds to Python get_provider_name)
   */
  std::string getProviderName() const override { return Constant::ECS_RAM_ROLE; }

protected:
  /**
   * @brief Implement credential refresh logic (corresponds to Python _refresh_credentials)
   */
  virtual RefreshResult doRefresh() const override;

  /**
   * @brief Calculate stale_time (corresponds to Python _get_stale_time)
   * 
   * Python logic:
   * - If expiration < 0: return now + 60 minutes
   * - Otherwise: return expiration - 15 minutes
   */
  int64_t getStaleTime(int64_t expiration) const;

  /**
   * @brief Calculate prefetch_time (corresponds to Python _get_prefetch_time)
   *
   * Python logic:
   * - If expiration < 0: return now + 5 minutes
   * - Otherwise: return now + 60 minutes
   */
  static int64_t getPrefetchTime(int64_t expiration);

private:
  /**
   * @brief Get role name (corresponds to Python _get_role_name)
   */
  std::string getRoleName() const;

  /**
   * @brief Get IMDSv2 Token (corresponds to Python _get_metadata_token)
   */
  std::string getMetadataToken() const;

  // URL constants
  static const std::string URL_IN_ECS_META_DATA;
  static const std::string URL_IN_ECS_METADATA_TOKEN;
  static const std::string META_DATA_SERVICE_HOST;

  // Error messages
  static const std::string ECS_METADATA_FETCH_ERROR_MSG;
  static const std::string ECS_METADATA_TOKEN_FETCH_ERROR_MSG;

  // Member variables
  mutable std::string roleName_;           // Role name
  mutable bool disableIMDSv1_;             // Disable IMDSv1
  mutable std::atomic<bool> shouldRefresh_; // Refresh flag (corresponds to Python _should_refresh)
  bool asyncUpdateEnabled_;                 // Enable async update
  int64_t connectTimeout_;                  // Connection timeout
  int64_t readTimeout_;                     // Read timeout
};

} // namespace Credential
} // namespace AlibabaCloud

#endif
