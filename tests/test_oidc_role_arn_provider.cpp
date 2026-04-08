#include <gtest/gtest.h>
#include <alibabacloud/credentials/provider/OIDCRoleArnProvider.hpp>
#include <alibabacloud/credentials/Constant.hpp>
#include <alibabacloud/credentials/Exception.hpp>
#include <darabonba/Env.hpp>
#include <fstream>
#include <memory>

using namespace AlibabaCloud::Credentials;

// ==================== OIDCRoleArnProvider Constructor Tests ====================

class OIDCRoleArnProviderFullTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create a temporary OIDC token file for testing
    tokenFilePath_ = "/tmp/test_oidc_token_" + std::to_string(std::time(nullptr)) + ".txt";
    std::ofstream tokenFile(tokenFilePath_);
    tokenFile << "test_oidc_token_content_for_unit_test";
    tokenFile.close();
  }

  void TearDown() override {
    // Cleanup temporary files
    if (!tokenFilePath_.empty()) {
      std::remove(tokenFilePath_.c_str());
    }
    // Clear environment variables
    Darabonba::Env::unsetEnv(Constant::ENV_ROLE_ARN);
    Darabonba::Env::unsetEnv(Constant::ENV_OIDC_PROVIDER_ARN);
    Darabonba::Env::unsetEnv(Constant::ENV_OIDC_TOKEN_FILE);
    Darabonba::Env::unsetEnv(Constant::ENV_ROLE_SESSION_NAME);
    Darabonba::Env::unsetEnv(Constant::ENV_STS_REGION);
    Darabonba::Env::unsetEnv(Constant::ENV_VPC_ENDPOINT_ENABLED);
  }

  std::string tokenFilePath_;
};

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithConfigBasic) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setRoleSessionName("test-session")
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithConfigAllOptions) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setRoleSessionName("test-session")
        .setPolicy("{\"Version\":\"1\",\"Statement\":[]}")
        .setDurationSeconds(7200)
        .setStsRegionId("cn-shanghai")
        .setStsEndpoint("sts.cn-shanghai.aliyuncs.com")
        .setEnableVpc(true)
        .setConnectTimeout(15000)
        .setTimeout(8000)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithConfigAndRegionId) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setRegionId("cn-beijing")
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithParametersBasic) {
  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(
        "acs:ram::123456789012:role/test-role",
        "acs:ram::123456789012:oidc-provider/test-provider",
        tokenFilePath_,
        "test-session"
    );
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithParametersAllOptions) {
  auto policy = std::make_shared<std::string>("{\"Version\":\"1\"}");

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(
        "acs:ram::123456789012:role/test-role",
        "acs:ram::123456789012:oidc-provider/test-provider",
        tokenFilePath_,
        "custom-session",
        policy,
        3600,
        "cn-hangzhou",
        "sts.aliyuncs.com"
    );
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithParametersDefaultSessionName) {
  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(
        "acs:ram::123456789012:role/test-role",
        "acs:ram::123456789012:oidc-provider/test-provider",
        tokenFilePath_
        // roleSessionName defaults to "defaultSessionName"
    );
  });
}

// ==================== Environment Variable Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithEnvRoleArn) {
  // Set environment variables
  Darabonba::Env::setEnv(Constant::ENV_ROLE_ARN, "acs:ram::env-test:role/env-role");

  auto config = std::make_shared<Models::Config>();
  config->setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);
  // No roleArn set in config, should use env

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithEnvOidcProviderArn) {
  Darabonba::Env::setEnv(Constant::ENV_OIDC_PROVIDER_ARN, "acs:ram::env-test:oidc-provider/env-provider");

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithEnvOidcTokenFile) {
  Darabonba::Env::setEnv(Constant::ENV_OIDC_TOKEN_FILE, tokenFilePath_.c_str());

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithEnvRoleSessionName) {
  Darabonba::Env::setEnv(Constant::ENV_ROLE_SESSION_NAME, "env-session-name");

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithEnvStsRegion) {
  Darabonba::Env::setEnv(Constant::ENV_STS_REGION, "cn-shenzhen");

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithEnvVpcEnabled) {
  Darabonba::Env::setEnv(Constant::ENV_VPC_ENDPOINT_ENABLED, "true");

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithAllEnvVariables) {
  Darabonba::Env::setEnv(Constant::ENV_ROLE_ARN, "acs:ram::env-all:role/env-role");
  Darabonba::Env::setEnv(Constant::ENV_OIDC_PROVIDER_ARN, "acs:ram::env-all:oidc-provider/env-provider");
  Darabonba::Env::setEnv(Constant::ENV_OIDC_TOKEN_FILE, tokenFilePath_.c_str());
  Darabonba::Env::setEnv(Constant::ENV_ROLE_SESSION_NAME, "env-all-session");
  Darabonba::Env::setEnv(Constant::ENV_STS_REGION, "cn-qingdao");
  Darabonba::Env::setEnv(Constant::ENV_VPC_ENDPOINT_ENABLED, "true");

  // Empty config should use all env variables
  auto config = std::make_shared<Models::Config>();
  config->setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

// ==================== Config Priority Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, ConfigPriorityOverEnvRoleArn) {
  Darabonba::Env::setEnv(Constant::ENV_ROLE_ARN, "acs:ram::env:role/env-role");

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::config:role/config-role")  // Config should take priority
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConfigPriorityOverEnvOidcProviderArn) {
  Darabonba::Env::setEnv(Constant::ENV_OIDC_PROVIDER_ARN, "acs:ram::env:oidc-provider/env-provider");

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::config:oidc-provider/config-provider")  // Config priority
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConfigPriorityOverEnvOidcTokenFile) {
  // Create another token file for env
  std::string envTokenPath = "/tmp/test_oidc_env_token.txt";
  std::ofstream envTokenFile(envTokenPath);
  envTokenFile << "env_token";
  envTokenFile.close();

  Darabonba::Env::setEnv(Constant::ENV_OIDC_TOKEN_FILE, envTokenPath.c_str());

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)  // Config path should be used
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });

  std::remove(envTokenPath.c_str());
}

