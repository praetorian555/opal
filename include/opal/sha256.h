#pragma once

#include "opal/container/array-view.h"
#include "opal/container/expected.h"
#include "opal/container/string.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

/** A SHA-256 digest: 32 bytes, in the order the standard writes them. */
struct Sha256Digest
{
    u8 bytes[32] = {};

    bool operator==(const Sha256Digest&) const = default;
};

/**
 * @brief Computes a SHA-256 digest (FIPS 180-4) of a message that arrives in pieces, such as a file read in chunks.
 *
 * A cryptographic hash, for content identity and integrity checks such as a download against its published digest. For hashing
 * keys in containers use Hasher, which is far faster.
 */
class Sha256
{
public:
    /** Starts an empty message. */
    Sha256() noexcept;

    /**
     * Appends bytes to the message.
     * @param data Bytes to append. May be null when @p size is 0.
     * @param size Number of bytes.
     */
    void Update(const u8* data, u64 size);

    /**
     * Appends bytes to the message.
     * @param data Bytes to append.
     */
    void Update(ArrayView<const u8> data);

    /**
     * Finishes the message.
     * @return The digest of every byte appended. The object then holds a new, empty message.
     */
    Sha256Digest Finalize();

private:
    u32 m_state[8];
    /** Bytes of the message not yet compressed, fewer than a block. */
    u8 m_block[64];
    u64 m_block_size = 0;
    u64 m_message_size = 0;
};

namespace Hash
{
/**
 * Calculates the SHA-256 digest of an array of bytes.
 * @param data Bytes to hash. May be null when @p size is 0.
 * @param size Number of bytes.
 * @return The digest.
 */
Sha256Digest CalcSha256(const u8* data, u64 size);
}  // namespace Hash

/**
 * Formats a digest the way checksum files and package manifests write it.
 * @param digest Digest to format.
 * @param allocator Allocator for the string. Default allocator is used if nullptr.
 * @return 64 lowercase hexadecimal digits, or ErrorCode::OutOfMemory.
 */
Expected<StringUtf8, ErrorCode> ToHexString(const Sha256Digest& digest, AllocatorBase* allocator = nullptr);

}  // namespace Opal
