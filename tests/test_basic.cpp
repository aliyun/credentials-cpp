#include <gtest/gtest.h>
#include <alibabacloud/credential/Constant.hpp>
#include <alibabacloud/credential/Model.hpp>

using namespace AlibabaCloud::Credential;

// ==================== Basic Constant Tests ====================

TEST(BasicConstantTest, CloudSSOType) {
  EXPECT_EQ("sso", Constant::CLOUD_SSO);
}

TEST(BasicConstantTest, OAuthType) {
  EXPECT_EQ("oauth", Constant::OAUTH);
}

// ==================== Basic Model Tests ====================

TEST(BasicModelTest, TimeoutDefaults) {
  Models::Config config;
  EXPECT_EQ(5000, config.getTimeout());
  EXPECT_EQ(10000, config.getConnectTimeout());
  EXPECT_FALSE(config.getDisableIMDSv1());
}

TEST(BasicModelTest, SetTimeout) {
  Models::Config config;
  config.setTimeout(8000);
  EXPECT_EQ(8000, config.getTimeout());
}

TEST(BasicModelTest, SetDisableIMDSv1) {
  Models::Config config;
  config.setDisableIMDSv1(true);
  EXPECT_TRUE(config.getDisableIMDSv1());
}

TEST(BasicModelTest, ConfigToMap) {
  Models::Config config;
  config.setTimeout(7000)
        .setConnectTimeout(14000)
        .setDisableIMDSv1(true);
  
  auto json = config.toMap();
  
  EXPECT_EQ(7000, json["timeout"].get<int64_t>());
  EXPECT_EQ(14000, json["connectTimeout"].get<int64_t>());
  EXPECT_TRUE(json["disableIMDSv1"].get<bool>());
}
