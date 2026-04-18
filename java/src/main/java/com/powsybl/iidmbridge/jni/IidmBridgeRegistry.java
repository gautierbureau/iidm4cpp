package com.powsybl.iidmbridge.jni;

import com.powsybl.iidm.network.Network;
import com.powsybl.iidm.serde.NetworkSerDe;

import java.nio.file.Path;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Used in embedded mode to make a network available to the C++ JNI backend.
 *
 * <p>The Java program must call {@link #register(String, Network)} (or
 * {@link #register(Network)} for the default ID) before Dynawo's C++ main
 * starts. The JNI backend then calls {@link #get(String)} through JNI to
 * retrieve the network object.</p>
 *
 * <p>Typical usage:</p>
 * <pre>{@code
 * Network network = NetworkSerDe.read(Paths.get("case.xiidm"));
 * IidmBridgeRegistry.register("case1", network);
 *
 * // Launch Dynawo via JNI or process API
 * DynawoRunner.run(simulationParameters);
 * }</pre>
 */
public final class IidmBridgeRegistry {

    private static final Map<String, Network> networks = new ConcurrentHashMap<>();

    private IidmBridgeRegistry() {}

    /**
     * Register a network under a given ID.
     *
     * @param id      identifier used by the C++ side to retrieve the network
     * @param network the PowSyBl network object
     */
    public static void register(String id, Network network) {
        networks.put(id, network);
    }

    /**
     * Register under the default ID {@code "default"}.
     *
     * @param network the PowSyBl network object
     */
    public static void register(Network network) {
        register("default", network);
    }

    /**
     * Called from JNI to retrieve a previously registered network.
     *
     * @param id network identifier
     * @return the registered Network
     * @throws IllegalStateException if no network is registered with the given id
     */
    public static Network get(String id) {
        Network n = networks.get(id);
        if (n == null) {
            throw new IllegalStateException("No network registered with id: " + id);
        }
        return n;
    }

    /**
     * Called from JNI to list all registered network IDs.
     *
     * @return array of registered IDs
     */
    public static String[] listIds() {
        return networks.keySet().toArray(new String[0]);
    }

    /**
     * Unregister a network by ID.
     *
     * @param id network identifier to remove
     */
    public static void unregister(String id) {
        networks.remove(id);
    }

    /**
     * Clear all registered networks.
     */
    public static void clear() {
        networks.clear();
    }

    /**
     * Called from JNI to save a registered network to an XIIDM file.
     *
     * @param id       network identifier
     * @param filePath destination file path
     */
    public static void save(String id, String filePath) {
        Network network = get(id);
        NetworkSerDe.write(network, Path.of(filePath));
    }
}
