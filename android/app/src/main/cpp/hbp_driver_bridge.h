#ifndef JSXPOSEDX_HBP_DRIVER_BRIDGE_H
#define JSXPOSEDX_HBP_DRIVER_BRIDGE_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 终极通用硬件断点执行引擎 - JNI 侧直连通道
 * 接口直接挂载到 JsxposedX 的 NativeBridge 类下
 */

JNIEXPORT jboolean JNICALL
Java_com_jsxposed_x_core_bridge_memory_1tool_1native_MemoryToolHelperNativeBridge_installHbp(
        JNIEnv* env, jobject thiz, jbyteArray req_payload);

JNIEXPORT jboolean JNICALL
Java_com_jsxposed_x_core_bridge_memory_1tool_1native_MemoryToolHelperNativeBridge_cleanupHbp(
        JNIEnv* env, jobject thiz);

#ifdef __cplusplus
}
#endif

#endif // JSXPOSEDX_HBP_DRIVER_BRIDGE_H
