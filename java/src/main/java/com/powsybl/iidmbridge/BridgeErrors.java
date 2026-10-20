package com.powsybl.iidmbridge;

import com.powsybl.commons.PowsyblException;

/**
 * Classifies Java exceptions for the C++ side, so that both backends raise the same C++
 * exception type for the same failure.
 *
 * <p>The kind codes must match {@code iidm::detail::BridgeErrorKind} in
 * {@code src/BridgeErrors.h}.</p>
 */
public final class BridgeErrors {

    public static final int OK                 = 0;
    public static final int GENERIC            = 1; // iidm::IidmException
    public static final int INVALID_HANDLE     = 2; // iidm::InvalidHandleException
    public static final int PROPERTY_NOT_FOUND = 3; // iidm::PropertyNotFoundException
    public static final int EXTENSION_NOT_FOUND = 4; // iidm::ExtensionNotFoundException
    public static final int VALIDATION         = 5; // iidm::ValidationException
    public static final int NETWORK_NOT_FOUND  = 6; // iidm::NetworkNotFoundException

    private BridgeErrors() {}

    /** A handle that does not designate any object (0, or released). */
    public static class InvalidHandleException extends IllegalArgumentException {
        public InvalidHandleException(String message) {
            super(message);
        }
    }

    /** A property, child type or relation code that is unknown for the object it is used on. */
    public static class PropertyNotFoundException extends IllegalArgumentException {
        public PropertyNotFoundException(String message) {
            super(message);
        }
    }

    /** The extension a property belongs to is not attached to the object. */
    public static class ExtensionNotFoundException extends IllegalStateException {
        public ExtensionNotFoundException(String message) {
            super(message);
        }
    }

    /** No network registered under the requested id (JNI embedded mode). */
    public static class NetworkNotFoundException extends IllegalStateException {
        public NetworkNotFoundException(String message) {
            super(message);
        }
    }

    public static int kindOf(Throwable t) {
        if (t instanceof InvalidHandleException) {
            return INVALID_HANDLE;
        }
        if (t instanceof PropertyNotFoundException || t instanceof ClassCastException
                || t instanceof NullPointerException) {
            // ClassCastException: the code exists but does not apply to this type of object.
            // NullPointerException: an optional part of the object is absent (tap changer,
            // reactive limits model...), e.g. getRatioTapChanger().getTargetV().
            return PROPERTY_NOT_FOUND;
        }
        if (t instanceof ExtensionNotFoundException) {
            return EXTENSION_NOT_FOUND;
        }
        if (t instanceof NetworkNotFoundException) {
            return NETWORK_NOT_FOUND;
        }
        if (t instanceof PowsyblException || t instanceof IllegalArgumentException
                || t instanceof IllegalStateException || t instanceof UnsupportedOperationException) {
            // The network model rejected the operation (invalid value, wrong topology, removal of
            // a voltage level that still has branches...)
            return VALIDATION;
        }
        return GENERIC;
    }

    /** Message for the C++ exception: the Java message, prefixed by the class for unexpected errors. */
    public static String message(Throwable t) {
        String msg = t.getMessage();
        if (kindOf(t) == GENERIC || t instanceof ClassCastException || t instanceof NullPointerException
                || msg == null || msg.isEmpty()) {
            return t.getClass().getName() + (msg == null || msg.isEmpty() ? "" : ": " + msg);
        }
        return msg;
    }
}
