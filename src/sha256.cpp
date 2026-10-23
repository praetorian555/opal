#include "opal/sha256.h"

#include <cstring>

namespace
{

using Opal::u32;
using Opal::u64;
using Opal::u8;

constexpr u32 k_initial_state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

constexpr u32 k_round_constants[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be,
    0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa,
    0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85,
    0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
    0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

u32 RotateRight(u32 value, u32 count)
{
    return (value >> count) | (value << (32u - count));
}

void Compress(u32 (&state)[8], const u8* block)
{
    u32 schedule[64];
    for (u32 i = 0; i < 16; ++i)
    {
        schedule[i] = (static_cast<u32>(block[4 * i]) << 24) | (static_cast<u32>(block[4 * i + 1]) << 16) |
                      (static_cast<u32>(block[4 * i + 2]) << 8) | static_cast<u32>(block[4 * i + 3]);
    }
    for (u32 i = 16; i < 64; ++i)
    {
        const u32 s0 = RotateRight(schedule[i - 15], 7) ^ RotateRight(schedule[i - 15], 18) ^ (schedule[i - 15] >> 3);
        const u32 s1 = RotateRight(schedule[i - 2], 17) ^ RotateRight(schedule[i - 2], 19) ^ (schedule[i - 2] >> 10);
        schedule[i] = schedule[i - 16] + s0 + schedule[i - 7] + s1;
    }

    u32 a = state[0];
    u32 b = state[1];
    u32 c = state[2];
    u32 d = state[3];
    u32 e = state[4];
    u32 f = state[5];
    u32 g = state[6];
    u32 h = state[7];
    for (u32 i = 0; i < 64; ++i)
    {
        const u32 sum1 = RotateRight(e, 6) ^ RotateRight(e, 11) ^ RotateRight(e, 25);
        const u32 choice = (e & f) ^ (~e & g);
        const u32 temp1 = h + sum1 + choice + k_round_constants[i] + schedule[i];
        const u32 sum0 = RotateRight(a, 2) ^ RotateRight(a, 13) ^ RotateRight(a, 22);
        const u32 majority = (a & b) ^ (a & c) ^ (b & c);
        const u32 temp2 = sum0 + majority;
        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

}  // namespace

Opal::Sha256::Sha256() noexcept : m_block{}
{
    std::memcpy(m_state, k_initial_state, sizeof(m_state));
}

void Opal::Sha256::Update(const u8* data, u64 size)
{
    if (size == 0)
    {
        return;
    }
    m_message_size += size;
    if (m_block_size > 0)
    {
        const u64 taken = size < 64 - m_block_size ? size : 64 - m_block_size;
        std::memcpy(m_block + m_block_size, data, taken);
        m_block_size += taken;
        data += taken;
        size -= taken;
        if (m_block_size < 64)
        {
            return;
        }
        Compress(m_state, m_block);
        m_block_size = 0;
    }
    for (; size >= 64; data += 64, size -= 64)
    {
        Compress(m_state, data);
    }
    if (size > 0)
    {
        std::memcpy(m_block, data, size);
        m_block_size = size;
    }
}

void Opal::Sha256::Update(ArrayView<const u8> data)
{
    Update(data.GetData(), data.GetSize());
}

Opal::Sha256Digest Opal::Sha256::Finalize()
{
    const u64 bit_count = m_message_size * 8;
    m_block[m_block_size++] = 0x80;
    if (m_block_size > 56)
    {
        std::memset(m_block + m_block_size, 0, 64 - m_block_size);
        Compress(m_state, m_block);
        m_block_size = 0;
    }
    std::memset(m_block + m_block_size, 0, 56 - m_block_size);
    for (u64 i = 0; i < 8; ++i)
    {
        m_block[56 + i] = static_cast<u8>(bit_count >> (56 - 8 * i));
    }
    Compress(m_state, m_block);

    Sha256Digest digest;
    for (u64 i = 0; i < 8; ++i)
    {
        for (u64 j = 0; j < 4; ++j)
        {
            digest.bytes[4 * i + j] = static_cast<u8>(m_state[i] >> (24 - 8 * j));
        }
    }
    *this = Sha256();
    return digest;
}

Opal::Sha256Digest Opal::Hash::CalcSha256(const u8* data, u64 size)
{
    Sha256 sha;
    sha.Update(data, size);
    return sha.Finalize();
}

Opal::Expected<Opal::StringUtf8, Opal::ErrorCode> Opal::ToHexString(const Sha256Digest& digest, AllocatorBase* allocator)
{
    constexpr char8 k_digits[] = "0123456789abcdef";
    char8 text[64];
    for (u64 i = 0; i < 32; ++i)
    {
        text[2 * i] = k_digits[digest.bytes[i] >> 4];
        text[2 * i + 1] = k_digits[digest.bytes[i] & 0xF];
    }
    return StringUtf8::Create(text, 64, allocator);
}