TEST_F(OIDCRoleArnProviderFullTest, ConfigPriorityOverEnvRoleSessionName) {
  Darabonba::Env::setEnv(Constant::ENV_ROLE_SESSION_NAME, "env-session-name");

  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setRoleSessionName("config-session-name")  // Config priority
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

// ==================== Provider Name Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, GetProviderName) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  OIDCRoleArnProvider provider(config);
  EXPECT_EQ(Constant::OIDC_ROLE_ARN, provider.getProviderName());
}

TEST_F(OIDCRoleArnProviderFullTest, GetProviderNameWithParameters) {
  OIDCRoleArnProvider provider(
      "acs:ram::123456789012:role/test-role",
      "acs:ram::123456789012:oidc-provider/test-provider",
      tokenFilePath_
  );
  EXPECT_EQ(Constant::OIDC_ROLE_ARN, provider.getProviderName());
}

// ==================== Error Handling Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, EmptyRoleArnThrowsException) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("")  // Empty roleArn
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  // Should throw CredentialException during construction
  EXPECT_THROW({
    OIDCRoleArnProvider provider(config);
  }, CredentialException);
}

TEST_F(OIDCRoleArnProviderFullTest, EmptyOidcProviderArnThrowsException) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("")  // Empty oidcProviderArn
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  // Should throw CredentialException during construction
  EXPECT_THROW({
    OIDCRoleArnProvider provider(config);
  }, CredentialException);
}

TEST_F(OIDCRoleArnProviderFullTest, EmptyOidcTokenFilePathThrowsException) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath("")  // Empty path
        .setType(Constant::OIDC_ROLE_ARN);

  // Should throw CredentialException during construction
  EXPECT_THROW({
    OIDCRoleArnProvider provider(config);
  }, CredentialException);
}

TEST_F(OIDCRoleArnProviderFullTest, EmptyRoleSessionNameFallsBackToDefault) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setRoleSessionName("")  // Empty session name
        .setType(Constant::OIDC_ROLE_ARN);

  // Should not throw, falls back to default
  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

// ==================== Policy Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithPolicy) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setPolicy("{\"Version\":\"1\",\"Statement\":[{\"Effect\":\"Allow\",\"Action\":\"*\",\"Resource\":\"*\"}]}")
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithParametersAndPolicy) {
  auto policy = std::make_shared<std::string>("{\"Version\":\"1\"}");

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(
        "acs:ram::123456789012:role/test-role",
        "acs:ram::123456789012:oidc-provider/test-provider",
        tokenFilePath_,
        "session",
        policy
    );
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithNullPolicy) {
  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(
        "acs:ram::123456789012:role/test-role",
        "acs:ram::123456789012:oidc-provider/test-provider",
        tokenFilePath_,
        "session",
        nullptr  // null policy
    );
  });
}

// ==================== Timeout Configuration Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithCustomTimeouts) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setConnectTimeout(20000)
        .setTimeout(10000)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithDefaultTimeouts) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);
  // No timeout set, should use defaults

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

// ==================== VPC Configuration Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithEnableVpcTrue) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setEnableVpc(true)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithEnableVpcFalse) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setEnableVpc(false)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

