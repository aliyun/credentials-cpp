/**
 * Alibaba Cloud Credentials C++ SDK - All Credential Types Examples
 * 
 * This example demonstrates all credential types:
 * 1. Default Credentials Provider Chain
 * 2. AccessKey
 * 3. STS
 * 4. RamRoleArn
 * 5. OIDCRoleArn
 * 6. EcsRamRole
 * 7. URLCredential (credentials_uri)
 * 8. BearerToken
 * 9. RsaKeyPair
 * 10. CLI Profile
 * 11. Profile (credentials file)
 */

#include <alibabacloud/credentials/Client.hpp>
#include <iostream>

using namespace AlibabaCloud::Credentials;

// ============================================================================
// 1. Default Credentials Provider Chain (Recommended)
// ============================================================================
void defaultCredentialsProviderChainExample() {
    std::cout << "=== 1. Default Credentials Provider Chain ===" << std::endl;
    std::cout << "The default chain tries providers in order:" << std::endl;
    std::cout << "  1. Environment Variables" << std::endl;
    std::cout << "  2. OIDC RAM Role" << std::endl;
    std::cout << "  3. CLI Profile" << std::endl;
    std::cout << "  4. Profile (credentials file)" << std::endl;
    std::cout << "  5. ECS RAM Role" << std::endl;
    std::cout << "  6. Credentials URI" << std::endl;
    std::cout << std::endl;
    
    try {
        // Use default constructor - automatically uses the credential chain
        Client client;
        
        if (!client.empty()) {
            auto credential = client.getCredential();
            std::cout << "Successfully obtained credentials!" << std::endl;
            std::cout << "  Type: " << credential.getType() << std::endl;
            std::cout << "  Provider: " << client.getProviderName() << std::endl;
        } else {
            std::cout << "No credentials found in the default chain." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Note: " << e.what() << std::endl;
    }
    std::cout << std::endl;
}

// ============================================================================
// 2. AccessKey Credentials
// ============================================================================
void accessKeyExample() {
    std::cout << "=== 2. AccessKey Credentials ===" << std::endl;
    std::cout << "Note: AccessKey should NEVER be hardcoded in your code!" << std::endl;
    std::cout << "Please set environment variables:" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_ACCESS_KEY_ID" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_ACCESS_KEY_SECRET" << std::endl;
    std::cout << std::endl;
    
    // Get credentials from environment variables
    const char* accessKeyId = std::getenv("ALIBABA_CLOUD_ACCESS_KEY_ID");
    const char* accessKeySecret = std::getenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
    
    if (!accessKeyId || !accessKeySecret) {
        std::cout << "  Skipped: Environment variables not set." << std::endl;
        std::cout << std::endl;
        return;
    }
    
    Models::Config config;
    config.setType("access_key")
          .setAccessKeyId(accessKeyId)
          .setAccessKeySecret(accessKeySecret);
    
    Client client(config);
    
    auto credential = client.getCredential();
    std::cout << "  Type: " << credential.getType() << std::endl;
    std::cout << "  AccessKeyId: " << credential.getAccessKeyId() << std::endl;
    std::cout << "  Provider: " << client.getProviderName() << std::endl;
    std::cout << std::endl;
}

// ============================================================================
// 3. STS Credentials
// ============================================================================
void stsExample() {
    std::cout << "=== 3. STS Credentials ===" << std::endl;
    std::cout << "Note: Get STS credentials from environment variables." << std::endl;
    std::cout << "Please set:" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_ACCESS_KEY_ID" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_ACCESS_KEY_SECRET" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_SECURITY_TOKEN" << std::endl;
    std::cout << std::endl;
    
    const char* accessKeyId = std::getenv("ALIBABA_CLOUD_ACCESS_KEY_ID");
    const char* accessKeySecret = std::getenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
    const char* securityToken = std::getenv("ALIBABA_CLOUD_SECURITY_TOKEN");
    
    if (!accessKeyId || !accessKeySecret || !securityToken) {
        std::cout << "  Skipped: Environment variables not set." << std::endl;
        std::cout << std::endl;
        return;
    }
    
    Models::Config config;
    config.setType("sts")
          .setAccessKeyId(accessKeyId)
          .setAccessKeySecret(accessKeySecret)
          .setSecurityToken(securityToken);
    
    Client client(config);
    
    auto credential = client.getCredential();
    std::cout << "  Type: " << credential.getType() << std::endl;
    std::cout << "  AccessKeyId: " << credential.getAccessKeyId() << std::endl;
    std::cout << "  Has SecurityToken: " << (!credential.getSecurityToken().empty() ? "Yes" : "No") << std::endl;
    std::cout << "  Provider: " << client.getProviderName() << std::endl;
    std::cout << std::endl;
}

// ============================================================================
// 4. RamRoleArn Credentials
// ============================================================================
void ramRoleArnExample() {
    std::cout << "=== 4. RamRoleArn Credentials ===" << std::endl;
    std::cout << "Note: This requires valid credentials from environment variables." << std::endl;
    std::cout << "Please set:" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_ACCESS_KEY_ID" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_ACCESS_KEY_SECRET" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_ROLE_ARN (optional, can set in config)" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_ROLE_SESSION_NAME (optional)" << std::endl;
    std::cout << std::endl;
    
    const char* accessKeyId = std::getenv("ALIBABA_CLOUD_ACCESS_KEY_ID");
    const char* accessKeySecret = std::getenv("ALIBABA_CLOUD_ACCESS_KEY_SECRET");
    
    if (!accessKeyId || !accessKeySecret) {
        std::cout << "  Skipped: Environment variables not set." << std::endl;
        std::cout << std::endl;
        return;
    }
    
    Models::Config config;
    config.setType("ram_role_arn")
          .setAccessKeyId(accessKeyId)
          .setAccessKeySecret(accessKeySecret)
          .setRoleArn("<your-role-arn>")  // Or from env: ALIBABA_CLOUD_ROLE_ARN
          .setRoleSessionName("<your-role-session-name>")
          // Optional parameters
          .setPolicy("<your-policy>")
          .setDurationSeconds(3600)
          .setStsRegionId("cn-hangzhou");
    
    std::cout << "  Config Type: " << config.getType() << std::endl;
    std::cout << "  RoleArn: " << config.getRoleArn() << std::endl;
    std::cout << "  RoleSessionName: " << config.getRoleSessionName() << std::endl;
    std::cout << "  DurationSeconds: " << config.getDurationSeconds() << std::endl;
    std::cout << "  Note: Actual client creation skipped (requires valid Role ARN)" << std::endl;
    std::cout << std::endl;
    
    // Uncomment to actually create client (requires valid credentials and Role ARN)
    // Client client(config);
    // auto credential = client.getCredential();
}

// ============================================================================
// 5. OIDCRoleArn Credentials
// ============================================================================
void oidcRoleArnExample() {
    std::cout << "=== 5. OIDCRoleArn Credentials ===" << std::endl;
    std::cout << "Note: This requires OIDC configuration to work." << std::endl;
    std::cout << "Environment variables:" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_ROLE_ARN" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_OIDC_PROVIDER_ARN" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_OIDC_TOKEN_FILE" << std::endl;
    std::cout << "  - ALIBABA_CLOUD_ROLE_SESSION_NAME (optional)" << std::endl;
    std::cout << std::endl;
    
    Models::Config config;
    config.setType("oidc_role_arn")
          .setRoleArn("<your-role-arn>")
          .setOidcProviderArn("<your-oidc-provider-arn>")
          .setOidcTokenFilePath("<path-to-oidc-token-file>")
          .setRoleSessionName("<your-role-session-name>")
          // Optional parameters
          .setDurationSeconds(3600)
          .setStsRegionId("cn-hangzhou");
    
    std::cout << "  Config Type: " << config.getType() << std::endl;
    std::cout << "  RoleArn: " << config.getRoleArn() << std::endl;
    std::cout << "  OidcProviderArn: " << config.getOidcProviderArn() << std::endl;
    std::cout << std::endl;
    
    // Uncomment to actually create client (requires valid OIDC setup)
    // Client client(config);
    // auto credential = client.getCredential();
}

// ============================================================================
// 6. EcsRamRole Credentials
// ============================================================================
void ecsRamRoleExample() {
    std::cout << "=== 6. EcsRamRole Credentials ===" << std::endl;
    std::cout << "Note: This only works on Alibaba Cloud ECS instances." << std::endl;
    std::cout << "Environment variable: ALIBABA_CLOUD_ECS_METADATA=<role-name>" << std::endl;
    std::cout << std::endl;
    
    Models::Config config;
    config.setType("ecs_ram_role")
          .setRoleName("<your-ecs-role-name>")
          // Optional: disable IMDSv1 for enhanced security
          .setDisableIMDSv1(false);
    
    std::cout << "  Config Type: " << config.getType() << std::endl;
    std::cout << "  RoleName: " << config.getRoleName() << std::endl;
    std::cout << "  DisableIMDSv1: " << (config.getDisableIMDSv1() ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Uncomment to actually create client (only works on ECS)
    // Client client(config);
    // auto credential = client.getCredential();
}

// ============================================================================
// 7. URLCredential (credentials_uri)
// ============================================================================
void urlCredentialExample() {
    std::cout << "=== 7. URLCredential (credentials_uri) ===" << std::endl;
    std::cout << "Note: The URL must return JSON in the following format:" << std::endl;
    std::cout << R"(  {
    "Code": "Success",
    "AccessKeyId": "<access-key-id>",
    "AccessKeySecret": "<access-key-secret>",
    "SecurityToken": "<security-token>",
    "Expiration": "2023-12-31T12:00:00Z"
  })" << std::endl;
    std::cout << "Environment variable: ALIBABA_CLOUD_CREDENTIALS_URI=<url>" << std::endl;
    std::cout << std::endl;
    
    Models::Config config;
    config.setType("credentials_uri")
          .setCredentialsUri("<your-credentials-url>");

    std::cout << "  Config Type: " << config.getType() << std::endl;
    std::cout << "  CredentialsUri: " << config.getCredentialsUri() << std::endl;
    std::cout << std::endl;
    
    // Uncomment to actually create client (requires valid URL)
    // Client client(config);
    // auto credential = client.getCredential();
}

