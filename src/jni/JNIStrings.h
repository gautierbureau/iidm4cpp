#pragma once

#include <jni.h>
#include <string>

namespace iidm { namespace detail {

// Strings cross the JNI boundary as UTF-16, converted here exactly as the GraalVM backend
// converts them in Java (JNI's own "UTF" functions use modified UTF-8, which differs for
// characters outside the BMP). Strings are C strings, as in the GraalVM backend: they end at
// the first NUL character.

// UTF-8 to UTF-16. False when the text is not well-formed UTF-8 (the caller then lets Java
// decode it, replacing the malformed sequences as the GraalVM backend does).
inline bool utf8ToUtf16(const char* text, std::u16string& out) {
    out.clear();
    const unsigned char* s = reinterpret_cast<const unsigned char*>(text);
    while (*s) {
        unsigned char c = *s;
        char32_t code;
        int extra;
        if (c < 0x80) {
            code = c;
            extra = 0;
        } else if (c >= 0xC2 && c <= 0xDF) {
            code = c & 0x1F;
            extra = 1;
        } else if (c >= 0xE0 && c <= 0xEF) {
            code = c & 0x0F;
            extra = 2;
        } else if (c >= 0xF0 && c <= 0xF4) {
            code = c & 0x07;
            extra = 3;
        } else {
            return false;
        }
        ++s;
        for (int i = 0; i < extra; ++i, ++s) {
            if ((*s & 0xC0) != 0x80) return false;
            code = (code << 6) | (*s & 0x3F);
        }
        // Overlong forms, surrogates and code points beyond Unicode are malformed
        if ((extra == 2 && code < 0x800) || (extra == 3 && (code < 0x10000 || code > 0x10FFFF))
            || (code >= 0xD800 && code <= 0xDFFF)) {
            return false;
        }
        if (code < 0x10000) {
            out.push_back(static_cast<char16_t>(code));
        } else {
            code -= 0x10000;
            out.push_back(static_cast<char16_t>(0xD800 + (code >> 10)));
            out.push_back(static_cast<char16_t>(0xDC00 + (code & 0x3FF)));
        }
    }
    return true;
}

// UTF-16 to UTF-8 as Java's String.getBytes(UTF_8): an unpaired surrogate becomes '?'.
inline std::string utf16ToUtf8(const jchar* text, jsize length) {
    std::string out;
    out.reserve(static_cast<std::size_t>(length));
    for (jsize i = 0; i < length; ++i) {
        char32_t code = text[i];
        if (code == 0) break; // a C string
        if (code >= 0xD800 && code <= 0xDBFF && i + 1 < length && text[i + 1] >= 0xDC00 && text[i + 1] <= 0xDFFF) {
            code = 0x10000 + ((code - 0xD800) << 10) + (text[++i] - 0xDC00);
        } else if (code >= 0xD800 && code <= 0xDFFF) {
            out.push_back('?');
            continue;
        }
        if (code < 0x80) {
            out.push_back(static_cast<char>(code));
        } else if (code < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (code >> 6)));
            out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        } else if (code < 0x10000) {
            out.push_back(static_cast<char>(0xE0 | (code >> 12)));
            out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | (code >> 18)));
            out.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
        }
    }
    return out;
}

// The text of a Java string (the local reference is kept)
inline std::string javaString(JNIEnv* env, jstring value) {
    const jsize length = env->GetStringLength(value);
    std::u16string chars(static_cast<std::size_t>(length), u'\0');
    if (length > 0) env->GetStringRegion(value, 0, length, reinterpret_cast<jchar*>(&chars[0]));
    return utf16ToUtf8(reinterpret_cast<const jchar*>(chars.data()), length);
}

} } // namespace iidm::detail
