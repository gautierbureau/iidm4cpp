package com.powsybl.iidmbridge.graalvm;

import com.powsybl.iidm.network.Network;
import com.powsybl.iidm.serde.NetworkSerDe;
import com.powsybl.iidmbridge.PropertyDispatcher;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.UnmanagedMemory;
import org.graalvm.nativeimage.c.CContext;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CCharPointerPointer;
import org.graalvm.nativeimage.c.type.CLongPointer;
import org.graalvm.nativeimage.c.type.CIntPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;
import org.graalvm.word.WordFactory;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;

/**
 * GraalVM Native Image entry points.
 * Each method annotated with {@code @CEntryPoint} is exported as a C function
 * callable from the C++ GraalVMBackend.
 */
public final class IidmEntryPoints {

    private IidmEntryPoints() {}

    @CEntryPoint(name = "iidm_load_network")
    public static long loadNetwork(IsolateThread thread, CCharPointer filePath) {
        try {
            String path = CTypeConversion.toJavaString(filePath);
            Network network = Network.read(Path.of(path));
            return NetworkRegistry.register(network);
        } catch (Exception e) {
            return 0L; // INVALID_HANDLE
        }
    }

    @CEntryPoint(name = "iidm_save_network")
    public static int saveNetwork(IsolateThread thread, long handle, CCharPointer filePath) {
        try {
            String path = CTypeConversion.toJavaString(filePath);
            Network network = (Network) NetworkRegistry.lookup(handle);
            NetworkSerDe.write(network, Path.of(path));
            return 0;
        } catch (Exception e) {
            return 1;
        }
    }

    /**
     * Load a network from an in-memory byte buffer.
     *
     * @return a positive handle on success, 0 on failure.
     */
    @CEntryPoint(name = "iidm_load_network_bytes")
    public static long loadNetworkBytes(IsolateThread thread,
                                        CCharPointer data, int length,
                                        CCharPointer filenameHint) {
        try {
            byte[] bytes = new byte[length];
            for (int i = 0; i < length; i++) {
                bytes[i] = data.read(i);
            }
            // filenameHint is reserved for future multi-format support;
            // NetworkSerDe currently only handles XIIDM via InputStream.
            try (ByteArrayInputStream in = new ByteArrayInputStream(bytes)) {
                Network network = NetworkSerDe.read(in);
                return NetworkRegistry.register(network);
            }
        } catch (Exception e) {
            return 0L;
        }
    }

    /**
     * Serialize a network into a freshly allocated unmanaged byte buffer.
     * The caller is responsible for releasing it via {@code iidm_free_bytes}.
     *
     * <p>On success returns 0 and writes the buffer pointer and length into
     * {@code outData}/{@code outLen}. On failure returns a non-zero code and
     * leaves {@code outData}/{@code outLen} undefined.</p>
     */
    @CEntryPoint(name = "iidm_save_network_bytes")
    public static int saveNetworkBytes(IsolateThread thread, long handle,
                                       CCharPointer filenameHint,
                                       CCharPointerPointer outData,
                                       CIntPointer outLen) {
        try {
            Network network = (Network) NetworkRegistry.lookup(handle);
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            NetworkSerDe.write(network, baos);
            byte[] bytes = baos.toByteArray();
            CCharPointer ptr = UnmanagedMemory.malloc(bytes.length);
            for (int i = 0; i < bytes.length; i++) {
                ptr.write(i, bytes[i]);
            }
            outData.write(ptr);
            outLen.write(bytes.length);
            return 0;
        } catch (Exception e) {
            outData.write(WordFactory.nullPointer());
            outLen.write(0);
            return 1;
        }
    }

    @CEntryPoint(name = "iidm_free_bytes")
    public static void freeBytes(IsolateThread thread, CCharPointer ptr) {
        if (ptr.isNonNull()) {
            UnmanagedMemory.free(ptr);
        }
    }

    @CEntryPoint(name = "iidm_get_double")
    public static double getDouble(IsolateThread thread, long handle, int property) {
        return PropertyDispatcher.getDouble(handle, property);
    }

