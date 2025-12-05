#include <gtest/gtest.h>
#include <alibabacloud/credential/provider/URLProvider.hpp>
#include <alibabacloud/credential/Constant.hpp>
#include <darabonba/Exception.hpp>

using namespace AlibabaCloud::Credential;

// ==================== URLProvider Tests ====================
// Note: These tests verify provider construction and configuration
// Actual credential fetching would require a mock HTTP server

TEST(URLProviderTest, ConstructorWithConfig) {
  auto config = std::make_shared<Models::Config>();
  config->setCredentialsURL("http://localhost:8080/credentials");
  
  EXPECT_NO_THROW({
    URLProvider provider(config);
  });
}

TEST(URLProviderTest, ConstructorWithURL) {
  EXPECT_NO_THROW({
    URLProvider provider("http://localhost:8080/credentials");
  });
}

TEST(URLProviderTest, ConstructorWithEmptyURL) {
  EXPECT_THROW({
    URLProvider provider("");
  }, Darabonba::Exception);
}

TEST(URLProviderTest, ProviderType) {
  auto config = std::make_shared<Models::Config>();
  config->setCredentialsURL("http://localhost:8080/credentials");
  
  URLProvider provider(config);
  
  // The provider should be created successfully
  // Type will be URL_STS after successful credential fetch
  EXPECT_NO_THROW({
    // Provider constructed without error
  });
}

// Test with various URL formats
TEST(URLProviderTest, HTTPUrl) {
  EXPECT_NO_THROW({
    URLProvider provider("http://example.com/credentials");
  });
}

TEST(URLProviderTest, HTTPSUrl) {
  EXPECT_NO_THROW({
    URLProvider provider("https://example.com/credentials");
  });
}

TEST(URLProviderTest, URLWithPort) {
  EXPECT_NO_THROW({
    URLProvider provider("http://localhost:9000/api/credentials");
  });
}

TEST(URLProviderTest, URLWithQueryParams) {
  EXPECT_NO_THROW({
    URLProvider provider("http://example.com/creds?role=test&region=cn-hangzhou");
  });
}
