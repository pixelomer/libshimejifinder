//
// libshimejifinder - library for finding and extracting shimeji from archives
// Copyright (C) 2025 pixelomer
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#if SHIMEJIFINDER_USE_JNI

#include "../utf8_convert.hpp"
#include <jni.h>

namespace shimejifinder {

JNIEnv *jni_env = nullptr;

static jclass CharsetDecoder;
static jclass Charset;
static jclass CodingErrorAction;
static jclass ByteBuffer;
static jclass StringBuilder;
static jmethodID mIDForName;
static jmethodID mIDNewDecoder;
static jmethodID mIDOnMalformedInput;
static jmethodID mIDOnUnmappableCharacter;
static jmethodID mIDDecode;
static jmethodID mIDWrap;
static jmethodID mIDAppend;
static jmethodID mIDStringBuilderInit;
static jmethodID mIDToString;
static jfieldID fIDREPORT;
static jobject oUTF8Charset;
static jobject oShiftJISCharset;
static jobject oCodingErrorActionReport;

static bool jni_objects_cached = false;

template<typename T>
static T make_global(T obj) {
    if (obj == nullptr) return nullptr;
    T globalRef = (T)jni_env->NewGlobalRef(obj);
    jni_env->DeleteLocalRef(obj);
    return globalRef;
}

static void init_jni() {
    if (!jni_objects_cached) {
        StringBuilder = make_global(jni_env->FindClass("java/lang/StringBuilder"));
        mIDStringBuilderInit = jni_env->GetMethodID(StringBuilder, "<init>", "()V");
        mIDToString = jni_env->GetMethodID(StringBuilder, "toString", "()Ljava/lang/String;");
        mIDAppend = jni_env->GetMethodID(StringBuilder, "append",
            "(Ljava/lang/CharSequence;)Ljava/lang/StringBuilder;");
        ByteBuffer = make_global(jni_env->FindClass("java/nio/ByteBuffer"));
        mIDWrap = jni_env->GetStaticMethodID(ByteBuffer, "wrap", "([B)Ljava/nio/ByteBuffer;");
        CharsetDecoder = make_global(jni_env->FindClass("java/nio/charset/CharsetDecoder"));
        Charset = make_global(jni_env->FindClass("java/nio/charset/Charset"));
        mIDForName = jni_env->GetStaticMethodID(Charset, "forName",
            "(Ljava/lang/String;)Ljava/nio/charset/Charset;");
        mIDNewDecoder = jni_env->GetMethodID(Charset, "newDecoder",
            "()Ljava/nio/charset/CharsetDecoder;");
        mIDOnMalformedInput = jni_env->GetMethodID(CharsetDecoder, "onMalformedInput",
            "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetDecoder;");
        mIDOnUnmappableCharacter = jni_env->GetMethodID(CharsetDecoder, "onUnmappableCharacter",
            "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetDecoder;");
        CodingErrorAction = make_global(jni_env->FindClass("java/nio/charset/CodingErrorAction"));
        fIDREPORT = jni_env->GetStaticFieldID(CodingErrorAction, "REPORT",
            "Ljava/nio/charset/CodingErrorAction;");
        mIDDecode = jni_env->GetMethodID(CharsetDecoder, "decode",
            "(Ljava/nio/ByteBuffer;)Ljava/nio/CharBuffer;");
        jstring sUTF8 = jni_env->NewStringUTF("UTF-8");
        oUTF8Charset = make_global(jni_env->CallStaticObjectMethod(Charset, mIDForName, sUTF8));
        jni_env->DeleteLocalRef(sUTF8);
        if (jni_env->ExceptionCheck()) {
            oUTF8Charset = nullptr;
            jni_env->ExceptionClear();
        }
        jstring sShiftJIS = jni_env->NewStringUTF("Shift_JIS");
        oShiftJISCharset = make_global(jni_env->CallStaticObjectMethod(Charset, mIDForName, sShiftJIS));
        jni_env->DeleteLocalRef(sShiftJIS);
        if (jni_env->ExceptionCheck()) {
            oShiftJISCharset = nullptr;
            jni_env->ExceptionClear();
        }
        oCodingErrorActionReport = make_global(jni_env->GetStaticObjectField(CodingErrorAction, fIDREPORT));
        jni_objects_cached = true;
    }
}

static bool jni_decode(jobject charset, const jbyte *c_bytes, jsize length, std::string *out) {
    #define discard(thiz, x) do { \
        ret = (x); \
        if (jni_env->ExceptionCheck()) { \
            ret = nullptr; \
            goto fail; \
        } \
        if (ret != thiz && ret != nullptr) { \
            jni_env->DeleteLocalRef(ret); \
        } \
        ret = nullptr; \
    } while(0)