    @CEntryPoint(name = "iidm_set_double")
    public static void setDouble(IsolateThread thread, long handle, int property, double value) {
        PropertyDispatcher.setDouble(handle, property, value);
    }

    @CEntryPoint(name = "iidm_get_int")
    public static int getInt(IsolateThread thread, long handle, int property) {
        return PropertyDispatcher.getInt(handle, property);
    }

    @CEntryPoint(name = "iidm_set_int")
    public static void setInt(IsolateThread thread, long handle, int property, int value) {
        PropertyDispatcher.setInt(handle, property, value);
    }

    @CEntryPoint(name = "iidm_get_bool")
    public static int getBool(IsolateThread thread, long handle, int property) {
        return PropertyDispatcher.getBool(handle, property);
    }

    @CEntryPoint(name = "iidm_set_bool")
    public static void setBool(IsolateThread thread, long handle, int property, int value) {
        PropertyDispatcher.setBool(handle, property, value);
    }

    @CEntryPoint(name = "iidm_get_string")
    public static CCharPointer getString(IsolateThread thread, long handle, int property) {
        String value = PropertyDispatcher.getString(handle, property);
        byte[] bytes = value.getBytes(StandardCharsets.UTF_8);
        CCharPointer ptr = UnmanagedMemory.malloc(bytes.length + 1);
        for (int i = 0; i < bytes.length; i++) {
            ptr.write(i, bytes[i]);
        }
        ptr.write(bytes.length, (byte) 0);
        return ptr;
    }

    @CEntryPoint(name = "iidm_set_string")
    public static void setString(IsolateThread thread, long handle, int property, CCharPointer value) {
        PropertyDispatcher.setString(handle, property, CTypeConversion.toJavaString(value));
    }

    @CEntryPoint(name = "iidm_free_string")
    public static void freeString(IsolateThread thread, CCharPointer ptr) {
        org.graalvm.nativeimage.UnmanagedMemory.free(ptr);
    }

    @CEntryPoint(name = "iidm_get_children")
    public static void getChildren(IsolateThread thread,
                                   long parentHandle,
                                   int childType,
                                   CLongPointer outHandles,
                                   CIntPointer outCount) {
        long[] handles = PropertyDispatcher.getChildren(parentHandle, childType);
        outCount.write(handles.length);
        if (outHandles.isNonNull()) {
            for (int i = 0; i < handles.length; i++) {
                outHandles.addressOf(i).write(handles[i]);
            }
        }
    }

    @CEntryPoint(name = "iidm_get_related")
    public static long getRelated(IsolateThread thread, long handle, int relation) {
        return PropertyDispatcher.getRelated(handle, relation);
    }

    @CEntryPoint(name = "iidm_get_int_list")
    public static void getIntList(IsolateThread thread,
                                  long handle,
                                  int listCode,
                                  CIntPointer outValues,
                                  CIntPointer outCount) {
        int[] values = PropertyDispatcher.getIntList(handle, listCode);
        outCount.write(values.length);
        if (outValues.isNonNull()) {
            for (int i = 0; i < values.length; i++) {
                outValues.addressOf(i).write(values[i]);
            }
        }
    }

    @CEntryPoint(name = "iidm_get_related_by_index")
    public static long getRelatedByIndex(IsolateThread thread, long handle, int relation, int index) {
        return PropertyDispatcher.getRelatedByIndex(handle, relation, index);
    }

    @CEntryPoint(name = "iidm_get_related_by_string")
    public static long getRelatedByString(IsolateThread thread, long handle, int relation, CCharPointer key) {
        return PropertyDispatcher.getRelatedByString(handle, relation, CTypeConversion.toJavaString(key));
    }

    @CEntryPoint(name = "iidm_find_by_id")
    public static long findById(IsolateThread thread, long networkHandle, int objectType,
                                CCharPointer id) {
        String javaId = CTypeConversion.toJavaString(id);
        return PropertyDispatcher.findById(networkHandle, objectType, javaId);
    }

    @CEntryPoint(name = "iidm_release_handle")
    public static void releaseHandle(IsolateThread thread, long handle) {
        NetworkRegistry.release(handle);
    }
}
