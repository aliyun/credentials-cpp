#include <alibabacloud/credential.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>

using namespace std;

Alibabacloud_Credential::Config::Config(map<string, string> *config) {
  typedef map<string, void *>::iterator MapIterator;
  map<string, string> conf =
      config == nullptr ? map<string, string>() : *config;
  if (conf.find("accessKeyId") != conf.end()) {
    accessKeyId = new string(conf["accessKeyId"]);
  }
  if (conf.find("accessKeySecret") != conf.end()) {
    accessKeySecret = new string(conf["accessKeySecret"]);
  }
  if (conf.find("securityToken") != conf.end()) {
    securityToken = new string(conf["securityToken"]);
  }
  if (conf.find("bearerToken") != conf.end()) {
    bearerToken = new string(conf["bearerToken"]);
  }
  if (conf.find("durationSeconds") != conf.end()) {
    durationSeconds = new int(stoi(conf["durationSeconds"]));
  }
  if (conf.find("roleArn") != conf.end()) {
    roleArn = new string(conf["roleArn"]);
  }
  if (conf.find("policy") != conf.end()) {
    policy = new string(conf["policy"]);
  }
  if (conf.find("roleSessionExpiration") != conf.end()) {
    roleSessionExpiration = new int(stoi(conf["roleSessionExpiration"]));
  }
  if (conf.find("roleSessionName") != conf.end()) {
    roleSessionName = new string(conf["roleSessionName"]);
  }
  if (conf.find("publicKeyId") != conf.end()) {
    publicKeyId = new string(conf["publicKeyId"]);
  }
  if (conf.find("privateKeyFile") != conf.end()) {
    privateKeyFile = new string(conf["privateKeyFile"]);
    ifstream ifs(*privateKeyFile);
    string str((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    privateKeySecret = new string(str);
  }
  if (conf.find("roleName") != conf.end()) {
    roleName = new string(conf["roleName"]);
  }
  if (conf.find("type") != conf.end()) {
    type = new string(conf["type"]);
  } else {
    // default
    type = new string("access_key");
  }
}
Alibabacloud_Credential::Config::~Config() {
  delete accessKeyId;
  delete accessKeySecret;
  delete securityToken;
  delete bearerToken;
  delete durationSeconds;
  delete roleArn;
  delete policy;
  delete roleSessionExpiration;
  delete roleSessionName;
  delete publicKeyId;
  delete privateKeyFile;
  delete privateKeySecret;
  delete roleName;
  delete type;
};
Alibabacloud_Credential::Config::Config() = default;
