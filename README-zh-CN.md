[English](/README.md) | 简体中文

# Alibaba Cloud Credentials for C++

[![codecov](https://codecov.io/gh/aliyun/credentials-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/aliyun/credentials-cpp)
[![Travis Build Status](https://travis-ci.org/aliyun/credentials-cpp.svg?branch=master)](https://travis-ci.org/aliyun/credentials-cpp)

![](https://aliyunsdk-pages.alicdn.com/icons/AlibabaCloud.svg)

Alibaba Cloud Credentials for C++ 是帮助 C++ 开发者管理凭据的工具。

## 目录

- [环境要求](#环境要求)
- [安装](#安装)
- [快速使用](#快速使用)
- [凭证类型](#凭证类型)
  - [默认凭证提供者链](#默认凭证提供者链)
  - [AccessKey](#accesskey)
  - [STS](#sts)
  - [RamRoleArn](#ramrolearn)
  - [OIDCRoleArn](#oidcrolearn)
  - [EcsRamRole](#ecsramrole)
  - [URLCredential](#urlcredential)
  - [BearerToken](#bearertoken)
  - [RsaKeyPair](#rsakeypair)
- [配置](#配置)
- [问题](#问题)
- [发行说明](#发行说明)
- [相关](#相关)
- [许可证](#许可证)

## 环境要求

### 编译器要求

- **Windows**: Visual Studio 2015 或以上版本
- **Linux**: GCC 4.9 或以上版本
- **macOS**: Clang (Xcode Command Line Tools)

### 构建工具

- **CMake**: 3.10 或以上版本（Windows 推荐 3.15+）
- **C++ 标准**: C++11 或更高版本

### 系统要求

- **内存**: 建议 4GB 或以上
- **磁盘空间**: 建议至少 500MB 可用空间

### 依赖库

本库依赖 **darabonba-core**，需要安装以下系统库：

- **OpenSSL**: 用于加密和网络通信
  - Windows: 通过 vcpkg 安装 (`vcpkg install openssl`) 或 chocolatey 安装 (`choco install openssl`)
  - Linux: `sudo apt-get install libssl-dev` (Ubuntu/Debian) 或 `sudo yum install openssl-devel` (CentOS/RHEL)
  - macOS: `brew install openssl`

- **libcurl**: 用于 HTTP 请求
  - Windows: 通过 vcpkg 安装 (`vcpkg install curl`)
  - Linux: `sudo apt-get install libcurl4-openssl-dev` (Ubuntu/Debian) 或 `sudo yum install libcurl-devel` (CentOS/RHEL)
  - macOS: 系统预装或 `brew install curl`

- **zlib**: 用于压缩（curl 依赖，静态构建时必需）
  - Windows: 通过 vcpkg 安装 (`vcpkg install zlib`)
  - Linux: `sudo apt-get install zlib1g-dev` (Ubuntu/Debian) 或 `sudo yum install zlib-devel` (CentOS/RHEL)
  - macOS: 系统预装或 `brew install zlib`

- **uuid** (仅 Linux): 用于生成 UUID
  - Ubuntu/Debian: `sudo apt-get install uuid-dev`
  - CentOS/RHEL: `sudo yum install libuuid-devel`
  - macOS: 无需安装（使用 CoreFoundation 框架）
  - Windows: 无需安装（使用 Windows UUID API）

**快速安装所有依赖：**

```bash
# Ubuntu/Debian
sudo apt-get install -y libssl-dev libcurl4-openssl-dev zlib1g-dev uuid-dev

# CentOS/RHEL
sudo yum install -y openssl-devel libcurl-devel zlib-devel libuuid-devel

# macOS (Homebrew)
brew install openssl curl

# Windows (vcpkg)
vcpkg install openssl curl zlib
```

## 安装

### Linux/macOS

```bash
git clone https://github.com/aliyun/credentials-cpp.git
cd credentials-cpp
sh scripts/install.sh
```

### Windows

1. 使用 git-bash 执行以下命令，从 GitHub 克隆源码：

```bash
git clone https://github.com/aliyun/credentials-cpp.git
```

2. 使用 Visual Studio 进行编译：
   - 在根目录下创建 `cmake_build` 目录
   - 打开 CMake GUI，然后进行以下操作：
     - 选择 `Browse Source` 为源代码目录（`credentials-cpp`）
     - 选择 `Browse Build` 为构建目录（`cmake_build`）
     - 点击 `Configure`
     - 点击 `Generate`，生成 VS 解决方案

3. 编译安装 C++ SDK：
   - 进入 `cmake_build` 目录，使用 Visual Studio 打开 `alibabacloud_credentials.sln` 解决方案
   - 选择 `Release` 配置
   - 在"生成"->"配置管理器"中勾选 INSTALL
   - 生成 -> 生成解决方案

### 快速构建

```bash
# 克隆并构建
git clone https://github.com/aliyun/credentials-cpp.git
cd credentials-cpp
mkdir build && cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --install .
```

**📖 详细构建说明请参考 [BUILD.md](BUILD.md) | [BUILD-zh-CN.md](BUILD-zh-CN.md)**

## 快速使用

在您开始之前，您需要注册阿里云帐户并获取您的[凭证](https://usercenter.console.aliyun.com/#/manage/ak)。

**📚 更多示例请参考 [examples/README.md](examples/README.md) | [examples/README-zh-CN.md](examples/README-zh-CN.md)**

### 使用默认凭证提供者链（推荐）

```cpp
#include <alibabacloud/credentials/Client.hpp>
#include <iostream>

using namespace AlibabaCloud::Credentials;

int main() {
    // 使用默认凭证提供者链
    Client client;
    
    // 获取凭证
    auto credential = client.getCredential();
    std::cout << "AccessKeyId: " << credential.getAccessKeyId() << std::endl;
    std::cout << "AccessKeySecret: " << credential.getAccessKeySecret() << std::endl;
    std::cout << "Type: " << credential.getType() << std::endl;
    
    return 0;
}
```

### 使用指定凭证类型

```cpp
#include <alibabacloud/credentials/Client.hpp>
#include <iostream>

using namespace AlibabaCloud::Credentials;

int main() {
    // 使用 AccessKey 创建配置
    Models::Config config;
    config.setType("access_key")
          .setAccessKeyId("<your-access-key-id>")
          .setAccessKeySecret("<your-access-key-secret>");
    
    // 使用配置创建客户端
    Client client(config);
    
    // 获取凭证
    auto credential = client.getCredential();
    std::cout << "AccessKeyId: " << credential.getAccessKeyId() << std::endl;
    
    return 0;
}
```

## 凭证类型

### 默认凭证提供者链

如果您不指定凭证类型，客户端会按照如下顺序查找凭证：

1. **环境变量**：`ALIBABA_CLOUD_ACCESS_KEY_ID` 和 `ALIBABA_CLOUD_ACCESS_KEY_SECRET`
2. **OIDC RAM 角色**：通过环境变量 `ALIBABA_CLOUD_ROLE_ARN`、`ALIBABA_CLOUD_OIDC_PROVIDER_ARN` 和 `ALIBABA_CLOUD_OIDC_TOKEN_FILE` 配置
3. **配置文件**：`~/.alibabacloud/credentials.ini` 或 `~/.alibabacloud/credentials.json`
4. **ECS 实例 RAM 角色**：通过 ECS 实例元数据服务（IMDS）获取（如果设置了 `ALIBABA_CLOUD_ECS_METADATA`）
5. **Credentials URI**：通过环境变量 `ALIBABA_CLOUD_CREDENTIALS_URI` 指定的 URL 获取

**通过环境变量设置凭证：**

```bash
# Linux/macOS
export ALIBABA_CLOUD_ACCESS_KEY_ID="<your-access-key-id>"
export ALIBABA_CLOUD_ACCESS_KEY_SECRET="<your-access-key-secret>"

# Windows (PowerShell)
$env:ALIBABA_CLOUD_ACCESS_KEY_ID="<your-access-key-id>"
$env:ALIBABA_CLOUD_ACCESS_KEY_SECRET="<your-access-key-secret>"

# Windows (CMD)
set ALIBABA_CLOUD_ACCESS_KEY_ID=<your-access-key-id>
set ALIBABA_CLOUD_ACCESS_KEY_SECRET=<your-access-key-secret>
```

**通过配置文件设置凭证（`~/.alibabacloud/credentials.ini`）：**

```ini
[default]
type = access_key
access_key_id = <your-access-key-id>
access_key_secret = <your-access-key-secret>
```

### AccessKey

通过[用户信息管理][ak]设置 access_key，它们具有该账户完全的权限，请妥善保管。有时出于安全考虑，您不能把具有完全访问权限的主账户 AccessKey 交于一个项目的开发者使用，您可以[创建 RAM 子账户][ram]并为子账户[授权][permissions]，使用 RAM 子用户的 AccessKey 来进行 API 调用。

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("access_key")
          .setAccessKeyId("<your-access-key-id>")
          .setAccessKeySecret("<your-access-key-secret>");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    
    return 0;
}
```

### STS

通过安全令牌服务（Security Token Service，简称 STS），申请临时安全凭证（Temporary Security Credentials，简称 TSC），创建临时安全凭证。

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("sts")
          .setAccessKeyId("<your-access-key-id>")
          .setAccessKeySecret("<your-access-key-secret>")
          .setSecurityToken("<your-security-token>");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    printf("SecurityToken: %s\n", credential.getSecurityToken().c_str());
    
    return 0;
}
```

### RamRoleArn

通过指定 [RAM 角色][RAM Role]，让凭证自动申请维护 STS Token。你可以通过为 `Policy` 赋值来限制获取到的 STS Token 的权限（[如何创建策略][policy]）。

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("ram_role_arn")
          .setAccessKeyId("<your-access-key-id>")
          .setAccessKeySecret("<your-access-key-secret>")
          .setRoleArn("<your-role-arn>")
          .setRoleSessionName("<your-role-session-name>")
          // 可选：设置策略、有效期和地域
          .setPolicy("<your-policy>")
          .setDurationSeconds(3600)
          .setStsRegionId("cn-hangzhou");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    printf("SecurityToken: %s\n", credential.getSecurityToken().c_str());
    
    return 0;
}
```

### OIDCRoleArn

通过指定 OIDC 角色，凭证将使用 OIDC Token 自动申请维护 STS Token。

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("oidc_role_arn")
          .setRoleArn("<your-role-arn>")
          .setOidcProviderArn("<your-oidc-provider-arn>")
          .setOidcTokenFilePath("<path-to-oidc-token-file>")
          .setRoleSessionName("<your-role-session-name>")
          // 可选：设置有效期和地域
          .setDurationSeconds(3600)
          .setStsRegionId("cn-hangzhou");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    printf("SecurityToken: %s\n", credential.getSecurityToken().c_str());
    
    return 0;
}
```

**或通过环境变量配置：**

```bash
export ALIBABA_CLOUD_ROLE_ARN="<your-role-arn>"
export ALIBABA_CLOUD_OIDC_PROVIDER_ARN="<your-oidc-provider-arn>"
export ALIBABA_CLOUD_OIDC_TOKEN_FILE="<path-to-oidc-token-file>"
export ALIBABA_CLOUD_ROLE_SESSION_NAME="<your-role-session-name>"
```

### EcsRamRole

通过指定角色名称，让凭证自动申请维护 STS Token。

默认情况下，凭证工具会以安全强化模式（IMDSv2）访问 ECS 的元数据服务。如果抛出异常，凭证工具会切换到普通模式（IMDSv1）。您可以配置 `disableIMDSv1` 参数或 `ALIBABA_CLOUD_IMDSV1_DISABLE` 环境变量来指定异常处理逻辑：

- `false`（默认值）：凭证工具继续以普通模式（IMDSv1）获取访问凭证。
- `true`：抛出异常，凭证工具继续以安全强化模式获取访问凭证。

您可以指定 `ALIBABA_CLOUD_ECS_METADATA_DISABLED=true` 来禁止凭证工具访问 ECS 的元数据服务。

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("ecs_ram_role")
          .setRoleName("<your-ecs-role-name>")
          // 可选：禁用 IMDSv1 以增强安全性
          .setDisableIMDSv1(false);
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    printf("SecurityToken: %s\n", credential.getSecurityToken().c_str());
    
    return 0;
}
```

**或通过环境变量配置：**

```bash
export ALIBABA_CLOUD_ECS_METADATA="<your-ecs-role-name>"
```

### URLCredential

通过指定一个 URL，凭证工具将自动请求此 URL 来获取凭证。

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("credentials_uri")
          .setCredentialsUri("<your-credentials-url>");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    printf("SecurityToken: %s\n", credential.getSecurityToken().c_str());
    
    return 0;
}
```

**或通过环境变量配置：**

```bash
export ALIBABA_CLOUD_CREDENTIALS_URI="<your-credentials-url>"
```

URL 返回的响应必须是以下格式：

```json
{
  "Code": "Success",
  "AccessKeyId": "<your-access-key-id>",
  "AccessKeySecret": "<your-access-key-secret>",
  "SecurityToken": "<your-security-token>",
  "Expiration": "2023-12-31T12:00:00Z"
}
```

### BearerToken

如呼叫中心（CCC）需用此凭证，请自行申请维护 Bearer Token。

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("bearer")
          .setBearerToken("<your-bearer-token>");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("BearerToken: %s\n", credential.getBearerToken().c_str());
    
    return 0;
}
```

### RsaKeyPair

通过指定公钥 ID 和私钥文件，让凭证自动申请维护 AccessKey。仅支持日本站。

```cpp
#include <alibabacloud/credentials/Client.hpp>

using namespace AlibabaCloud::Credentials;

int main() {
    Models::Config config;
    config.setType("rsa_key_pair")
          .setPublicKeyId("<your-public-key-id>")
          .setPrivateKeyFile("<path-to-private-key-file>");
    
    Client client(config);
    
    auto credential = client.getCredential();
    printf("AccessKeyId: %s\n", credential.getAccessKeyId().c_str());
    printf("AccessKeySecret: %s\n", credential.getAccessKeySecret().c_str());
    
    return 0;
}
```

## 配置

### Config 参数

| 参数 | 类型 | 说明 |
|------|------|------|
| `type` | string | 凭证类型 |
| `accessKeyId` | string | Access Key ID |
| `accessKeySecret` | string | Access Key Secret |
| `securityToken` | string | Security Token |
| `bearerToken` | string | Bearer Token |
| `roleArn` | string | RAM 角色 ARN |
| `roleSessionName` | string | 角色会话名称 |
| `policy` | string | 权限策略（JSON 字符串） |
| `durationSeconds` | int64_t | 会话有效期（秒），默认值：3600 |
| `roleName` | string | ECS RAM 角色名称 |
| `oidcProviderArn` | string | OIDC 提供者 ARN |
| `oidcTokenFilePath` | string | OIDC Token 文件路径 |
| `publicKeyId` | string | RSA 公钥 ID |
| `privateKeyFile` | string | RSA 私钥文件路径 |
| `credentialsUri` | string | 获取凭证的 URL |
| `stsEndpoint` | string | STS 服务端点，默认值：sts.aliyuncs.com |
| `stsRegionId` | string | STS 地域 ID |
| `regionId` | string | 地域 ID，默认值：cn-hangzhou |
| `timeout` | int64_t | 读取超时时间（毫秒），默认值：5000 |
| `connectTimeout` | int64_t | 连接超时时间（毫秒），默认值：10000 |
| `disableIMDSv1` | bool | 禁用 ECS 元数据的 IMDSv1，默认值：false |
| `enableVpc` | bool | 启用 VPC 端点，默认值：false |

### 环境变量

| 变量 | 说明 |
|------|------|
| `ALIBABA_CLOUD_ACCESS_KEY_ID` | Access Key ID |
| `ALIBABA_CLOUD_ACCESS_KEY_SECRET` | Access Key Secret |
| `ALIBABA_CLOUD_SECURITY_TOKEN` | Security Token |
| `ALIBABA_CLOUD_ROLE_ARN` | RAM 角色 ARN |
| `ALIBABA_CLOUD_OIDC_PROVIDER_ARN` | OIDC 提供者 ARN |
| `ALIBABA_CLOUD_OIDC_TOKEN_FILE` | OIDC Token 文件路径 |
| `ALIBABA_CLOUD_ROLE_SESSION_NAME` | 角色会话名称 |
| `ALIBABA_CLOUD_ECS_METADATA` | ECS RAM 角色名称 |
| `ALIBABA_CLOUD_ECS_METADATA_DISABLED` | 禁用 ECS 元数据服务 |
| `ALIBABA_CLOUD_IMDSV1_DISABLE` | 禁用 IMDSv1 |
| `ALIBABA_CLOUD_CREDENTIALS_URI` | 获取凭证的 URL |
| `ALIBABA_CLOUD_STS_REGION` | STS 地域 |
| `ALIBABA_CLOUD_VPC_ENDPOINT_ENABLED` | 启用 VPC 端点 |

## 问题

[提交 Issue](https://github.com/aliyun/credentials-cpp/issues/new/choose)，不符合指南的问题可能会立即关闭。

## 发行说明

每个版本的详细更改记录在[发行说明](/CHANGELOG.md)中。

## 相关

- [OpenAPI 开发者门户][open-api]
- [Latest Release][latest-release]
- [阿里云控制台][console]
- [阿里云官网][aliyun]

## 许可证

[Apache-2.0](/LICENSE.md)

Copyright (c) 2009-present, Alibaba Cloud All rights reserved.

[open-api]: https://next.api.aliyun.com
[latest-release]: https://github.com/aliyun/credentials-cpp/releases
[console]: https://home.console.aliyun.com
[aliyun]: https://www.aliyun.com
[ak]: https://usercenter.console.aliyun.com/#/manage/ak
[ram]: https://ram.console.aliyun.com/users
[permissions]: https://ram.console.aliyun.com/permissions
[RAM Role]: https://ram.console.aliyun.com/#/role/list
[policy]: https://help.aliyun.com/document_detail/28664.html
