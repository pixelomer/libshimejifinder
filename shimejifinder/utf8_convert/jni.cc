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

static jclass CharsetDecoder = nullptr;
static jclass Charset = nullptr;
static jclass CodingErrorAction = nullptr;
static jclass ByteBuffer = nullptr;
static jclass StringBuilder = nullptr;
static jmethodID mIDForName = nullptr;
static jmethodID mIDNewDecoder = nullptr;
static jmethodID mIDOnMalformedInput = nullptr;
static jmethodID mIDOnUnmappableCharacter = nullptr;
static jmethodID mIDDecode = nullptr;
static jmethodID mIDWrap = nullptr;
static jmethodID mIDAppend = nullptr;
static jmethodID mIDStringBuilderInit = nullptr;
static jmethodID mIDToString = nullptr;
static jfieldID fIDREPORT = nullptr;
static jobject oUTF8Charset = nullptr;
static jobject oShiftJISCharset = nullptr;
static jobject oCodingErrorActionReport = nullptr;

static bool jni_objects_cached = false;
static bool jni_init_attempted = false;

bool init_jni() {
    if (!jni_init_attempted) {
        jni_init_attempted = true;

        #define safe_load(a, b) do { \
            a = (b); \
            if (jni_env->ExceptionCheck() || a == nullptr) { \
                if (jni_env->ExceptionCheck()) jni_env->ExceptionClear(); \
                a = nullptr; \
                return false; \
            } \
        } while (0)

        #define make_global(ref) do { \
            jobject globalRef = (jobject)jni_env->NewGlobalRef(ref); \
            if (jni_env->ExceptionCheck() || globalRef == nullptr) { \
                if (jni_env->ExceptionCheck()) jni_env->ExceptionClear(); \
                jni_env->DeleteLocalRef(ref); \
                ref = nullptr; \
                return false; \
            } \
            jni_env->DeleteLocalRef(ref); \
            ref = (decltype(ref))globalRef; \
        } while (0)

        // StringBuilder class
        safe_load(StringBuilder, jni_env->FindClass("java/lang/StringBuilder"));
        make_global(StringBuilder);

        // StringBuilder methods
        safe_load(mIDStringBuilderInit, jni_env->GetMethodID(StringBuilder,
            "<init>", "()V"));
        safe_load(mIDToString, jni_env->GetMethodID(StringBuilder, "toString",
            "()Ljava/lang/String;"));
        safe_load(mIDAppend, jni_env->GetMethodID(StringBuilder, "append",
            "(Ljava/lang/CharSequence;)Ljava/lang/StringBuilder;"));
        
        // ByteBuffer class
        safe_load(ByteBuffer, jni_env->FindClass("java/nio/ByteBuffer"));
        make_global(ByteBuffer);

        // ByteBuffer methods
        safe_load(mIDWrap, jni_env->GetStaticMethodID(ByteBuffer, "wrap",
            "([B)Ljava/nio/ByteBuffer;"));
        
        // CharsetDecoder class
        safe_load(CharsetDecoder, jni_env->FindClass("java/nio/charset/CharsetDecoder"));
        make_global(CharsetDecoder);

        // CharsetDecoder methods
        safe_load(mIDOnMalformedInput, jni_env->GetMethodID(CharsetDecoder,
            "onMalformedInput",
            "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetDecoder;"));
        safe_load(mIDOnUnmappableCharacter, jni_env->GetMethodID(CharsetDecoder,
            "onUnmappableCharacter",
            "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetDecoder;"));
        safe_load(mIDDecode, jni_env->GetMethodID(CharsetDecoder, "decode",
            "(Ljava/nio/ByteBuffer;)Ljava/nio/CharBuffer;"));

        // Charset class
        safe_load(Charset, jni_env->FindClass("java/nio/charset/Charset"));
        make_global(Charset);

        // Charset methods
        safe_load(mIDForName, jni_env->GetStaticMethodID(Charset, "forName",
            "(Ljava/lang/String;)Ljava/nio/charset/Charset;"));
        safe_load(mIDNewDecoder, jni_env->GetMethodID(Charset, "newDecoder",
            "()Ljava/nio/charset/CharsetDecoder;"));
        
        // CodingErrorAcion class
        safe_load(CodingErrorAction, jni_env->FindClass("java/nio/charset/CodingErrorAction"));
        make_global(CodingErrorAction);

        // CodingErrorAction fields
        safe_load(fIDREPORT, jni_env->GetStaticFieldID(CodingErrorAction, "REPORT",
            "Ljava/nio/charset/CodingErrorAction;"));
        safe_load(oCodingErrorActionReport, jni_env->GetStaticObjectField(CodingErrorAction,
            fIDREPORT));
        make_global(oCodingErrorActionReport);
        
        // UTF-8 charset (required)
        jstring sUTF8;
        safe_load(sUTF8, jni_env->NewStringUTF("UTF-8"));
        oUTF8Charset = jni_env->CallStaticObjectMethod(Charset, mIDForName, sUTF8);
        if (jni_env->ExceptionCheck() || oUTF8Charset == nullptr) {
            if (jni_env->ExceptionCheck()) jni_env->ExceptionClear();
            oUTF8Charset = nullptr;
            jni_env->DeleteLocalRef(sUTF8);
            return false;
        }
        make_global(oUTF8Charset);
        jni_env->DeleteLocalRef(sUTF8);

        // Shift_JIS charset (optional)
        jstring sShiftJIS;
        safe_load(sShiftJIS, jni_env->NewStringUTF("Shift_JIS"));
        oShiftJISCharset = jni_env->CallStaticObjectMethod(Charset, mIDForName, sShiftJIS);
        if (jni_env->ExceptionCheck() || oShiftJISCharset == nullptr) {
            if (jni_env->ExceptionCheck()) jni_env->ExceptionClear();
            oShiftJISCharset = nullptr;
            jni_env->DeleteLocalRef(sShiftJIS);
        }
        else {
            jni_env->DeleteLocalRef(sShiftJIS);
            make_global(oShiftJISCharset);
        }

        // All JNI objects have been cached
        jni_objects_cached = true;

        #undef safe_load
        #undef make_global
    }
    return jni_objects_cached;
}

