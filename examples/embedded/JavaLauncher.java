/**
 * Embedded mode example — Java orchestrator side.
 *
 * This class loads a PowSyBl network, registers it with IidmBridgeRegistry,
 * and then calls the C++ simulation code via JNI.
 *
 * Compile and run:
 *   javac -cp powsybl-core.jar:iidm-bridge-java.jar JavaLauncher.java
 *   java  -cp powsybl-core.jar:iidm-bridge-java.jar \
 *         -Djava.library.path=/path/to/libiidmbridge.so \
 *         JavaLauncher case.xiidm
 */

import com.powsybl.iidm.network.*;
import com.powsybl.iidmbridge.jni.IidmBridgeRegistry;
import com.powsybl.iidm.network.extensions.ActivePowerControl;

import java.nio.file.Path;

public class JavaLauncher {

    // Loads libexample_embedded.so, which links iidmbridge and exports the JNI entry point.
    static {
        System.loadLibrary("example_embedded");
    }

    // JNI native method declaration — implemented in DynawoEmbedded.cpp
    private static native void runEmbedded(String networkId);

    public static void main(String[] args) throws Exception {
        if (args.length < 1) {
            System.err.println("Usage: JavaLauncher <path-to-network.xiidm>");
            System.exit(1);
        }

        // Load the network on the Java side
        Network network = Network.read(Path.of(args[0]));
        System.out.println("[Java] Loaded network: " + network.getId());

        // Register it so the C++ JNI backend can find it
        String networkId = "case1";
        IidmBridgeRegistry.register(networkId, network);

        // Call the C++ simulation entry point
        System.out.println("[Java] Handing off to C++ simulation...");
        runEmbedded(networkId);
        System.out.println("[Java] C++ simulation returned.");

        // The network object in this JVM now reflects any changes made by C++
        network.getGenerators().forEach(g ->
            System.out.printf("[Java] Generator %s targetP=%.1f MW%n",
                              g.getId(), g.getTargetP()));

        System.out.println("[Java] Generator droop");
        double droop = network.getGenerator("_GEN____3_SM").getExtension(ActivePowerControl.class).getDroop();
        System.out.println("[Java] droop=" + droop);
    }
}
