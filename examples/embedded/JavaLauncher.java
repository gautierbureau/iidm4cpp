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
import com.powsybl.iidm.network.extensions.CoordinatedReactiveControl;
import com.powsybl.iidm.network.extensions.CoordinatedReactiveControlAdder;

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

        // Ensure _GEN____3_SM has CoordinatedReactiveControl so the JNI extension path is exercised
        Generator gen3 = network.getGenerator("_GEN____3_SM");
        if (gen3 != null && gen3.getExtension(CoordinatedReactiveControl.class) == null) {
            gen3.newExtension(CoordinatedReactiveControlAdder.class).withQPercent(40.0).add();
        }

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

        Generator g3 = network.getGenerator("_GEN____3_SM");
        if (g3 != null) {
            ActivePowerControl apc = g3.getExtension(ActivePowerControl.class);
            if (apc != null)
                System.out.println("[Java] _GEN____3_SM activePowerControl.droop=" + apc.getDroop());
            CoordinatedReactiveControl crc = g3.getExtension(CoordinatedReactiveControl.class);
            if (crc != null)
                System.out.println("[Java] _GEN____3_SM coordinatedReactiveControl.qPercent=" + crc.getQPercent());
        }
    }
}