// ==================== Duration Seconds Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithCustomDurationSeconds) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setDurationSeconds(7200)  // 2 hours
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithParametersCustomDuration) {
  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(
        "acs:ram::123456789012:role/test-role",
        "acs:ram::123456789012:oidc-provider/test-provider",
        tokenFilePath_,
        "session",
        nullptr,
        1800  // 30 minutes
    );
  });
}

// ==================== STS Endpoint Configuration Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithCustomStsEndpoint) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setStsEndpoint("sts.cn-hongkong.aliyuncs.com")
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithParametersCustomEndpoint) {
  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(
        "acs:ram::123456789012:role/test-role",
        "acs:ram::123456789012:oidc-provider/test-provider",
        tokenFilePath_,
        "session",
        nullptr,
        3600,
        "cn-shanghai",
        "sts.cn-shanghai.aliyuncs.com"
    );
  });
}

// ==================== STS Region Configuration Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithStsRegionId) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setStsRegionId("cn-zhangjiakou")
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, ConstructorWithRegionIdFallback) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setRegionId("cn-chengdu")  // RegionId as fallback for StsRegionId
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

// ==================== Multiple Instances Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, MultipleProviders) {
  std::string tokenPath2 = "/tmp/test_oidc_token_2.txt";
  std::ofstream tokenFile2(tokenPath2);
  tokenFile2 << "second_token";
  tokenFile2.close();

  auto config1 = std::make_shared<Models::Config>();
  config1->setRoleArn("acs:ram::111:role/role1")
         .setOidcProviderArn("acs:ram::111:oidc-provider/provider1")
         .setOidcTokenFilePath(tokenFilePath_)
         .setRoleSessionName("session1")
         .setType(Constant::OIDC_ROLE_ARN);

  auto config2 = std::make_shared<Models::Config>();
  config2->setRoleArn("acs:ram::222:role/role2")
         .setOidcProviderArn("acs:ram::222:oidc-provider/provider2")
         .setOidcTokenFilePath(tokenPath2)
         .setRoleSessionName("session2")
         .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider1(config1);
    OIDCRoleArnProvider provider2(config2);
  });

  std::remove(tokenPath2.c_str());
}

// ==================== Shared Pointer Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, SharedPtrProvider) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  auto provider = std::make_shared<OIDCRoleArnProvider>(config);
  EXPECT_NE(provider, nullptr);
  EXPECT_EQ(Constant::OIDC_ROLE_ARN, provider->getProviderName());
}

// ==================== Edge Cases ====================

TEST_F(OIDCRoleArnProviderFullTest, LongRoleArn) {
  std::string longRoleArn = "acs:ram::" + std::string(100, '0') + ":role/very-long-role-name";
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn(longRoleArn)
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, LongSessionName) {
  std::string longSessionName = "very-long-session-name-" + std::string(50, 'x');
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setRoleSessionName(longSessionName)
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

TEST_F(OIDCRoleArnProviderFullTest, SpecialCharactersInPolicy) {
  auto config = std::make_shared<Models::Config>();
  config->setRoleArn("acs:ram::123456789012:role/test-role")
        .setOidcProviderArn("acs:ram::123456789012:oidc-provider/test-provider")
        .setOidcTokenFilePath(tokenFilePath_)
        .setPolicy("{\"Version\":\"1\",\"Statement\":[{\"Effect\":\"Allow\",\"Action\":\"oss:GetObject\",\"Resource\":\"acs:oss:*:*:bucket/path/file.txt\"}]}")
        .setType(Constant::OIDC_ROLE_ARN);

  EXPECT_NO_THROW({
    OIDCRoleArnProvider provider(config);
  });
}

// ==================== Type Constant Tests ====================

TEST_F(OIDCRoleArnProviderFullTest, VerifyConstantValue) {
  EXPECT_EQ(Constant::OIDC_ROLE_ARN, "oidc_role_arn");
}

TEST_F(OIDCRoleArnProviderFullTest, VerifyEnvConstantValues) {
  EXPECT_EQ(Constant::ENV_ROLE_ARN, "ALIBABA_CLOUD_ROLE_ARN");
  EXPECT_EQ(Constant::ENV_OIDC_PROVIDER_ARN, "ALIBABA_CLOUD_OIDC_PROVIDER_ARN");
  EXPECT_EQ(Constant::ENV_OIDC_TOKEN_FILE, "ALIBABA_CLOUD_OIDC_TOKEN_FILE");
  EXPECT_EQ(Constant::ENV_ROLE_SESSION_NAME, "ALIBABA_CLOUD_ROLE_SESSION_NAME");
}