#include <gtest/gtest.h>
#include <alibabacloud/credential/AuthUtil.hpp>
#include <darabonba/http/Request.hpp>
#include <cstdlib>
#include <thread>
#include <chrono>

using namespace AlibabaCloud::Credential;

// ==================== AuthUtil Tests ====================

class AuthUtilTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Save original ALIBABA_CLOUD_PROFILE environment
    const char* profile = std::getenv("ALIBABA_CLOUD_PROFILE");
    if (profile) {
      originalProfile_ = profile;
    }
  }
  
  void TearDown() override {
    // Restore original environment
    if (!originalProfile_.empty()) {
#if defined(_WIN32) || defined(_WIN64)
      // Windows: set environment variable
      _putenv_s("ALIBABA_CLOUD_PROFILE", originalProfile_.c_str());
#else
      setenv("ALIBABA_CLOUD_PROFILE", originalProfile_.c_str(), 1);
#endif
    } else {
#if defined(_WIN32) || defined(_WIN64)
      // Windows: clear environment variable
      _putenv_s("ALIBABA_CLOUD_PROFILE", "");
#else
      unsetenv("ALIBABA_CLOUD_PROFILE");
#endif
    }
  }
  
  std::string originalProfile_;
};

TEST_F(AuthUtilTest, DefaultClientType) {
  // Default should be "default" if no env var set
#if defined(_WIN32) || defined(_WIN64)
  _putenv_s("ALIBABA_CLOUD_PROFILE", "");
#else
  unsetenv("ALIBABA_CLOUD_PROFILE");
#endif
  
  // Note: clientType_ is initialized at program start
  // We can only test the getter
  std::string type = AuthUtil::clientType();
  EXPECT_FALSE(type.empty());
}

TEST_F(AuthUtilTest, SetAndGetClientType) {
  AuthUtil::setClientType("test_client");
  EXPECT_EQ("test_client", AuthUtil::clientType());
  
  AuthUtil::setClientType("production");
  EXPECT_EQ("production", AuthUtil::clientType());
}

TEST_F(AuthUtilTest, SetClientTypeReturnsTrue) {
  bool result = AuthUtil::setClientType("new_type");
  EXPECT_TRUE(result);
}

TEST_F(AuthUtilTest, SetEmptyClientType) {
  AuthUtil::setClientType("");
  EXPECT_EQ("", AuthUtil::clientType());
}

TEST_F(AuthUtilTest, SetClientTypeWithSpecialCharacters) {
  AuthUtil::setClientType("test-client_123");
  EXPECT_EQ("test-client_123", AuthUtil::clientType());
}

TEST_F(AuthUtilTest, GenerateSessionName) {
  std::string sessionName = AuthUtil::generateSessionName();
  
  // Should start with "credentials-cpp-"
  EXPECT_EQ(0, sessionName.find("credentials-cpp-"));
  
  // Should contain timestamp
  EXPECT_GT(sessionName.length(), 16);
}

TEST_F(AuthUtilTest, GenerateSessionNameUnique) {
  std::string name1 = AuthUtil::generateSessionName();
  
  // Sleep for a millisecond to ensure different timestamp
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  
  std::string name2 = AuthUtil::generateSessionName();
  
  // Names should be different
  EXPECT_NE(name1, name2);
}

TEST_F(AuthUtilTest, GenerateSessionNameFormat) {
  std::string sessionName = AuthUtil::generateSessionName();
  
  // Should be in format: credentials-cpp-{timestamp}
  size_t dashPos = sessionName.find("credentials-cpp-");
  EXPECT_EQ(0, dashPos);
  
  // Extract timestamp part
  std::string timestampPart = sessionName.substr(16);
  EXPECT_FALSE(timestampPart.empty());
  
  // Timestamp part should be numeric
  for (char c : timestampPart) {
    EXPECT_TRUE(isdigit(c));
  }
}

TEST_F(AuthUtilTest, MultipleSessionNamesIncreasing) {
  std::string name1 = AuthUtil::generateSessionName();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  std::string name2 = AuthUtil::generateSessionName();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  std::string name3 = AuthUtil::generateSessionName();
  
  // All should be different
  EXPECT_NE(name1, name2);
  EXPECT_NE(name2, name3);
  EXPECT_NE(name1, name3);
}

TEST_F(AuthUtilTest, ClientTypeThreadSafety) {
  // Test concurrent access to clientType
  std::vector<std::thread> threads;
  
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([i]() {
      std::string type = "type_" + std::to_string(i);
      AuthUtil::setClientType(type);
      std::string result = AuthUtil::clientType();
      // The result may or may not match due to race conditions
      // But should not crash
      EXPECT_FALSE(result.empty());
    });
  }
  
  for (auto& t : threads) {
    t.join();
  }
}

TEST_F(AuthUtilTest, SetClientTypeWithLongString) {
  std::string longType(1000, 'x');
  AuthUtil::setClientType(longType);
  EXPECT_EQ(longType, AuthUtil::clientType());
}

TEST_F(AuthUtilTest, ClientTypePreservesValue) {
  AuthUtil::setClientType("preserved_value");
  
  // Call other methods
  AuthUtil::generateSessionName();
  AuthUtil::generateSessionName();
  
  // Client type should still be preserved
  EXPECT_EQ("preserved_value", AuthUtil::clientType());
}

// ==================== User-Agent Tests ====================

TEST_F(AuthUtilTest, GetSDKVersion) {
  std::string version = AuthUtil::getSDKVersion();
  EXPECT_EQ("0.1.0", version);
  EXPECT_FALSE(version.empty());
}

