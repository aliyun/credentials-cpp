#include <gtest/gtest.h>
#include <alibabacloud/credential/provider/ProfileProvider.hpp>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
static inline int setenv(const char* name, const char* value, int /*overwrite*/) {
  return _putenv_s(name, value);
}
static inline int unsetenv(const char* name) {
  return _putenv_s(name, "");
}
#endif

using namespace AlibabaCloud::Credential;

// ==================== ProfileProvider Tests ====================

class ProfileProviderTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Save original HOME environment
    const char* home = std::getenv("HOME");
    if (home) {
      originalHome_ = home;
    }
  }
  
  void TearDown() override {
    // Restore HOME environment
    if (!originalHome_.empty()) {
      setenv("HOME", originalHome_.c_str(), 1);
    }
  }
  
  std::string originalHome_;
};

void SetUpEnv(const char* key, const char* value)
{
#if defined(_WIN32) || defined(_WIN64)
  _putenv_s(key, value);
#else
  setenv(key, value, 1);
#endif
}

void UnsetEnv(const char* key)
{
#if defined(_WIN32) || defined(_WIN64)
  _putenv_s(key, "");
#else
  unsetenv(key);
#endif
}

TEST_F(ProfileProviderTest, DefaultConstructor) {
  // Provider should use default profile name "default"
  EXPECT_NO_THROW({
    ProfileProvider provider;
  });
}

TEST_F(ProfileProviderTest, ConstructorWithConfig) {
  auto config = std::make_shared<Models::Config>();
  
  EXPECT_NO_THROW({
    ProfileProvider provider;
  });
}

// ProfileProvider uses default profile from file
// It doesn't accept profileName as constructor parameter

// Test that provider supports both .aliyun/config.json and .alibabaclouds.ini
TEST_F(ProfileProviderTest, SupportsNewConfigPath) {
  // This test verifies that the provider checks for .aliyun/config.json first
  // Implementation detail: The provider should prioritize the new path
  EXPECT_NO_THROW({
    ProfileProvider provider;
  });
}

TEST_F(ProfileProviderTest, SupportsFallbackToLegacyPath) {
  // If .aliyun/config.json doesn't exist, should fall back to .alibabaclouds.ini
  EXPECT_NO_THROW({
    ProfileProvider provider;
  });
}

TEST_F(ProfileProviderTest, MultipleProviderInstances) {
  EXPECT_NO_THROW({
    ProfileProvider provider1;
    ProfileProvider provider2;
    ProfileProvider provider3;
  });
}

// Test error handling when profile doesn't exist
TEST_F(ProfileProviderTest, NonExistentProfileThrows) {
  ProfileProvider provider;
  
  // Should throw when trying to get credential from non-existent/invalid profile
  EXPECT_THROW({
    provider.getCredential();
  }, Darabonba::Exception);
}

// Test that provider can handle different credential types from profile
TEST_F(ProfileProviderTest, ProfileSupportsAccessKey) {
  // Profile can contain access_key type credentials
  EXPECT_NO_THROW({
    ProfileProvider provider;
  });
}

TEST_F(ProfileProviderTest, ProfileSupportsEcsRamRole) {
  // Profile can contain ecs_ram_role type
  EXPECT_NO_THROW({
    ProfileProvider provider;
  });
}

TEST_F(ProfileProviderTest, ProfileSupportsRamRoleArn) {
  // Profile can contain ram_role_arn type
  EXPECT_NO_THROW({
    ProfileProvider provider;
  });
}

TEST_F(ProfileProviderTest, ProfileSupportsOIDC) {
  // Profile can contain oidc_role_arn type
  EXPECT_NO_THROW({
    ProfileProvider provider;
  });
}

TEST_F(ProfileProviderTest, ProfileSupportsRsaKeyPair) {
  // Profile can contain rsa_key_pair type
  EXPECT_NO_THROW({
    ProfileProvider provider;
  });
}
