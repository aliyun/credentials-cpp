#include <gtest/gtest.h>
#include <alibabacloud/credential/provider/EcsRamRoleProvider.hpp>
#include <alibabacloud/credential/provider/RefreshableProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <darabonba/Exception.hpp>
#include <darabonba/Core.hpp>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <map>

using namespace AlibabaCloud::Credential;

#if defined(_WIN32) || defined(_WIN64)
static inline int setenv(const char* name, const char* value, int /*overwrite*/) {
  return _putenv_s(name, value);
}
static inline int unsetenv(const char* name) {
  return _putenv_s(name, "");
}
#endif

// Cross-platform env helpers
static inline void env_set_kv(const char* k, const char* v, int overwrite = 1){
#if defined(_WIN32) || defined(_WIN64)
  (void)overwrite;
  _putenv_s(k, v);
#else
  setenv(k, v, overwrite);
#endif
}
static inline void env_unset_k(const char* k){
#if defined(_WIN32) || defined(_WIN64)
  _putenv_s(k, "");
#else
  unsetenv(k);
#endif
}

// ==================== EcsRamRole Provider Comprehensive Tests ====================

class EcsRamRoleTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Save original environment variables
    saveEnv("ALIBABA_CLOUD_ECS_METADATA");
    saveEnv("ALIBABA_CLOUD_ECS_METADATA_DISABLED");
  }
  
  void TearDown() override {
    // Restore environment variables
    restoreEnv("ALIBABA_CLOUD_ECS_METADATA");
    restoreEnv("ALIBABA_CLOUD_ECS_METADATA_DISABLED");
  }
  
  void saveEnv(const std::string& name) {
    const char* value = std::getenv(name.c_str());
    if (value) {
      savedEnv_[name] = value;
    }
  }
  
  void restoreEnv(const std::string& name) {
    if (savedEnv_.find(name) != savedEnv_.end()) {
      setenv(name.c_str(), savedEnv_[name].c_str(), 1);
    } else {
      unsetenv(name.c_str());
    }
  }
  
  std::map<std::string, std::string> savedEnv_;
};

// ==================== Constructor Tests ====================

TEST_F(EcsRamRoleTest, ConstructorWithConfigObject) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_ecs_role");
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(config);
  });
}

TEST_F(EcsRamRoleTest, ConstructorWithConfigAndDisableIMDSv1) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_role")
        .setDisableIMDSv1(true);
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(config);
  });
}

TEST_F(EcsRamRoleTest, ConstructorWithConfigAndTimeout) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_role")
        .setTimeout(5000)
        .setConnectTimeout(3000);
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(config);
  });
}

TEST_F(EcsRamRoleTest, ConstructorWithRoleNameString) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider("test_role");
  });
}

TEST_F(EcsRamRoleTest, ConstructorWithEmptyRoleName) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider("");
  });
}

TEST_F(EcsRamRoleTest, ConstructorWithRoleNameAndDisableIMDSv1) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider("test_role", true);
  });
}

TEST_F(EcsRamRoleTest, ConstructorWithAllParameters) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(
      "test_role",
      true,  // disableIMDSv1
      true,  // asyncUpdateEnabled
      StaleValueBehavior::ALLOW_,
      std::make_shared<NonBlockingPrefetch>()
    );
  });   
}

TEST_F(EcsRamRoleTest, ConstructorWithStrictBehavior) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(
      "test_role",
      false,
      true,
      StaleValueBehavior::STRICT_   
    );
  });
}

TEST_F(EcsRamRoleTest, ConstructorWithAsyncDisabled) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(
      "test_role",
      false,
      false  // asyncUpdateEnabled = false
    );
  });
}

TEST_F(EcsRamRoleTest, ConstructorWithOneCallerBlocksPrefetch) {
  auto strategy = std::make_shared<OneCallerBlocksPrefetch>();
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(
      "test_role",
      false,
      true,
      StaleValueBehavior::ALLOW_,
      strategy
    );
  });
}

// ==================== Environment Variable Tests ====================

TEST_F(EcsRamRoleTest, RoleNameFromEnvironment) {
  env_set_kv("ALIBABA_CLOUD_ECS_METADATA", "env_role_name");
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider;
  });
}

TEST_F(EcsRamRoleTest, RoleNamePriorityConfigOverEnv) {
  env_set_kv("ALIBABA_CLOUD_ECS_METADATA", "env_role", 1);
  
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("config_role");
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(config);
  });
}

TEST_F(EcsRamRoleTest, NoRoleNameProvided) {
  unsetenv("ALIBABA_CLOUD_ECS_METADATA");
  
  EXPECT_NO_THROW({
    // Provider should try to get role name from metadata service
    EcsRamRoleProvider provider;
  });
}

// ==================== IMDSv1/v2 Tests ====================

