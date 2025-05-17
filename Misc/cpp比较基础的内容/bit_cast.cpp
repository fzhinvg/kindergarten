//
// Created by fzhinvg on 2025/5/11.
//
// bit_cast
#include <iostream>
#include <cstdint>
#include <bit>

class Source
{
public:
    uint32_t value;
    explicit Source(uint32_t _value) : value(_value)
    {}
};

class Destination
{
public:
    uint8_t value1;
    uint8_t value2;
    uint8_t value3;
    uint8_t value4;
};

int main()
{
    Source source{0x44434241};

    // 类型之间安全地按位转换对象表示，核心思想是“直接重新解释内存”
    // 参与转换的两个类型需要满足都是平凡可复制的
    // 无运行时开销
    // 两个类型需要保持大小一致
    auto destination = std::bit_cast<Destination>(source);

    std::cout << destination.value1 << '\n' // A 0x41
              << destination.value2 << '\n' // B 0x42
              << destination.value3 << '\n' // C 0x43
              << destination.value4 << '\n' // D 0x44
              << std::endl;

    return 0;
}
#if 0
#include <bit>
#include <cstdint>
#include <iostream>
#include <bitset>

// 将 float 分解为符号、指数、尾数
void analyze_float(float f) {
    // 将 float 按位转换为 uint32_t（大小相同）
    uint32_t bits = std::bit_cast<uint32_t>(f);

    // 提取符号位（最高位）
    uint32_t sign_bit = (bits >> 31) & 0x1;

    // 提取指数部分（接下来的8位）
    uint32_t exponent_bits = (bits >> 23) & 0xFF;

    // 提取尾数部分（低23位）
    uint32_t mantissa_bits = bits & 0x7FFFFF;

    // 计算实际指数值（IEEE 754 偏移量是127）
    int exponent = static_cast<int>(exponent_bits) - 127;

    // 计算尾数值（隐含前导1，除非是次正规数）
    float mantissa = 1.0f + (static_cast<float>(mantissa_bits) / (1 << 23));

    std::cout << "Float value: " << f << std::endl;
    std::cout << "Bit pattern: 0b" << std::bitset<32>(bits) << std::endl;
    std::cout << "Sign: " << (sign_bit ? "Negative (-)" : "Positive (+)") << std::endl;
    std::cout << "Exponent (raw bits): 0b" << std::bitset<8>(exponent_bits)
              << " (dec: " << exponent_bits << "), actual exponent: " << exponent << std::endl;
    std::cout << "Mantissa (raw bits): 0b" << std::bitset<23>(mantissa_bits)
              << ", value: " << mantissa << std::endl;
    std::cout << "----------------------------------" << std::endl;
}

int main() {
    analyze_float(3.14f);     // 正数
    analyze_float(1.0f);      // 精确表示
    analyze_float(-0.0f);     // 负零
    analyze_float(0.0f);      // 零
    return 0;
}
#endif