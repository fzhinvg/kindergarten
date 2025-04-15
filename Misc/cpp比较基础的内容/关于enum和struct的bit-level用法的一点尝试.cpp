//
// Created by fzhinvg on 2024/12/27.
// update date: 2025/04/15
// 采用了更加具体的场景演示
#include <iostream>
#include <cstdint>
#include <bit>
#include <bitset>
#include <climits>

//#include <winsock2.h> // for htonl() ntohl()
//#pragma comment(lib, "Ws2_32.lib")  // 自动链接库

// ========== 1. 客户端权限 (枚举 + 位掩码) ===========
// 客户端只管理玩家操作权限
enum class ClientPermission : uint8_t
{
    None = 0b0000'0000,
    Move = 0b0000'0001, //0x01<<0
    Attack = 0b0000'0010, //0x01<<1
    Trade = 0b0000'0100,
    Default = Move | Attack | Trade
};

// 客户端权限运算符重载
inline ClientPermission operator|(ClientPermission lhs, ClientPermission rhs)
{
    return static_cast<ClientPermission>(
            static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
    );
}

inline ClientPermission operator&(ClientPermission lhs, ClientPermission rhs)
{
    return static_cast<ClientPermission>(
            static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
    );
}

inline ClientPermission operator~(ClientPermission permission)
{
    return static_cast<ClientPermission>(
            ~static_cast<uint8_t>(permission)  // 取反底层值
    );
}

// ========== 2. 玩家异常状态 (位域) ===========
// 专注游戏内角色状态（客户端管理）
struct CharacterStatus
{
    uint8_t isBurning: 1;  // 灼烧状态
    uint8_t isStunned: 1;  // 眩晕状态
    uint8_t isSleeping: 1; // 睡眠状态
    uint8_t : 5;           // 保留位（可扩展）
};

// ========== 3. 账户状态 (独立管理) ===========
// 服务端管理（伪代码示意）
struct AccountStatus
{
    bool isOnline;   // 是否在线
    bool isBanned;   // 是否封禁
};

// ========== 4. 工具函数 ===========
void printPermissions(ClientPermission permission)
{
    std::bitset<8> bits(static_cast<uint8_t>(permission));
    std::cout << "Client Permissions: " << bits << "\n";
}

void printCharacterStatus(const CharacterStatus &status)
{
    std::cout << "Character Status: ["
              << " Burning=" << static_cast<bool>(status.isBurning)
              << " Stunned=" << static_cast<bool>(status.isStunned)
              << " Sleeping=" << static_cast<bool>(status.isSleeping)
              << "]\n";
    std::cout << "Binary: "
              << std::bitset<8>(*reinterpret_cast<const uint8_t *>(&status))
              << "\n";
}

// ========== 5. 主函数 ===========
int main()
{
    if constexpr (std::endian::native == std::endian::little)
    {
        std::cout << "System is little-endian\n";
        //    0x12345678
        //	  Little-Endian 低位字节存储在低地址
        //	  地址：0x1000  0x1001  0x1002  0x1003
        //	  数据：  0x78    0x56    0x34    0x12
    } else
    {
        std::cout << "System is big-endian\n";
        //	  Big-Endian 高位字节存储在低地址
        //	  地址：0x1000  0x1001  0x1002  0x1003
        //	  数据：  0x12    0x34    0x56    0x78
    }
    // 字节序会影响包含多字节成员变量的类实例的实际内存布局
    // 但不会影响类成员之间的相对顺序和对齐规则

    // ----- 1. 客户端权限演示 ------
    std::cout << "[1. Client Permission Management]\n";
    ClientPermission clientPermission = ClientPermission::Default;
    printPermissions(clientPermission);
    // 禁用交易权限
    clientPermission = clientPermission & ~ClientPermission::Trade;
    std::cout << "After disabling Trade:\n";
    printPermissions(clientPermission);

    // ----- 2. 异常状态演示 ------
    std::cout << "\n[2. Character Status (Bit-field)]\n";
    CharacterStatus status{};
    status.isBurning = 1;
    status.isStunned = 1;
    printCharacterStatus(status);

    // 使用<bitset>检查状态
    std::bitset<8> stateBits(*reinterpret_cast<uint8_t *>(&status));
    std::cout << "Is character stunned? "
              << stateBits.test(1) << "\n";

    // ----- 3. <bit> 操作演示 ------
    std::cout << "\n[3. <bit> Advanced Operations]\n";
    uint32_t value = 0b1101'0101;
    std::cout << "Trailing zeros in " << std::bitset<8>(value)
              << ": " << std::countr_zero(value) << "\n";

    return 0;
}
// os更多采用小端序的原因：
// 除开历史原因；
// 1.硬件实现更简单，电路设计更简单，地址计算更直接
// 2.性能优化优势，内存访问效率，比如int16转型成int32，低地址已包含有效数据，不需要做位移
//   加法乘法等从低位数开始计算
// 还有更多，但是大端序与之相比没有任何优越性
// 那为什么网络字节序更偏爱大端序呢？
// 除开历史原因；
// 1.人类可读性与调试便利，比如再用wireshark抓包的时候
// 抓包示例（IPv4头部）：
// 源IP地址：C0 A8 01 01 → 192.168.1.1（无需反转字节）
// 便于抓包分析
// 2.按传输顺序直接拼接（如 [0x12, 0x34] → 0x1234）
// 3.新增字段时保持高位在前的一致性

// 重点在于不同的场景选择合适的工具，而不是一味地追求采用看起来贴近底层、原始的写法
// 无论是位掩码、结构化数据还是数据库，每种方案都有其独特的优势和适用场景
// 1.简单场景：开关型状态管理
//     推荐：位掩码/bool
//     适用条件：状态为布尔值 true or false；状态数量可控（少于 64 个）可以用一两个字节表示的；高频读写，性能敏感
//     不过如果涉及到高频读写，我反而不推荐使用位掩码，而是直接使用bool操作一个字节，这样可以避免高频次的无意义的位运算反而提高了运行指令数量
// 2.中等场景：复杂进度与依赖关系
//     推荐：结构化数据（JSON/XML）
//     适用条件：需要存储非布尔值（如进度百分比、任务阶段）；存在状态依赖（如任务A完成后才能触发任务B）；需要跨平台或与外部工具（如关卡编辑器）交互
// 3.复杂场景：大型游戏与在线服务
//     推荐工具：数据库
//     适用条件：海量玩家数据（如MMO游戏）；需要事务支持（如任务奖励原子性发放）；复杂查询需求（如“统计全服未完成某任务的玩家”）。

// 当然，实际使用肯定是上面各种场景在业务的不同层级出现，实际上需要混合使用，所以重点在于不同的场景选择合适的工具