__attribute__((noinline))
bool jni_decode(jobject charset, const jbyte *c_bytes, jsize length, std::string *out) {
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
    if (jni_env->ExceptionCheck() || decoder == nullptr) {
        decoder = nullptr;
        goto fail;
    }

    // configure decoder to throw on decode error
    discard(decoder, (jni_env->CallObjectMethod(decoder, mIDOnMalformedInput,
        oCodingErrorActionReport)));
    discard(decoder, (jni_env->CallObjectMethod(decoder, mIDOnUnmappableCharacter,
        oCodingErrorActionReport)));
    
    // create byte array to store original string bytes
    bytes = jni_env->NewByteArray(length);
    if (jni_env->ExceptionCheck() || bytes == nullptr) {
        bytes = nullptr;
        goto fail;
    }

    // copy bytes to byte array
    jni_env->SetByteArrayRegion(bytes, 0, length, c_bytes);

    // wrap byte array in a ByteBuffer object
    byteBuffer = jni_env->CallStaticObjectMethod(ByteBuffer, mIDWrap, bytes);
    if (jni_env->ExceptionCheck() || byteBuffer == nullptr) {
        byteBuffer = nullptr;
        goto fail;
    }

    // attempt to decode the bytes
    charBuffer = jni_env->CallObjectMethod(decoder, mIDDecode, byteBuffer);

    // the decoder will throw an exception if the decode operation fails
    if (jni_env->ExceptionCheck()) {
        valid = false;
        jni_env->ExceptionClear();
    }
    else {
        valid = true;
    }

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
            if (jni_env->ExceptionCheck() || stringBuilder == nullptr) {
                stringBuilder = nullptr;
                goto fail;
            }

            // append decoded string to StringBuilder
            discard(stringBuilder, (jni_env->CallObjectMethod(stringBuilder, mIDAppend,
                charBuffer)));

            // obtain final jstring
            str = (jstring)jni_env->CallObjectMethod(stringBuilder, mIDToString);
            if (jni_env->ExceptionCheck() || str == nullptr) {
                str = nullptr;
                goto fail;
            }

            // get UTF-8 bytes from jstring
            chars = jni_env->GetStringUTFChars(str, nullptr);
            if (jni_env->ExceptionCheck() || chars == nullptr) {
                chars = nullptr;
                goto fail;
            }

            // obtain std::string from UTF-8 bytes
            *out = { chars, (size_t)jni_env->GetStringUTFLength(str) };
        }
    }

    goto cleanup;
fail:
    success = false;
cleanup:
    if (jni_env->ExceptionCheck()) {
        jni_env->ExceptionClear();
    }
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
    if (!init_jni() || oUTF8Charset == nullptr) {
        // it is generally preferable to pretend all strings are valid
        // utf-8 if JNI initialization fails for whatever reason
        return true;
    }
    return jni_decode(oUTF8Charset, (const jbyte *)str.c_str(), (jsize)str.size(), nullptr);
}

bool shift_jis_to_utf8(std::string &str) {
    if (!init_jni() || oShiftJISCharset == nullptr) {
        return false;
    }
    return jni_decode(oShiftJISCharset, (const jbyte *)str.c_str(), (jsize)str.size(), &str);
}

}

#endif
