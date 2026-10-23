#include "test-helpers.h"

#include <cstring>

#include "opal/sha256.h"

using namespace Opal;

namespace
{

StringUtf8 Hex(const Sha256Digest& digest)
{
    Expected<StringUtf8, ErrorCode> hex = ToHexString(digest);
    REQUIRE(hex.HasValue());
    return Move(hex.GetValue());
}

Sha256Digest OfText(const char* text)
{
    return Hash::CalcSha256(reinterpret_cast<const u8*>(text), std::strlen(text));
}

}  // namespace

TEST_CASE("SHA-256 matches the FIPS 180-4 examples", "[Sha256]")
{
    REQUIRE(Hex(Hash::CalcSha256(nullptr, 0)) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    REQUIRE(Hex(OfText("abc")) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    REQUIRE(Hex(OfText("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")) ==
            "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");

    // A million 'a's, fed a thousand at a time.
    u8 block[1000];
    std::memset(block, 'a', sizeof(block));
    Sha256 sha;
    for (int i = 0; i < 1000; ++i)
    {
        sha.Update(block, sizeof(block));
    }
    REQUIRE(Hex(sha.Finalize()) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}

TEST_CASE("SHA-256 pads messages around the block boundary", "[Sha256]")
{
    // 55 bytes leave room for the length in the last block, 56 do not; 64 and 119 end on a block or just short of two.
    u8 block[128];
    std::memset(block, 'a', sizeof(block));
    REQUIRE(Hex(Hash::CalcSha256(block, 55)) == "9f4390f8d30c2dd92ec9f095b65e2b9ae9b0a925a5258e241c9f1e910f734318");
    REQUIRE(Hex(Hash::CalcSha256(block, 56)) == "b35439a4ac6f0948b6d6f9e3c6af0f5f590ce20f1bde7090ef7970686ec6738a");
    REQUIRE(Hex(Hash::CalcSha256(block, 63)) == "7d3e74a05d7db15bce4ad9ec0658ea98e3f06eeecf16b4c6fff2da457ddc2f34");
    REQUIRE(Hex(Hash::CalcSha256(block, 64)) == "ffe054fe7ae0cb6dc65c3af9b61d5209f439851db43d0ba5997337df154668eb");
    REQUIRE(Hex(Hash::CalcSha256(block, 65)) == "635361c48bb9eab14198e76ea8ab7f1a41685d6ad62aa9146d301d4f17eb0ae0");
    REQUIRE(Hex(Hash::CalcSha256(block, 119)) == "31eba51c313a5c08226adf18d4a359cfdfd8d2e816b13f4af952f7ea6584dcfb");
}

TEST_CASE("SHA-256 hashes every byte value", "[Sha256]")
{
    // Reference digests from GNU sha256sum. The ASCII examples above never set a byte's top bit.
    u8 all_values[4 * 256 + 61];
    for (u64 i = 0; i < 4 * 256; ++i)
    {
        all_values[i] = static_cast<u8>(i);
    }
    std::memset(all_values + 4 * 256, 0xff, 61);
    REQUIRE(Hex(Hash::CalcSha256(all_values, 256)) == "40aff2e9d2d8922e47afd4648e6967497158785fbd1da870e7110266bf944880");
    REQUIRE(Hex(Hash::CalcSha256(all_values, sizeof(all_values))) == "97af4a6e7cd3ec72fc0300cf2646fbf48b90801e6c744e0f35bb5ec14784ed3d");

    // Bytes from a linear congruential generator, x = x * 1103515245 + 12345, taking the top byte.
    u8 noise[4097];
    u32 state = 1;
    for (u8& byte : noise)
    {
        state = state * 1103515245u + 12345u;
        byte = static_cast<u8>(state >> 24);
    }
    REQUIRE(Hex(Hash::CalcSha256(noise, 1000)) == "dd8c914beb8d49a6b6f3eb6302c9175821c2225fe45d6ff66260cc72d61da26c");
    REQUIRE(Hex(Hash::CalcSha256(noise, sizeof(noise))) == "8697e32e92863e7040bd316487c9de01a011e979e60753309661322e2e3f1e42");
}

TEST_CASE("SHA-256 gives the same digest however the message is split", "[Sha256]")
{
    u8 message[200];
    for (u64 i = 0; i < sizeof(message); ++i)
    {
        message[i] = static_cast<u8>(i * 7 % 251);
    }
    const Sha256Digest whole = Hash::CalcSha256(message, sizeof(message));

    for (u64 split = 0; split <= sizeof(message); ++split)
    {
        CAPTURE(split);
        Sha256 sha;
        sha.Update(message, split);
        sha.Update(ArrayView<const u8>(message + split, sizeof(message) - split));
        REQUIRE(sha.Finalize() == whole);
    }

    Sha256 bytewise;
    for (const u8 byte : message)
    {
        bytewise.Update(&byte, 1);
    }
    REQUIRE(bytewise.Finalize() == whole);
}

TEST_CASE("SHA-256 starts over after Finalize", "[Sha256]")
{
    Sha256 sha;
    sha.Update(reinterpret_cast<const u8*>("abc"), 3);
    REQUIRE(sha.Finalize() == OfText("abc"));
    REQUIRE(sha.Finalize() == Hash::CalcSha256(nullptr, 0));
    REQUIRE_FALSE(OfText("abc") == OfText("abd"));
}
