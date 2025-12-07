#ifndef ALIBABACLOUD_CREDENTIAL_EXPORT_HPP_
#define ALIBABACLOUD_CREDENTIAL_EXPORT_HPP_

// Export/import macro for Windows DLLs and default visibility on other platforms
#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef BUILDING_DLL
    #define ALIBABACLOUD_CREDENTIAL_EXPORT __declspec(dllexport)
  #else
    #define ALIBABACLOUD_CREDENTIAL_EXPORT __declspec(dllimport)
  #endif
#else
  #if __GNUC__ >= 4
    #define ALIBABACLOUD_CREDENTIAL_EXPORT __attribute__((visibility("default")))
  #else
    #define ALIBABACLOUD_CREDENTIAL_EXPORT
  #endif
#endif

#endif // ALIBABACLOUD_CREDENTIAL_EXPORT_HPP_
