package com.powsybl.iidmbridge.graalvm;

import com.powsybl.iidm.network.Network;
import com.powsybl.iidm.serde.NetworkSerDe;
import com.powsybl.iidmbridge.PropertyDispatcher;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.UnmanagedMemory;
import org.graalvm.nativeimage.c.CContext;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CLongPointer;
import org.graalvm.nativeimage.c.type.CIntPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;

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
            Network network = NetworkSerDe.read(Path.of(path));
            return NetworkRegistry.register(network);
        } catch (Exception e) {
            return 0L; // INVALID_HANDLE
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