// ============================================================================
// 8. BearerToken Credentials
// ============================================================================
void bearerTokenExample() {
    std::cout << "=== 8. BearerToken Credentials ===" << std::endl;
    std::cout << "Note: Used for Cloud Call Centre (CCC) and similar services." << std::endl;
    std::cout << "Get Bearer Token from your service provider." << std::endl;
    std::cout << "Please set: ALIBABA_CLOUD_BEARER_TOKEN" << std::endl;
    std::cout << std::endl;
    
    const char* bearerToken = std::getenv("ALIBABA_CLOUD_BEARER_TOKEN");
    
    if (!bearerToken) {
        std::cout << "  Skipped: Environment variable ALIBABA_CLOUD_BEARER_TOKEN not set." << std::endl;
        std::cout << std::endl;
        return;
    }
    
    Models::Config config;
    config.setType("bearer")
          .setBearerToken(bearerToken);
    
    Client client(config);
    
    auto credential = client.getCredential();
    std::cout << "  Type: " << credential.getType() << std::endl;
    std::cout << "  Has BearerToken: " << (!credential.getBearerToken().empty() ? "Yes" : "No") << std::endl;
    std::cout << "  Provider: " << client.getProviderName() << std::endl;
    std::cout << std::endl;
}

// ============================================================================
// 9. RsaKeyPair Credentials
// ============================================================================
void rsaKeyPairExample() {
    std::cout << "=== 9. RsaKeyPair Credentials ===" << std::endl;
    std::cout << "Note: This requires RSA key pair configuration." << std::endl;
    std::cout << std::endl;
    
    Models::Config config;
    config.setType("rsa_key_pair")
          .setPublicKeyId("<your-public-key-id>")
          .setPrivateKeyFile("<path-to-private-key-file>");
    
    std::cout << "  Config Type: " << config.getType() << std::endl;
    std::cout << "  PublicKeyId: " << config.getPublicKeyId() << std::endl;
    std::cout << "  PrivateKeyFile: " << config.getPrivateKeyFile() << std::endl;
    std::cout << std::endl;
    
    // Uncomment to actually create client (requires valid RSA key pair)
    // Client client(config);
    // auto credential = client.getCredential();
}

