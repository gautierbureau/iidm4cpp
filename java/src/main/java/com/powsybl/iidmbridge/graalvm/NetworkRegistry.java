package com.powsybl.iidmbridge.graalvm;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Maps long integer handles to Java objects (any IIDM object, not just Network).
 * Used by the GraalVM backend to pass object references across the C boundary.
 */
public final class NetworkRegistry {

    private static final Map<Long, Object> registry = new ConcurrentHashMap<>();
    private static final AtomicLong nextHandle = new AtomicLong(1);

    private NetworkRegistry() {}

    public static long register(Object obj) {
        long handle = nextHandle.getAndIncrement();
        registry.put(handle, obj);
        return handle;
    }

    public static Object lookup(long handle) {
        Object obj = registry.get(handle);
        if (obj == null) {
            throw new IllegalArgumentException("Unknown handle: " + handle);
        }
        return obj;
    }

    public static void release(long handle) {
        registry.remove(handle);
    }

    public static void releaseAll() {
        registry.clear();
    }
}
