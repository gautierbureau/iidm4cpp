// Loads a network with both backends of an installed iidm-bridge package: the GraalVM backend
// with the package's native image (found by its name on the library path), and the JNI backend
// in a JVM started here with the package's jar.

#include <iidm/iidm.h>
#include <jni.h>
#include <iostream>
#include <string>

namespace {

// IEEE14.xiidm gives _GEN____2_SM targetP="40.0"
bool check(const iidm::Network& network, const char* backend) {
    iidm::optional<iidm::Generator> generator = network.getGenerator("_GEN____2_SM");
    if (!generator) {
        std::cerr << backend << ": generator _GEN____2_SM not found\n";
        return false;
    }
    const double targetP = generator->getTargetP();
    std::cout << backend << ": targetP(_GEN____2_SM) = " << targetP << " MW\n";
    if (targetP != 40.0) {
        std::cerr << backend << ": expected 40 MW\n";
        return false;
    }
    return true;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: smoke_test <network.xiidm> <iidm-bridge fat jar>\n";
        return 2;
    }
    try {
        iidm::NetworkLoadOptions graalvm;
        graalvm.mode = iidm::BackendMode::GRAALVM;
        if (!check(iidm::NetworkFactory::load(argv[1], graalvm), "GraalVM")) return 1;

        std::string classPath = std::string("-Djava.class.path=") + argv[2];
        JavaVMOption option;
        option.optionString = &classPath[0];
        option.extraInfo = nullptr;
        JavaVMInitArgs args;
        args.version = JNI_VERSION_1_8;
        args.nOptions = 1;
        args.options = &option;
        args.ignoreUnrecognized = JNI_FALSE;
        JavaVM* jvm = nullptr;
        JNIEnv* env = nullptr;
        if (JNI_CreateJavaVM(&jvm, reinterpret_cast<void**>(&env), &args) != JNI_OK) {
            std::cerr << "Cannot start a JVM\n";
            return 1;
        }
        iidm::NetworkLoadOptions jni;
        jni.mode = iidm::BackendMode::JNI;
        if (!check(iidm::NetworkFactory::load(argv[1], jni), "JNI")) return 1;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