    jobject decoder = nullptr;
    jobject ret = nullptr;
    jstring str = nullptr;
    jbyteArray bytes = nullptr;
    jobject charBuffer = nullptr;
    jobject stringBuilder = nullptr;
    jobject byteBuffer = nullptr;
    const char *chars = nullptr;
    bool valid = false, success = false;

    // create decoder
    decoder = jni_env->CallObjectMethod(charset, mIDNewDecoder);
    if (decoder == nullptr) goto fail;

    // configure decoder to throw on decode error
    discard(decoder, (jni_env->CallObjectMethod(decoder, mIDOnMalformedInput,
        oCodingErrorActionReport)));
    discard(decoder, (jni_env->CallObjectMethod(decoder, mIDOnUnmappableCharacter,
        oCodingErrorActionReport)));
    
    // create byte array to store original string bytes
    bytes = jni_env->NewByteArray(length);
    if (bytes == nullptr) goto fail;

    // copy bytes to byte array
    jni_env->SetByteArrayRegion(bytes, 0, length, c_bytes);

    // wrap byte array in a ByteBuffer object
    byteBuffer = jni_env->CallStaticObjectMethod(ByteBuffer, mIDWrap, bytes);
    if (byteBuffer == nullptr) goto fail;

    // attempt to decode the bytes
    charBuffer = jni_env->CallObjectMethod(decoder, mIDDecode, byteBuffer);

    // the decoder will throw an exception if the decode operation fails
    valid = !jni_env->ExceptionCheck();
    jni_env->ExceptionClear();

    // free resources that are no longer needed
#define clean(x) jni_env->DeleteLocalRef(x); x = nullptr;
    clean(byteBuffer);
    clean(bytes);
    clean(decoder);
#undef clean

    // check if decode operation was successful
    if (valid && charBuffer != nullptr) {
        success = true;
        
        // if requested, copy decoded string to output string
        if (out != nullptr) {
            // create StringBuilder
            stringBuilder = jni_env->NewObject(StringBuilder, mIDStringBuilderInit);
            if (stringBuilder == nullptr) goto fail;

            // append decoded string to StringBuilder
            discard(stringBuilder, (jni_env->CallObjectMethod(stringBuilder, mIDAppend,
                charBuffer)));

            // obtain final jstring
            str = (jstring)jni_env->CallObjectMethod(stringBuilder, mIDToString);
            if (str == nullptr) goto fail;

            // get UTF-8 bytes from jstring
            chars = jni_env->GetStringUTFChars(str, nullptr);
            if (chars == nullptr) goto fail;

            // obtain std::string from UTF-8 bytes
            *out = { chars, (size_t)jni_env->GetStringUTFLength(str) };
        }
    }

    goto cleanup;
fail:
    success = false;
cleanup:
    jni_env->ExceptionClear();
#define clean(x) if (x != nullptr) jni_env->DeleteLocalRef(x);
    if (chars != nullptr) {
        jni_env->ReleaseStringUTFChars(str, chars);
    }
    clean(str);
    clean(decoder);
    clean(ret);
    clean(bytes);
    clean(charBuffer);
    clean(stringBuilder);
    clean(byteBuffer);
#undef clean

#undef discard

    return success;
}

bool is_valid_utf8(const std::string &str) {
    init_jni();
    if (oUTF8Charset == nullptr) {
        // this should never happen
        // https://developer.android.com/reference/java/nio/charset/Charset#standard-charsets
        return false;
    }
    return jni_decode(oUTF8Charset, (const jbyte *)str.c_str(), (jsize)str.size(), nullptr);
}

bool shift_jis_to_utf8(std::string &str) {
    init_jni();
    if (oShiftJISCharset == nullptr) {
        // this may happen, in which case Shift_JIS archives will not be supported
        return false;
    }
    return jni_decode(oShiftJISCharset, (const jbyte *)str.c_str(), (jsize)str.size(), &str);
}

}

#endif
