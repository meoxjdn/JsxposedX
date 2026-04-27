
#include "hbp_driver_bridge.h"

#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <cstdint>

namespace wuwa_hbp {

    // 驱动通信协议 ID
    constexpr uint32_t CMD_HBP_INSTALL = 0x5A5A1001;
    constexpr uint32_t CMD_HBP_CLEANUP = 0x5A5A1002;
    constexpr const char* DEV_NAME = "/dev/logd_service";

    // 通用底层指令下发函数
    bool SendKernelCommand(uint32_t cmd_id, void* payload) {
        int fd = open(DEV_NAME, O_WRONLY);
        if (fd < 0) {
            return false;
        }

        // 构造给驱动的 16 字节头部包装
        struct CoreCmdPacket {
            uint32_t cmd_id;
            uint64_t payload_ptr;
        } __attribute__((packed)) pkt;

        pkt.cmd_id = cmd_id;
        pkt.payload_ptr = reinterpret_cast<uint64_t>(payload);

        // 一击入魂，砸进内核
        ssize_t ret = write(fd, &pkt, sizeof(pkt));
        close(fd);
        
        return ret == static_cast<ssize_t>(sizeof(pkt));
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_jsxposed_x_core_bridge_memory_1tool_1native_MemoryToolHelperNativeBridge_installHbp(
        JNIEnv* env, jobject /* thiz */, jbyteArray req_payload) {
    try {
        if (req_payload == nullptr) {
            return JNI_FALSE;
        }
        
        // 1. 获取 JS/Java 层传来的字节数组长度
        const jsize length = env->GetArrayLength(req_payload);
        
        // 2. 在 C++ 堆上分配一段连续内存（此处避开了 Java GC 导致内核态 copy_from_user 读到脏数据的致命缺陷）
        std::vector<uint8_t> buffer(static_cast<size_t>(length));
        
        // 3. 将数据拷贝到 Native 内存
        env->GetByteArrayRegion(req_payload, 0, length, reinterpret_cast<jbyte*>(buffer.data()));

        // 4. 将 Native 内存指针下发给驱动
        bool success = wuwa_hbp::SendKernelCommand(wuwa_hbp::CMD_HBP_INSTALL, buffer.data());
        
        return success ? JNI_TRUE : JNI_FALSE;
    } catch (...) {
        return JNI_FALSE;
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_jsxposed_x_core_bridge_memory_1tool_1native_MemoryToolHelperNativeBridge_cleanupHbp(
        JNIEnv* env, jobject /* thiz */) {
    try {
        bool success = wuwa_hbp::SendKernelCommand(wuwa_hbp::CMD_HBP_CLEANUP, nullptr);
        return success ? JNI_TRUE : JNI_FALSE;
    } catch (...) {
        return JNI_FALSE;
    }
}
