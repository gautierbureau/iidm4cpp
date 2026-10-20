package com.powsybl.iidmbridge.graalvm;

import com.powsybl.iidm.network.Network;
import com.powsybl.iidm.serde.NetworkSerDe;
import com.powsybl.iidmbridge.BridgeErrors;
import com.powsybl.iidmbridge.Creation;
import com.powsybl.iidmbridge.PropertyDispatcher;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.UnmanagedMemory;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CCharPointerPointer;
import org.graalvm.nativeimage.c.type.CDoublePointer;
import org.graalvm.nativeimage.c.type.CIntPointer;
import org.graalvm.nativeimage.c.type.CLongPointer;
import org.graalvm.nativeimage.c.type.WordPointer;
import org.graalvm.word.PointerBase;
import org.graalvm.word.WordFactory;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;

/**
 * GraalVM Native Image entry points.
 * Each method annotated with {@code @CEntryPoint} is exported as a C function
 * callable from the C++ GraalVMBackend.
 *
 * <p><b>Error protocol.</b> An exception escaping a {@code @CEntryPoint} aborts the whole
 * process, so every entry point that can fail catches {@link Throwable} and returns a status:
 * 0 on success, otherwise a {@link BridgeErrors} kind. Results are written through
 * out-pointers, only on success. After a failure the C++ side calls
 * {@code iidm_take_last_error} to get the message and throws the matching C++ exception.</p>
 *
 * <p>Buffers returned to C++ (strings, handle and int arrays, serialized networks) are
 * allocated with {@link UnmanagedMemory} and must be released with {@code iidm_free}.</p>
 */
public final class IidmEntryPoints {

    private static final ThreadLocal<Throwable> LAST_ERROR = new ThreadLocal<>();

    private IidmEntryPoints() {}

    private static int fail(Throwable t) {
        LAST_ERROR.set(t);
        return BridgeErrors.kindOf(t);
    }

    private static CCharPointer toCString(String value) {
        byte[] bytes = value.getBytes(StandardCharsets.UTF_8);
        CCharPointer ptr = UnmanagedMemory.malloc(bytes.length + 1);
        for (int i = 0; i < bytes.length; i++) {
            ptr.write(i, bytes[i]);
        }
        ptr.write(bytes.length, (byte) 0);
        return ptr;
    }

    /** A C string from C++, as UTF-8 (the JNI backend converts the same way); a null pointer is null. */
    private static String fromCString(CCharPointer value) {
        if (value.isNull()) {
            return null;
        }
        int length = 0;
        while (value.read(length) != 0) {
            length++;
        }
        byte[] bytes = new byte[length];
        for (int i = 0; i < length; i++) {
            bytes[i] = value.read(i);
        }
        return new String(bytes, StandardCharsets.UTF_8);
    }

    /** No value (null) is a null pointer. */
    private static CCharPointer toNullableCString(String value) {
        return value == null ? WordFactory.nullPointer() : toCString(value);
    }

    // ── Errors and memory ─────────────────────────────────────────────────

    /**
     * Message of the last failure of the calling thread (UTF-8, to release with
     * {@code iidm_free}), or null when there is none. Clears it.
     */
    @CEntryPoint(name = "iidm_take_last_error")
    public static CCharPointer takeLastError(IsolateThread thread) {
        Throwable t = LAST_ERROR.get();
        LAST_ERROR.remove();
        if (t == null) {
            return WordFactory.nullPointer();
        }
        try {
            return toCString(BridgeErrors.message(t));
        } catch (Throwable e) {
            return WordFactory.nullPointer();
        }
    }

    @CEntryPoint(name = "iidm_free")
    public static void free(IsolateThread thread, PointerBase ptr) {
        if (ptr.isNonNull()) {
            UnmanagedMemory.free(ptr);
        }
    }

    // ── Network lifecycle ─────────────────────────────────────────────────