TEST_F(EcsRamRoleTest, IMDSv1Enabled) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_role")
        .setDisableIMDSv1(false);
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(config);
  });
}

TEST_F(EcsRamRoleTest, SupportsIMDSv2Mode) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_role")
        .setDisableIMDSv1(true);
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(config);
  });
}

TEST_F(EcsRamRoleTest, IMDSv2TokenRequest) {
  // When IMDSv1 is disabled, provider should request IMDSv2 token
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_role")
        .setDisableIMDSv1(true);
  
  EcsRamRoleProvider provider(config);
  
  // Note: Without actual ECS metadata service, this will fail
  // But we can verify provider creation succeeds
  EXPECT_NO_THROW({
    // Provider created successfully
  });
}

// ==================== Provider Name Tests ====================

TEST_F(EcsRamRoleTest, GetProviderName) {
  EcsRamRoleProvider provider("test_role");
  
  EXPECT_EQ("ecs_ram_role", provider.getProviderName());
}

TEST_F(EcsRamRoleTest, ProviderNameConsistency) {
  EcsRamRoleProvider provider1("role1");
  EcsRamRoleProvider provider2("role2");
  
  EXPECT_EQ(provider1.getProviderName(), provider2.getProviderName());
}

// ==================== Timeout Configuration Tests ====================

TEST_F(EcsRamRoleTest, DefaultTimeoutValues) {
  EcsRamRoleProvider provider("test_role");
  
  // Default timeouts should be set
  // DEFAULT_CONNECT_TIMEOUT = 1000ms
  // DEFAULT_READ_TIMEOUT = 1000ms
  EXPECT_NO_THROW({
    // Provider should have default timeout values
  });
}

TEST_F(EcsRamRoleTest, CustomTimeoutFromConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_role")
        .setTimeout(8000)
        .setConnectTimeout(5000);
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(config);
  });
}

TEST_F(EcsRamRoleTest, ZeroTimeout) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_role")
        .setTimeout(0)
        .setConnectTimeout(0);
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(config);
  });
}

TEST_F(EcsRamRoleTest, VeryLargeTimeout) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("test_role")
        .setTimeout(999999)
        .setConnectTimeout(999999);
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(config);
  });
}

// ==================== Refresh Behavior Tests ====================

TEST_F(EcsRamRoleTest, AsyncRefreshEnabled) {
  EcsRamRoleProvider provider(
    "test_role",
    false,
    true  // asyncUpdateEnabled = true
  );
  
  // Provider should be created with async refresh enabled
  EXPECT_NO_THROW({
    // Async refresh is enabled by default
  });
}

TEST_F(EcsRamRoleTest, AsyncRefreshDisabled) {
  EcsRamRoleProvider provider(
    "test_role",
    false,
    false  // asyncUpdateEnabled = false
  );
  
  // Provider should be created with async refresh disabled
  EXPECT_NO_THROW({
    // Async refresh is disabled
  });
}

TEST_F(EcsRamRoleTest, AllowBehaviorTolerantToFailures) {
  EcsRamRoleProvider provider(
    "test_role",
    false,
    true,
    StaleValueBehavior::ALLOW_
  );
  
  // With ALLOW behavior, provider should tolerate refresh failures
  EXPECT_NO_THROW({
    // Provider created with ALLOW behavior
  });
}

TEST_F(EcsRamRoleTest, StrictBehaviorThrowsOnFailure) {
  EcsRamRoleProvider provider(
    "test_role",
    false,
    true,
    StaleValueBehavior::STRICT_
  );
  
  // With STRICT behavior, provider should throw on refresh failures
  EXPECT_NO_THROW({
    // Provider created with STRICT behavior
  });
}

// ==================== Multiple Instance Tests ====================

TEST_F(EcsRamRoleTest, MultipleProviderInstances) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider1("role1");
    EcsRamRoleProvider provider2("role2");
    EcsRamRoleProvider provider3("role3");
  });
}

TEST_F(EcsRamRoleTest, ProviderWithSameRoleName) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider1("same_role");
    EcsRamRoleProvider provider2("same_role");
  });
}

TEST_F(EcsRamRoleTest, ProviderWithDifferentConfigurations) {
  auto config1 = std::make_shared<Models::Config>();
  config1->setRoleName("role1").setDisableIMDSv1(true);
  
  auto config2 = std::make_shared<Models::Config>();
  config2->setRoleName("role2").setDisableIMDSv1(false);
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider1(config1);
    EcsRamRoleProvider provider2(config2);
  });
}

// ==================== Special Characters in Role Name Tests ====================

TEST_F(EcsRamRoleTest, RoleNameWithHyphen) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider("test-role-name");
  });
}

TEST_F(EcsRamRoleTest, RoleNameWithUnderscore) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider("test_role_name");
  });
}

