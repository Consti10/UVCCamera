

# This is just for mips, if you really needs MSA, un-comment and build with GCC.
# Note: Supporting GCC on NDK is already deprecated and GCC will be removed from NDK soon.
#NDK_TOOLCHAIN_VERSION := 4.9

APP_PLATFORM := android-9
APP_ABI := armeabi-v7a arm64-v8a
#APP_OPTIM := debug
APP_OPTIM := release
