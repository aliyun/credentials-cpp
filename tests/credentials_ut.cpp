#include "gtest/gtest.h"
#include <alibabacloud/credential.hpp>
#include <alibabacloud/credential_utils.hpp>
#include <json/json.h>

TEST(tests_credential, access_key) {
  map<string, string> m;
  m.insert(pair<string, string>("type", "access_key"));
  m.insert(pair<string, string>("accessKeyId", "fakeAccessKeyId"));
  m.insert(pair<string, string>("accessKeySecret", "fakeAccessKeySecret"));
  auto *config = new Alibabacloud_Credential::Config(m);
  auto *client = new Alibabacloud_Credential::Client(config);
  ASSERT_EQ(string("fakeAccessKeyId"), *client->getAccessKeyId());
  delete client;
}

TEST(tests_credential, bearer_token) {
  map<string, string> m;
  m.insert(pair<string, string>("type", "bearer_token"));
  m.insert(pair<string, string>("bearerToken", "<BearerToken>"));
  auto *config = new Alibabacloud_Credential::Config(m);
  auto *client = new Alibabacloud_Credential::Client(config);
  ASSERT_EQ("<BearerToken>", *client->getBearerToken());
  delete client;
}

TEST(tests_credential, sts) {
  map<string, string> m;
  m.insert(pair<string, string>("type", "sts"));
  m.insert(pair<string, string>("accessKeyId", "<AccessKeyId>"));
  m.insert(pair<string, string>("accessKeySecret", "<AccessKeySecret>"));
  m.insert(pair<string, string>("securityToken", "<SecurityToken>"));
  auto *config = new Alibabacloud_Credential::Config(m);
  auto *client = new Alibabacloud_Credential::Client(config);
  ASSERT_EQ("<AccessKeyId>", *client->getAccessKeyId());
  ASSERT_EQ("<AccessKeySecret>", *client->getAccessKeySecret());
  ASSERT_EQ("<SecurityToken>", *client->getSecurityToken());
  delete client;
}

TEST(tests_credential, ram_role_arn) {
  map<string, string> m;
  m.insert(pair<string, string>("type", "ram_role_arn"));
  m.insert(pair<string, string>("accessKeyId", "ram_role_arn_access_key_id"));
  m.insert(pair<string, string>("accessKeySecret",
                                "ram_role_arn_access_key_secret"));
  m.insert(pair<string, string>("roleArn", "<RoleArn>"));
  m.insert(pair<string, string>("roleSessionName", "<RoleSessionName>"));
  m.insert(pair<string, string>("policy", "<Policy>"));
  auto *config = new Alibabacloud_Credential::Config(m);
  auto *client = new Alibabacloud_Credential::Client(config);
  http_response response(status_codes::OK);
  response.set_body(
      R"({"Code":"Success","Credentials":{"AccessKeyId":"MockAccessKeyId","AccessKeySecret":"MockAccessKeySecret","SecurityToken":"MockSecurityToken","Expiration":"2222-08-08T08:08:08Z"}})");
  pushMockHttpResponse(response);
  ASSERT_EQ("MockAccessKeyId", *client->getAccessKeyId());
  ASSERT_EQ("MockAccessKeySecret", *client->getAccessKeySecret());
  ASSERT_EQ("MockSecurityToken", *client->getSecurityToken());
  ASSERT_EQ("<RoleArn>", *client->getRoleArn());
  ASSERT_EQ("<RoleSessionName>", *client->getRoleSessionName());
  ASSERT_EQ("<Policy>", *client->getPolicy());
  delete client;
}

TEST(tests_credential, ecs_ram_role) {
  map<string, string> m;
  m.insert(pair<string, string>("type", "ecs_ram_role"));
  m.insert(pair<string, string>("accessKeyId", "ecs_ram_role_access_key_id"));
  m.insert(pair<string, string>("accessKeySecret",
                                "ecs_ram_role_access_key_secret"));
  auto *config = new Alibabacloud_Credential::Config(m);
  auto *client = new Alibabacloud_Credential::Client(config);
  http_response response(status_codes::OK);
  response.set_body(
      R"({"Code":"Success","AccessKeyId":"MockAccessKeyId","AccessKeySecret":"MockAccessKeySecret","SecurityToken":"MockSecurityToken","Expiration":"2222-08-08T08:08:08Z"})");
  pushMockHttpResponse(response);
  http_response role_name_res(status_codes::OK);
  role_name_res.set_body("MockRoleName");
  pushMockHttpResponse(role_name_res);
  ASSERT_EQ("MockAccessKeyId", *client->getAccessKeyId());
  ASSERT_EQ("MockAccessKeySecret", *client->getAccessKeySecret());
  ASSERT_EQ("MockRoleName", *client->getRoleName());
  delete client;
}

TEST(tests_credential, rsa_key_pair) {
  map<string, string> m;
  m.insert(pair<string, string>("type", "rsa_key_pair"));
  m.insert(pair<string, string>("publicKeyId", "<PublicKeyId>"));
  m.insert(pair<string, string>("privateKeyFile", "<PrivateKeyFile>"));
  m.insert(pair<string, string>("accessKeySecret", "defaultAccessKeySecret"));
  auto *config = new Alibabacloud_Credential::Config(m);
  auto *client = new Alibabacloud_Credential::Client(config);
  http_response response(status_codes::OK);
  response.set_body(
      R"({"Code":"Success","Credentials":{"AccessKeyId":"MockAccessKeyId","AccessKeySecret":"MockAccessKeySecret","SecurityToken":"MockSecurityToken","Expiration":"2222-08-08T08:08:08Z"}})");
  pushMockHttpResponse(response);
  ASSERT_EQ("<PublicKeyId>", *client->getPublicKeyId());
  ASSERT_EQ("", *client->getPrivateKey());
  ASSERT_EQ("MockAccessKeyId", *client->getAccessKeyId());
  ASSERT_EQ("MockAccessKeySecret", *client->getAccessKeySecret());
  ASSERT_EQ("MockSecurityToken", *client->getSecurityToken());
  delete client;
}