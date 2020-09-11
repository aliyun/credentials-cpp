#include <alibabacloud/credential.hpp>
#include <iostream>

using namespace std;

string *Alibabacloud_Credential::Client::getAccessKeyId() {
  return _credential->getAccessKeyId();
}

string *Alibabacloud_Credential::Client::getAccessKeySecret() {
  return _credential->getAccessKeySecret();
}

string *Alibabacloud_Credential::Client::getSecurityToken() {
  return _credential->getSecurityToken();
}

string *Alibabacloud_Credential::Client::getBearerToken() {
  return _credential->getBearerToken();
}

string *Alibabacloud_Credential::Client::getType() {
  return _credential->getType();
}

string *Alibabacloud_Credential::Client::getRoleArn() {
  return _credential->getConfig()->roleArn;
}

string *Alibabacloud_Credential::Client::getRoleSessionName() {
  return _credential->getConfig()->roleSessionName;
}

string *Alibabacloud_Credential::Client::getPolicy() {
  return _credential->getConfig()->policy;
}

string *Alibabacloud_Credential::Client::getRoleName() {
  return _credential->getConfig()->roleName;
}

string *Alibabacloud_Credential::Client::getPublicKeyId() {
  return _credential->getConfig()->publicKeyId;
}

string *Alibabacloud_Credential::Client::getPrivateKey() {
  return _credential->getConfig()->privateKeySecret;
}

Alibabacloud_Credential::Credential *
Alibabacloud_Credential::Client::getCredential() {
  return _credential;
}

Alibabacloud_Credential::Client::Client(
    Alibabacloud_Credential::Config *config) {
  if (*config->type == string("bearer_token")) {
    auto *btc = new BearerTokenCredential(config);
    _credential = btc;
  } else if (*config->type == "sts") {
    auto *sts = new StsCredential(config);
    _credential = sts;
  } else if (*config->type == "ecs_ram_role") {
    auto *erc = new EcsRamRoleCredential(config);
    _credential = erc;
  } else if (*config->type == "ram_role_arn") {
    auto *rac = new RamRoleArnCredential(config);
    _credential = rac;
  } else if (*config->type == "rsa_key_pair") {
    auto *rkpc = new RsaKeyPairCredential(config);
    _credential = rkpc;
  } else {
    auto *akc = new AccessKeyCredential(config);
    _credential = akc;
  }
}

Alibabacloud_Credential::Client::~Client() { delete _credential; }
Alibabacloud_Credential::Client::Client() {
  map<string, string> m;
  m.insert(pair<string, string>("type", "access_key"));
  auto *config = new Config(m);
  auto *akc = new AccessKeyCredential(config);
  _credential = akc;
}