TEST_F(AuthUtilTest, GetOSName) {
  std::string osName = AuthUtil::getOSName();
  EXPECT_FALSE(osName.empty());
#if defined(__APPLE__) || defined(__MACH__)
  EXPECT_EQ("macOS", osName);
#elif defined(__linux__)
  EXPECT_EQ("Linux", osName);
#elif defined(_WIN32) || defined(_WIN64)
  EXPECT_EQ("Windows", osName);
#endif
}

TEST_F(AuthUtilTest, GetMachineName) {
  std::string machine = AuthUtil::getMachineName();
  EXPECT_FALSE(machine.empty());
  // Should be one of the known architectures
  bool isKnownArch = (machine == "x86_64" || machine == "arm64" || 
                      machine == "i386" || machine == "arm" || machine == "unknown");
  EXPECT_TRUE(isKnownArch);
}

TEST_F(AuthUtilTest, GetCppVersion) {
  std::string cppVer = AuthUtil::getCppVersion();
  EXPECT_FALSE(cppVer.empty());
  // C++ version should be at least 11 (the minimum required by this project)
  // Actual version may be higher (14, 17, 20) depending on compiler defaults or dependencies
  int version = std::stoi(cppVer);
  EXPECT_GE(version, 11);
  EXPECT_LE(version, 23); // Reasonable upper bound for current C++ standards
}

TEST_F(AuthUtilTest, GetUserAgentBasic) {
  std::string ua = AuthUtil::getUserAgent();
  
  // 打印实际的 User-Agent 用于调试
  std::cout << "User-Agent: " << ua << std::endl;
  
  // Format: AlibabaCloud ({os}; {machine}) C++/{version} Credentials/{credentials_version} TeaDSL/2
  EXPECT_EQ(0, ua.find("AlibabaCloud"));
  EXPECT_NE(std::string::npos, ua.find(AuthUtil::getOSName()));
  EXPECT_NE(std::string::npos, ua.find(AuthUtil::getMachineName()));
  EXPECT_NE(std::string::npos, ua.find("C++/" + AuthUtil::getCppVersion()));
  EXPECT_NE(std::string::npos, ua.find("Credentials/0.1.0"));
  EXPECT_NE(std::string::npos, ua.find("TeaDSL/2"));
}

TEST_F(AuthUtilTest, GetUserAgentFormat) {
  std::string ua = AuthUtil::getUserAgent();
  
  // Expected: "AlibabaCloud (macOS; arm64) C++/11 Credentials/0.1.0 TeaDSL/2"
  EXPECT_TRUE(ua.find("AlibabaCloud (") == 0);
  EXPECT_TRUE(ua.find("; ") != std::string::npos);
  EXPECT_TRUE(ua.find(") ") != std::string::npos);
  EXPECT_TRUE(ua.find("C++/") != std::string::npos);
  EXPECT_TRUE(ua.find("Credentials/") != std::string::npos);
  EXPECT_TRUE(ua.find("TeaDSL/2") != std::string::npos);
}

TEST_F(AuthUtilTest, GetUserAgentWithCustomUA) {
  std::string customUA = "MyApp/1.0";
  std::string ua = AuthUtil::getUserAgent(customUA);
  
  EXPECT_NE(std::string::npos, ua.find("AlibabaCloud"));
  EXPECT_NE(std::string::npos, ua.find("TeaDSL/2"));
  EXPECT_NE(std::string::npos, ua.find("MyApp/1.0"));
  // Custom UA should be after TeaDSL/2
  EXPECT_LT(ua.find("TeaDSL/2"), ua.find("MyApp/1.0"));
}

TEST_F(AuthUtilTest, GetUserAgentConsistency) {
  std::string ua1 = AuthUtil::getUserAgent();
  std::string ua2 = AuthUtil::getUserAgent();
  std::string ua3 = AuthUtil::getUserAgent();
  EXPECT_EQ(ua1, ua2);
  EXPECT_EQ(ua2, ua3);
}

TEST_F(AuthUtilTest, GetNewRequestBasic) {
  std::string url = "http://example.com/test";
  auto req = AuthUtil::getNewRequest(url);
  
  EXPECT_TRUE(req.headers().count("User-Agent") > 0);
  std::string ua = req.headers().at("User-Agent");
  EXPECT_EQ(0, ua.find("AlibabaCloud"));
  EXPECT_NE(std::string::npos, ua.find("TeaDSL/2"));
}

TEST_F(AuthUtilTest, GetNewRequestWithCustomUA) {
  std::string url = "http://example.com/test";
  std::string customUA = "TestClient/2.0";
  auto req = AuthUtil::getNewRequest(url, customUA);
  
  EXPECT_TRUE(req.headers().count("User-Agent") > 0);
  std::string ua = req.headers().at("User-Agent");
  EXPECT_NE(std::string::npos, ua.find("AlibabaCloud"));
  EXPECT_NE(std::string::npos, ua.find("TeaDSL/2"));
  EXPECT_NE(std::string::npos, ua.find("TestClient/2.0"));
}

TEST_F(AuthUtilTest, UserAgentCorrespondsToPythonSDK) {
  // Python: AlibabaCloud ({platform.system()}; {platform.machine()}) Python/{version} Credentials/{version} TeaDSL/2
  // C++:    AlibabaCloud ({os}; {machine}) C++/{version} Credentials/{version} TeaDSL/2
  
  std::string ua = AuthUtil::getUserAgent();
  
  EXPECT_TRUE(ua.find("AlibabaCloud (") == 0);
  EXPECT_TRUE(ua.find("; ") != std::string::npos);
  EXPECT_TRUE(ua.find(") ") != std::string::npos);
  EXPECT_TRUE(ua.find("C++/") != std::string::npos);
  EXPECT_TRUE(ua.find("Credentials/") != std::string::npos);
  EXPECT_TRUE(ua.find("TeaDSL/2") != std::string::npos);
}
