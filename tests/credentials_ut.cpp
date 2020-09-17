#include "gtest/gtest.h"
#include <alibabacloud/credential.hpp>
#include <alibabacloud/credential_utils.hpp>
#include <json/json.h>

using namespace Alibabacloud_Credential;

TEST(tests_credential, access_key) {
  map<string, string *> m;
  m.insert(pair<string, string *>("type", new string("access_key")));
  m.insert(
      pair<string, string *>("accessKeyId", new string("fakeAccessKeyId")));
  m.insert(pair<string, string *>("accessKeySecret",
                                  new string("fakeAccessKeySecret")));

  Client client = Client(new Config(m));

  ASSERT_EQ(string("fakeAccessKeyId"), client.getAccessKeyId());
}

TEST(tests_credential, bearer_token) {
  map<string, string *> m;

  m.insert(pair<string, string *>("type", new string("bearer_token")));
  m.insert(pair<string, string *>("bearerToken", new string("<BearerToken>")));

  Client client = Client(new Config(m));

  ASSERT_EQ("<BearerToken>", client.getBearerToken());
}

TEST(tests_credential, sts) {
  map<string, string *> m;
  m.insert(pair<string, string *>("type", new string("sts")));
  m.insert(pair<string, string *>("accessKeyId", new string("<AccessKeyId>")));
  m.insert(pair<string, string *>("accessKeySecret",
                                  new string("<AccessKeySecret>")));
  m.insert(
      pair<string, string *>("securityToken", new string("<SecurityToken>")));

  Client client = Client(new Config(m));

  ASSERT_EQ("<AccessKeyId>", client.getAccessKeyId());
  ASSERT_EQ("<AccessKeySecret>", client.getAccessKeySecret());
  ASSERT_EQ("<SecurityToken>", client.getSecurityToken());
}

TEST(tests_credential, ram_role_arn) {
  map<string, string *> m;
  m.insert(pair<string, string *>("type", new string("ram_role_arn")));
  m.insert(pair<string, string *>("accessKeyId",
                                  new string("ram_role_arn_access_key_id")));
  m.insert(pair<string, string *>(
      "accessKeySecret", new string("ram_role_arn_access_key_secret")));
  m.insert(pair<string, string *>("roleArn", new string("<RoleArn>")));
  m.insert(pair<string, string *>("roleSessionName",
                                  new string("<RoleSessionName>")));
  m.insert(pair<string, string *>("policy", new string("<Policy>")));
  auto *config = new Config(m);
  Client client = Client(config);

  http_response response(status_codes::OK);
  response.set_body(
      R"({"Code":"Success","Credentials":{"AccessKeyId":"MockAccessKeyId","AccessKeySecret":"MockAccessKeySecret","SecurityToken":"MockSecurityToken","Expiration":"2222-08-08T08:08:08Z"}})");
  pushMockHttpResponse(response);

  ASSERT_EQ(string("MockAccessKeyId"), client.getAccessKeyId());
  ASSERT_EQ("MockAccessKeySecret", client.getAccessKeySecret());
  ASSERT_EQ("MockSecurityToken", client.getSecurityToken());
  ASSERT_EQ("<RoleArn>", client.getRoleArn());
  ASSERT_EQ("<RoleSessionName>", client.getRoleSessionName());
  ASSERT_EQ("<Policy>", client.getPolicy());
}

TEST(tests_credential, ecs_ram_role) {
  map<string, string *> m;
  m.insert(pair<string, string *>("type", new string("ecs_ram_role")));
  m.insert(pair<string, string *>("accessKeyId",
                                  new string("ecs_ram_role_access_key_id")));
  m.insert(pair<string, string *>(
      "accessKeySecret", new string("ecs_ram_role_access_key_secret")));

  Client client = Client(new Config(m));

  http_response response(status_codes::OK);
  response.set_body(
      R"({"Code":"Success","AccessKeyId":"MockAccessKeyId","AccessKeySecret":"MockAccessKeySecret","SecurityToken":"MockSecurityToken","Expiration":"2222-08-08T08:08:08Z"})");
  pushMockHttpResponse(response);
  http_response role_name_res(status_codes::OK);
  role_name_res.set_body("MockRoleName");
  pushMockHttpResponse(role_name_res);

  ASSERT_EQ("MockAccessKeyId", client.getAccessKeyId());
  ASSERT_EQ("MockAccessKeySecret", client.getAccessKeySecret());
  ASSERT_EQ("MockRoleName", client.getRoleName());
}

TEST(tests_credential, rsa_key_pair) {
  map<string, string *> m;
  m.insert(pair<string, string *>("type", new string("rsa_key_pair")));
  m.insert(pair<string, string *>("publicKeyId", new string("<PublicKeyId>")));
  m.insert(
      pair<string, string *>("privateKeyFile", new string("<PrivateKeyFile>")));
  m.insert(pair<string, string *>("accessKeySecret",
                                  new string("defaultAccessKeySecret")));

  Client client = Client(new Config(m));

  http_response response(status_codes::OK);
  response.set_body(
      R"({"Code":"Success","Credentials":{"AccessKeyId":"MockAccessKeyId","AccessKeySecret":"MockAccessKeySecret","SecurityToken":"MockSecurityToken","Expiration":"2222-08-08T08:08:08Z"}})");
  pushMockHttpResponse(response);
  ASSERT_EQ("<PublicKeyId>", client.getPublicKeyId());
  ASSERT_EQ("", client.getPrivateKey());
  ASSERT_EQ("MockAccessKeyId", client.getAccessKeyId());
  ASSERT_EQ("MockAccessKeySecret", client.getAccessKeySecret());
  ASSERT_EQ("MockSecurityToken", client.getSecurityToken());
}

TEST(tests_credential, config) {
  map<string, string *> m;
  m.insert(pair<string, string *>("durationSeconds", new string("10000")));
  m.insert(
      pair<string, string *>("roleSessionExpiration", new string("10000")));
  m.insert(pair<string, string *>("roleName", new string("fake-role-name")));
  auto *config = new Alibabacloud_Credential::Config(m);
  ASSERT_EQ(10000, *config->durationSeconds);
  ASSERT_EQ(10000, *config->roleSessionExpiration);
  ASSERT_EQ(string("fake-role-name"), *config->roleName);
  delete config;
}