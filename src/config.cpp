#include <alibabacloud/credential.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>

using namespace std;

Alibabacloud_Credential::Config::Config(map<string, string> config) {
  typedef map<string, void *>::iterator MapIterator;
  if (config.find("accessKeyId") != config.end()) {
    accessKeyId = config["accessKeyId"];
  }
  if (config.find("accessKeySecret") != config.end()) {
    accessKeySecret = config["accessKeySecret"];
  }
  if (config.find("securityToken") != config.end()) {
    securityToken = config["securityToken"];
  }
  if (config.find("bearerToken") != config.end()) {
    bearerToken = config["bearerToken"];
  }
  if (config.find("durationSeconds") != config.end()) {
    durationSeconds = stoi(config["durationSeconds"]);
  }
  if (config.find("roleArn") != config.end()) {
    roleArn = config["roleArn"];
  }
  if (config.find("policy") != config.end()) {
    policy = config["policy"];
  }
  if (config.find("roleSessionExpiration") != config.end()) {
    roleSessionExpiration = stoi(config["roleSessionExpiration"]);
  }
  if (config.find("roleSessionName") != config.end()) {
    roleSessionName = config["roleSessionName"];
  }
  if (config.find("publicKeyId") != config.end()) {
    publicKeyId = config["publicKeyId"];
  }
  if (config.find("privateKeyFile") != config.end()) {
    privateKeyFile = config["privateKeyFile"];
    ifstream ifs(privateKeyFile);
    string str((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    privateKeySecret = str;
  }
  if (config.find("roleName") != config.end()) {
    roleName = config["roleName"];
  }
  if (config.find("type") != config.end()) {
    type = config["type"];
  } else {
    // default
    type = "access_key";
  }
}
Alibabacloud_Credential::Config::~Config() = default;
Alibabacloud_Credential::Config::Config() = default;
