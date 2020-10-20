#include <alibabacloud/credential.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>

using namespace std;

Alibabacloud_Credential::Config::Config(map<string, string> config) {
  if (config.find("accessKeyId") != config.end()) {
    accessKeyId = make_shared<string>(config.at("accessKeyId"));
  }
  if (config.find("accessKeySecret") != config.end()) {
    accessKeySecret = make_shared<string>(config.at("accessKeySecret"));
  }
  if (config.find("securityToken") != config.end()) {
    securityToken = make_shared<string>(config.at("securityToken"));
  }
  if (config.find("bearerToken") != config.end()) {
    bearerToken = make_shared<string>(config.at("bearerToken"));
  }
  if (config.find("durationSeconds") != config.end()) {
    durationSeconds = make_shared<int>(stoi(config.at("durationSeconds")));
  }
  if (config.find("roleArn") != config.end()) {
    roleArn = make_shared<string>(config.at("roleArn"));
  }
  if (config.find("policy") != config.end()) {
    policy = make_shared<string>(config.at("policy"));
  }
  if (config.find("roleSessionExpiration") != config.end()) {
    roleSessionExpiration = make_shared<int>(stoi(config.at("roleSessionExpiration")));
  }
  if (config.find("roleSessionName") != config.end()) {
    roleSessionName = make_shared<string>(config.at("roleSessionName"));
  }
  if (config.find("publicKeyId") != config.end()) {
    publicKeyId = make_shared<string>(config.at("publicKeyId"));
  }
  if (config.find("privateKeyFile") != config.end()) {
    privateKeyFile = make_shared<string>(config.at("privateKeyFile"));
    ifstream ifs(*privateKeyFile);
    string str((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    privateKeySecret = make_shared<string>(str);
  }
  if (config.find("roleName") != config.end()) {
    roleName = make_shared<string>(config.at("roleName"));
  }
  if (config.find("type") != config.end()) {
    type = make_shared<string>(config.at("type"));
  } else {
    // default
    type = make_shared<string>("access_key");
  }
}
Alibabacloud_Credential::Config::~Config() = default;;
Alibabacloud_Credential::Config::Config() = default;
