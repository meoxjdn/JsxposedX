#include <jni.h>
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

        // 构造给驱动的 16 字节头部包装，严格按照 #pragma pack 结构对齐
        struct CoreCmdPacket {
            uint32_t cmd_id;
            uint64_t payload_ptr;
        } __attribute__((packed)) pkt;

        pkt.cmd_id = cmd_id;
        // 🌟 大牛修复：增加 uintptr_t 中转，解决 armeabi-v7a (32位) 架构下指针大小不匹配的编译报错
        pkt.payload_ptr = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(payload));

        // 一击入魂，向字符设备写入指令
        ssize_t ret = write(fd, &pkt, sizeof(pkt));
        close(fd);
        
        return ret == static_cast<ssize_t>(sizeof(pkt));
    }
}

// JNI 接口：对应 Kotlin 中的 com.jsxposed.x.NewApiHook.installHbp
extern "C" JNIEXPORT jboolean JNICALL
Java_com_jsxposed_x_NewApiHook_installHbp(JNIEnv* env, jclass /* clazz */, jbyteArray req_payload) {
    try {
        if (req_payload == nullptr) {
            return JNI_FALSE;
        }
        
        const jsize length = env->GetArrayLength(req_payload);
        std::vector<uint8_t> buffer(static_cast<size_t>(length));
        env->GetByteArrayRegion(req_payload, 0, length, reinterpret_cast<jbyte*>(buffer.data()));

        bool success = wuwa_hbp::SendKernelCommand(wuwa_hbp::CMD_HBP_INSTALL, buffer.data());
        return success ? JNI_TRUE : JNI_FALSE;
    } catch (...) {
        return JNI_FALSE;
    }
}

// JNI 接口：对应 Kotlin 中的 com.jsxposed.x.NewApiHook.cleanupHbp
extern "C" JNIEXPORT jboolean JNICALL
Java_com_jsxposed_x_NewApiHook_cleanupHbp(JNIEnv* env, jclass /* clazz */) {
    try {
        bool success = wuwa_hbp::SendKernelCommand(wuwa_hbp::CMD_HBP_CLEANUP, nullptr);
        return success ? JNI_TRUE : JNI_FALSE;
    } catch (...) {
        return JNI_FALSE;
    }
}