    @CEntryPoint(name = "iidm_load_network")
    public static int loadNetwork(IsolateThread thread, CCharPointer filePath, CLongPointer outHandle) {
        try {
            Network network = Network.read(Path.of(fromCString(filePath)));
            outHandle.write(NetworkRegistry.register(network));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_create_network")
    public static int createNetwork(IsolateThread thread, CCharPointer id, CLongPointer outHandle) {
        try {
            outHandle.write(NetworkRegistry.register(Creation.createNetwork(fromCString(id))));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_save_network")
    public static int saveNetwork(IsolateThread thread, long handle, CCharPointer filePath) {
        try {
            Network network = (Network) NetworkRegistry.lookup(handle);
            NetworkSerDe.write(network, Path.of(fromCString(filePath)));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    /** Load a network from an in-memory byte buffer. */
    @CEntryPoint(name = "iidm_load_network_bytes")
    public static int loadNetworkBytes(IsolateThread thread,
                                       CCharPointer data, int length,
                                       CCharPointer filenameHint,
                                       CLongPointer outHandle) {
        try {
            byte[] bytes = new byte[length];
            for (int i = 0; i < length; i++) {
                bytes[i] = data.read(i);
            }
            // filenameHint is reserved for future multi-format support;
            // NetworkSerDe currently only handles XIIDM via InputStream.
            try (ByteArrayInputStream in = new ByteArrayInputStream(bytes)) {
                outHandle.write(NetworkRegistry.register(NetworkSerDe.read(in)));
            }
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    /** Serialize a network into a freshly allocated unmanaged byte buffer. */
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
            CCharPointer ptr = UnmanagedMemory.malloc(Math.max(bytes.length, 1));
            for (int i = 0; i < bytes.length; i++) {
                ptr.write(i, bytes[i]);
            }
            outData.write(ptr);
            outLen.write(bytes.length);
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_release_handle")
    public static void releaseHandle(IsolateThread thread, long handle) {
        NetworkRegistry.release(handle);
    }

    /** Number of objects registered in the isolate (diagnostics, tests). */
    @CEntryPoint(name = "iidm_handle_count")
    public static int handleCount(IsolateThread thread) {
        return NetworkRegistry.size();
    }

    // ── Scalar properties ─────────────────────────────────────────────────

    @CEntryPoint(name = "iidm_get_double")
    public static int getDouble(IsolateThread thread, long handle, int property, CDoublePointer out) {
        try {
            out.write(PropertyDispatcher.getDouble(handle, property));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_set_double")
    public static int setDouble(IsolateThread thread, long handle, int property, double value) {
        try {
            PropertyDispatcher.setDouble(handle, property, value);
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_get_int")
    public static int getInt(IsolateThread thread, long handle, int property, CIntPointer out) {
        try {
            out.write(PropertyDispatcher.getInt(handle, property));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_set_int")
    public static int setInt(IsolateThread thread, long handle, int property, int value) {
        try {
            PropertyDispatcher.setInt(handle, property, value);
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_get_bool")
    public static int getBool(IsolateThread thread, long handle, int property, CIntPointer out) {
        try {
            out.write(PropertyDispatcher.getBool(handle, property));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_set_bool")
    public static int setBool(IsolateThread thread, long handle, int property, int value) {
        try {
            PropertyDispatcher.setBool(handle, property, value);
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_get_string")
    public static int getString(IsolateThread thread, long handle, int property, CCharPointerPointer out) {
        try {
            out.write(toCString(PropertyDispatcher.getString(handle, property)));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_set_string")
    public static int setString(IsolateThread thread, long handle, int property, CCharPointer value) {
        try {
            PropertyDispatcher.setString(handle, property, fromCString(value));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    /** Writes the value, or {@link PropertyDispatcher#NO_INT} when there is none. */
    @CEntryPoint(name = "iidm_get_opt_int")
    public static int getOptInt(IsolateThread thread, long handle, int property, CLongPointer out) {
        try {
            out.write(PropertyDispatcher.getOptInt(handle, property));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    /** Writes the value, or a null pointer when there is none. */
    @CEntryPoint(name = "iidm_get_opt_string")
    public static int getOptString(IsolateThread thread, long handle, int property, CCharPointerPointer out) {
        try {
            out.write(toNullableCString(PropertyDispatcher.getOptString(handle, property)));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    /**
     * Writes the strings one after the other, each followed by a 0 byte, in one freshly
     * allocated buffer (null when empty), and the length of the buffer.
     */
    @CEntryPoint(name = "iidm_get_string_list")
    public static int getStringList(IsolateThread thread, long handle, int listCode,
                                    CCharPointerPointer outData, CIntPointer outLength) {
        try {
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            for (String value : PropertyDispatcher.getStringList(handle, listCode)) {
                buffer.writeBytes(value.getBytes(StandardCharsets.UTF_8));
                buffer.write(0);
            }
            byte[] bytes = buffer.toByteArray();
            CCharPointer ptr = WordFactory.nullPointer();
            if (bytes.length > 0) {
                ptr = UnmanagedMemory.malloc(bytes.length);
                for (int i = 0; i < bytes.length; i++) {
                    ptr.write(i, bytes[i]);
                }
            }
            outData.write(ptr);
            outLength.write(bytes.length);
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    /** Writes the value under the key, or a null pointer when there is none. */
    @CEntryPoint(name = "iidm_get_keyed_string")
    public static int getKeyedString(IsolateThread thread, long handle, int property, CCharPointer key,
                                     CCharPointerPointer out) {
        try {
            out.write(toNullableCString(PropertyDispatcher.getKeyedString(handle, property, fromCString(key))));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    /** A null value removes the key. */
    @CEntryPoint(name = "iidm_set_keyed_string")
    public static int setKeyedString(IsolateThread thread, long handle, int property, CCharPointer key,
                                     CCharPointer value) {
        try {
            PropertyDispatcher.setKeyedString(handle, property, fromCString(key), fromCString(value));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_is_same_object")
    public static int isSameObject(IsolateThread thread, long handle, long otherHandle, CIntPointer out) {
        try {
            out.write(PropertyDispatcher.isSameObject(handle, otherHandle));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    // ── Navigation ────────────────────────────────────────────────────────

    /** Writes a freshly allocated handle array (null when empty) and its length. */
    @CEntryPoint(name = "iidm_get_children")
    public static int getChildren(IsolateThread thread, long parentHandle, int childType,
                                  WordPointer outHandles, CIntPointer outCount) {
        try {
            long[] handles = PropertyDispatcher.getChildren(parentHandle, childType);
            CLongPointer arr = WordFactory.nullPointer();
            if (handles.length > 0) {
                arr = UnmanagedMemory.malloc(handles.length * Long.BYTES);
                for (int i = 0; i < handles.length; i++) {
                    arr.write(i, handles[i]);
                }
            }
            outHandles.write(arr);
            outCount.write(handles.length);
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    /** Writes a freshly allocated int array (null when empty) and its length. */
    @CEntryPoint(name = "iidm_get_int_list")
    public static int getIntList(IsolateThread thread, long handle, int listCode,
                                 WordPointer outValues, CIntPointer outCount) {
        try {
            int[] values = PropertyDispatcher.getIntList(handle, listCode);
            CIntPointer arr = WordFactory.nullPointer();
            if (values.length > 0) {
                arr = UnmanagedMemory.malloc(values.length * Integer.BYTES);
                for (int i = 0; i < values.length; i++) {
                    arr.write(i, values[i]);
                }
            }
            outValues.write(arr);
            outCount.write(values.length);
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_get_related")
    public static int getRelated(IsolateThread thread, long handle, int relation, CLongPointer out) {
        try {
            out.write(PropertyDispatcher.getRelated(handle, relation));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_set_related")
    public static int setRelated(IsolateThread thread, long handle, int relation, long valueHandle) {
        try {
            PropertyDispatcher.setRelated(handle, relation, valueHandle);
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_invoke")
    public static int invoke(IsolateThread thread, long handle, int code, CLongPointer out) {
        try {
            out.write(PropertyDispatcher.invoke(handle, code));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    /** args: count doubles (null when count is 0). */
    @CEntryPoint(name = "iidm_evaluate")
    public static int evaluate(IsolateThread thread, long handle, int code, CDoublePointer args, int count,
                               CDoublePointer out) {
        try {
            double[] values = new double[count];
            for (int i = 0; i < count; i++) {
                values[i] = args.read(i);
            }
            out.write(PropertyDispatcher.evaluate(handle, code, values));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_get_related_by_index")
    public static int getRelatedByIndex(IsolateThread thread, long handle, int relation, int index,
                                        CLongPointer out) {
        try {
            out.write(PropertyDispatcher.getRelatedByIndex(handle, relation, index));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_get_related_by_string")
    public static int getRelatedByString(IsolateThread thread, long handle, int relation, CCharPointer key,
                                         CLongPointer out) {
        try {
            out.write(PropertyDispatcher.getRelatedByString(handle, relation, fromCString(key)));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }

    @CEntryPoint(name = "iidm_find_by_id")
    public static int findById(IsolateThread thread, long networkHandle, int objectType,
                               CCharPointer id, CLongPointer out) {
        try {
            out.write(PropertyDispatcher.findById(networkHandle, objectType, fromCString(id)));
            return BridgeErrors.OK;
        } catch (Throwable t) {
            return fail(t);
        }
    }
}