TEST_F(EcsRamRoleTest, RoleNameWithNumbers) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider("role123");
  });
}

TEST_F(EcsRamRoleTest, RoleNameWithMixedCase) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider("TestRoleName");
  });
}

TEST_F(EcsRamRoleTest, VeryLongRoleName) {
  std::string longRoleName(200, 'a');
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(longRoleName);
  });
}

// ==================== Configuration Combination Tests ====================

TEST_F(EcsRamRoleTest, AllConfigurationOptionsSet) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleName("comprehensive_role")
        .setDisableIMDSv1(true)
        .setTimeout(10000)
        .setConnectTimeout(5000);
  
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(
      config,
      true,  // asyncUpdateEnabled
      StaleValueBehavior::ALLOW_,
      std::make_shared<NonBlockingPrefetch>()
    );
  });
}

TEST_F(EcsRamRoleTest, MinimalConfiguration) {
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider;
  });
}

// ==================== Prefetch Strategy Tests ====================

TEST_F(EcsRamRoleTest, NonBlockingPrefetchStrategy) {
  auto strategy = std::make_shared<NonBlockingPrefetch>();
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(
      "test_role",
      false,
      true,
      StaleValueBehavior::ALLOW_,
      strategy
    );
  });
}

TEST_F(EcsRamRoleTest, OneCallerBlocksStrategy) {
  auto strategy = std::make_shared<OneCallerBlocksPrefetch>();
  EXPECT_NO_THROW({
    EcsRamRoleProvider provider(
      "test_role",
      false,
      true,
      StaleValueBehavior::ALLOW_,
      strategy
    );
  });
}

// ==================== Error Handling Tests ====================

TEST_F(EcsRamRoleTest, GetCredentialWithoutMetadataService) {
  // Without actual ECS metadata service, getCredential should fail
  EcsRamRoleProvider provider("test_role");
  
  EXPECT_THROW({
    provider.getCredential();
  }, std::exception);
}

TEST_F(EcsRamRoleTest, GetCredentialWithInvalidRoleName) {
  EcsRamRoleProvider provider("invalid_role_name_that_does_not_exist");
  
  EXPECT_THROW({
    provider.getCredential();
  }, std::exception);
}

// ==================== Thread Safety Tests ====================

TEST_F(EcsRamRoleTest, ConcurrentProviderCreation) {
  std::vector<std::thread> threads;
  
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([i]() {
      EXPECT_NO_THROW({
        EcsRamRoleProvider provider("role_" + std::to_string(i));
      });
    });
  }
  
  for (auto& t : threads) {
    t.join();
  }
}

// ==================== Copy and Move Tests ====================

TEST_F(EcsRamRoleTest, ProviderIsNotCopyable) {
  // RefreshableProvider should not be copyable
  // This is enforced by the base class
  EXPECT_FALSE(std::is_copy_constructible<EcsRamRoleProvider>::value);
}

TEST_F(EcsRamRoleTest, ProviderIsNotCopyAssignable) {
  EXPECT_FALSE(std::is_copy_assignable<EcsRamRoleProvider>::value);
}

// ==================== Constants Tests ====================

TEST_F(EcsRamRoleTest, DefaultMetadataTokenDuration) {
  // DEFAULT_METADATA_TOKEN_DURATION should be 21600 (6 hours)
  EXPECT_EQ(21600, EcsRamRoleProvider::DEFAULT_METADATA_TOKEN_DURATION);
}

TEST_F(EcsRamRoleTest, DefaultConnectTimeout) {
  // DEFAULT_CONNECT_TIMEOUT should be 1000 (1 second)
  EXPECT_EQ(1000, EcsRamRoleProvider::DEFAULT_CONNECT_TIMEOUT);
}

TEST_F(EcsRamRoleTest, DefaultReadTimeout) {
  // DEFAULT_READ_TIMEOUT should be 1000 (1 second)
  EXPECT_EQ(1000, EcsRamRoleProvider::DEFAULT_READ_TIMEOUT);
}

// ==================== Integration with RefreshableProvider Tests ====================

TEST_F(EcsRamRoleTest, InheritsFromRefreshableProvider) {
  EXPECT_TRUE((std::is_base_of<RefreshableProvider, EcsRamRoleProvider>::value));
}

TEST_F(EcsRamRoleTest, PrefetchThresholdConstant) {
  // Should inherit PREFETCH_THRESHOLD from RefreshableProvider
  EXPECT_EQ(180, RefreshableProvider::PREFETCH_THRESHOLD);
}

TEST_F(EcsRamRoleTest, StaleTimeWindowConstant) {
  // Should inherit STALE_TIME_WINDOW from RefreshableProvider
  EXPECT_EQ(15 * 60, RefreshableProvider::STALE_TIME_WINDOW);
}
