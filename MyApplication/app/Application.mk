APP_ABI := arm64-v8a armeabi-v7a
APP_STL := gnustl_static
APP_PLATFORM := android-21
APP_CPPFLAGS := -std=c++11 -fpermissive -frtti -fexceptions
APP_OPTIM := release
APP_SHORT_COMMANDS := true

#NDK_TOOLCHAIN_VERSION defaults to GCC in NDK r12,  defaults to Clang since NDK r13b, and GCC is removed in NDK r18.
#set to GCC 4.9
NDK_TOOLCHAIN_VERSION := 4.9

APP_ALLOW_MISSING_DEPS := true