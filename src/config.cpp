#include <alibabacloud/credential.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>

using namespace std;

Alibabacloud_Credential::Config::Config(const shared_ptr<map<string, string>>& config) {
  typedef map<string, void *>::iterator MapIterator;
  map<string, string> conf =
      !config ? map<string, string>() : *config;
  if (conf.find("accessKeyId") != conf.end()) {
    accessKeyId = make_shared<string>(conf["accessKeyId"]);
  }
  if (conf.find("accessKeySecret") != conf.end()) {
    accessKeySecret = make_shared<string>(conf["accessKeySecret"]);
  }
  if (conf.find("securityToken") != conf.end()) {
    securityToken = make_shared<string>(conf["securityToken"]);
  }
  if (conf.find("bearerToken") != conf.end()) {
    bearerToken = make_shared<string>(conf["bearerToken"]);
  }
  if (conf.find("durationSeconds") != conf.end()) {
    durationSeconds = make_shared<int>(stoi(conf["durationSeconds"]));
  }
  if (conf.find("roleArn") != conf.end()) {
    roleArn = make_shared<string>(conf["roleArn"]);
  }
  if (conf.find("policy") != conf.end()) {
    policy = make_shared<string>(conf["policy"]);
  }
  if (conf.find("roleSessionExpiration") != conf.end()) {
    roleSessionExpiration = make_shared<int>(stoi(conf["roleSessionExpiration"]));
  }
  if (conf.find("roleSessionName") != conf.end()) {
    roleSessionName = make_shared<string>(conf["roleSessionName"]);
  }
  if (conf.find("publicKeyId") != conf.end()) {
    publicKeyId = make_shared<string>(conf["publicKeyId"]);
  }
  if (conf.find("privateKeyFile") != conf.end()) {
    privateKeyFile = make_shared<string>(conf["privateKeyFile"]);
    ifstream ifs(*privateKeyFile);
    string str((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    privateKeySecret = make_shared<string>(str);
  }
  if (conf.find("roleName") != conf.end()) {
    roleName = make_shared<string>(conf["roleName"]);
  }
  if (conf.find("type") != conf.end()) {
    type = make_shared<string>(conf["type"]);
  } else {
    // default
    type = make_shared<string>("access_key");
  }
}
Alibabacloud_Credential::Config::~Config() = default;;
Alibabacloud_Credential::Config::Config() = default;
