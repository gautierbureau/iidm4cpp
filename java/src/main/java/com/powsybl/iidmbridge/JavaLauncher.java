package com.powsybl.iidmbridge;

import com.powsybl.iidm.network.Network;
import com.powsybl.iidm.serde.NetworkSerDe;
import com.powsybl.iidmbridge.jni.IidmBridgeRegistry;

import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * Entry point for running the IIDM bridge in JNI embedded mode from an IDE or CLI.
 *
 * <p>Usage: JavaLauncher &lt;network-file&gt; [network-id]
 *
 * <p>The network file is loaded via PowSyBl IIDM SerDe and registered in
 * {@link IidmBridgeRegistry} so that the C++ JNI backend can retrieve it.
 * The default network ID is {@code "default"} when none is provided.
 */
public final class JavaLauncher {

    // Loads libexample_embedded.so, which links iidmbridge and exports the JNI entry point.
    static {
        System.loadLibrary("example_embedded");
    }

    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("Usage: JavaLauncher <network-file> [network-id]");
            System.exit(1);
        }

        Path networkFile = Paths.get(args[0]);
        String networkId = args.length >= 2 ? args[1] : "default";

        System.out.println("Loading network from: " + networkFile.toAbsolutePath());
        Network network = Network.read(networkFile);

        System.out.println("Network loaded: " + network.getNameOrId());
        System.out.printf("  Substations : %d%n", network.getSubstationCount());
        System.out.printf("  Voltage levels: %d%n", network.getVoltageLevelCount());
        System.out.printf("  Lines       : %d%n", network.getLineCount());

        IidmBridgeRegistry.register(networkId, network);
        System.out.println("Network registered with id: " + networkId);
        System.out.println("Ready for JNI backend calls.");

        // Call the C++ simulation entry point
        System.out.println("[Java] Handing off to C++ simulation...");
        runEmbedded(networkId);
        System.out.println("[Java] C++ simulation returned.");

        // The network object in this JVM now reflects any changes made by C++
        network.getGenerators().forEach(g ->
                System.out.printf("[Java] Generator %s targetP=%.1f MW%n",
                        g.getId(), g.getTargetP()));
    }
}
