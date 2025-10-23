#include <shimejifinder/utf8_convert.hpp>
#include <gtest/gtest.h>

int main(int argc, char **argv) {
    JavaVM *vm;
    JNIEnv *env;
    JavaVMInitArgs vm_args;
    jint res;

    // create JVM
    vm_args.version  = JNI_VERSION_9;
    vm_args.nOptions = 0;
    res = JNI_CreateJavaVM(&vm, (void **)&env, &vm_args);
    if (res != JNI_OK) {
        std::cerr << "Failed to create JVM" << std::endl;
        return EXIT_FAILURE;
    }
    shimejifinder::jni_env = env;

    // run tests
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