// ============================================================================
// 10. CLI Profile Credentials
// ============================================================================
void cliProfileExample() {
    std::cout << "=== 10. CLI Profile Credentials ===" << std::endl;
    std::cout << "Note: Uses Alibaba Cloud CLI configuration." << std::endl;
    std::cout << "Config file: ~/.aliyun/config.json" << std::endl;
    std::cout << "Disable via: ALIBABA_CLOUD_CLI_PROFILE_DISABLED=true" << std::endl;
    std::cout << std::endl;
    std::cout << "CLI Profile is automatically used in the default credential chain." << std::endl;
    std::cout << "It reads from ~/.aliyun/config.json created by 'aliyun configure'." << std::endl;
    std::cout << std::endl;
}

// ============================================================================
// 11. Profile Credentials (credentials file)
// ============================================================================
void profileExample() {
    std::cout << "=== 11. Profile Credentials (credentials file) ===" << std::endl;
    std::cout << "Note: Uses credentials file configuration." << std::endl;
    std::cout << "Config file: ~/.alibabacloud/credentials.ini" << std::endl;
    std::cout << std::endl;
    std::cout << "Example credentials.ini:" << std::endl;
    std::cout << R"(  [default]
  type = access_key
  access_key_id = <your-access-key-id>
  access_key_secret = <your-access-key-secret>

  [client1]
  type = ram_role_arn
  access_key_id = <your-access-key-id>
  access_key_secret = <your-access-key-secret>
  role_arn = <your-role-arn>
  role_session_name = <session-name>

  [client2]
  type = ecs_ram_role
  role_name = <your-ecs-role-name>
)" << std::endl;
    std::cout << std::endl;
    std::cout << "Profile is automatically used in the default credential chain." << std::endl;
    std::cout << std::endl;
}

// ============================================================================
// Main
// ============================================================================
int main() {
    std::cout << "========================================================" << std::endl;
    std::cout << "Alibaba Cloud Credentials C++ SDK - All Credential Types" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;
    
    try {
        defaultCredentialsProviderChainExample();
        accessKeyExample();
        stsExample();
        ramRoleArnExample();
        oidcRoleArnExample();
        ecsRamRoleExample();
        urlCredentialExample();
        bearerTokenExample();
        rsaKeyPairExample();
        cliProfileExample();
        profileExample();
        
        std::cout << "========================================================" << std::endl;
        std::cout << "All examples completed!" << std::endl;
        std::cout << "========================================================" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
