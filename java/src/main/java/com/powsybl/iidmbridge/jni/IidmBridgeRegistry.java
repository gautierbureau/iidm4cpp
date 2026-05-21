package com.powsybl.iidmbridge.jni;

import com.powsybl.iidm.network.Network;
import com.powsybl.iidm.serde.NetworkSerDe;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
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

    /**
     * Called from JNI to load a network from an in-memory byte buffer and
     * register it under the given id.
     *
     * @param id            network identifier to register under
     * @param data          serialized network bytes
     * @param filenameHint  filename hint (e.g. "network.xiidm") used by
     *                      PowSyBl to select the importer
     */
    public static void loadFromBytes(String id, byte[] data, String filenameHint) {
        try (ByteArrayInputStream in = new ByteArrayInputStream(data)) {
            // NetworkSerDe.read(InputStream) handles the XIIDM format; the
            // filenameHint is reserved for future multi-format support.
            Network network = NetworkSerDe.read(in);
            register(id, network);
        } catch (java.io.IOException e) {
            throw new IllegalStateException("Failed to read network bytes", e);
        }
    }

    /**
     * Called from JNI to serialize a registered network to an in-memory byte
     * buffer.
     *
     * @param id           network identifier
     * @param filenameHint format hint (reserved for future multi-format
     *                     support; currently always XIIDM)
     * @return serialized network bytes
     */
    public static byte[] saveToBytes(String id, String filenameHint) {
        Network network = get(id);
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        NetworkSerDe.write(network, out);
        return out.toByteArray();
    }
}
