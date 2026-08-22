#pragma once

#include "opal/container/array-view.h"
#include "opal/container/expected.h"
#include "opal/container/string.h"
#include "opal/error-codes.h"
#include "opal/types.h"

namespace Opal
{

enum class IpFamily : u8
{
    IpV4,
    IpV6
};

/**
 * @brief A numeric IP address of either family.
 */
struct IpAddress
{
    IpFamily family = IpFamily::IpV4;
    /** Network byte order. An IPv4 address uses the first four bytes and leaves the rest zero. */
    u8 bytes[16] = {};

    /**
     * @brief Parse a numeric address, either dotted IPv4 ("127.0.0.1") or IPv6 ("::1").
     * @param text The address in its string form. Host names are not resolved.
     * @return The parsed address, or ErrorCode::InvalidArgument when the text is not a numeric address.
     */
    [[nodiscard]] static Expected<IpAddress, ErrorCode> Parse(const StringUtf8& text);

    /**
     * @brief Format the address in its canonical string form.
     * @return The address as text, or an empty string when the OS refuses to format it.
     */
    [[nodiscard]] StringUtf8 ToString() const;

    /** @brief The IPv4 wildcard address 0.0.0.0, which binds to every local interface. */
    [[nodiscard]] static IpAddress AnyV4();
    /** @brief The IPv4 loopback address 127.0.0.1. */
    [[nodiscard]] static IpAddress LoopbackV4();
    /** @brief The IPv6 wildcard address ::. */
    [[nodiscard]] static IpAddress AnyV6();
    /** @brief The IPv6 loopback address ::1. */
    [[nodiscard]] static IpAddress LoopbackV6();

    bool operator==(const IpAddress& other) const;
};

/**
 * @brief An address and port pair naming one end of a connection.
 */
struct SocketEndpoint
{
    IpAddress address;
    /** Host byte order; the conversion to network order is internal. */
    u16 port = 0;

    bool operator==(const SocketEndpoint& other) const;
};

/**
 * @brief Prepare the OS networking stack for use. Every other function in this header requires a successful call
 * first. Calls may be repeated and nest; pair each successful one with a ShutdownNetworking. No-op on Linux.
 * @return ErrorCode::Success, or ErrorCode::OSFailure when the stack refuses to start.
 */
[[nodiscard]] ErrorCode InitializeNetworking();

/**
 * @brief Release the OS networking stack. Call once per successful InitializeNetworking, after the last socket is
 * closed. No-op on Linux.
 */
void ShutdownNetworking();

/**
 * @brief A TCP or UDP socket. Move-only; the destructor closes it. Sockets block by default and can be switched to
 * non-blocking, where an operation that would have to wait returns ErrorCode::WouldBlock instead.
 *
 * Methods called on a socket that is not valid return ErrorCode::InvalidArgument.
 */
class Socket
{
public:
    /**
     * @brief Create a TCP stream socket.
     * @param family The IP family the socket speaks.
     * @return The socket, or ErrorCode::OSFailure when the OS refuses to create it.
     */
    [[nodiscard]] static Expected<Socket, ErrorCode> CreateTcp(IpFamily family);

    /**
     * @brief Create a UDP datagram socket.
     * @param family The IP family the socket speaks.
     * @return The socket, or ErrorCode::OSFailure when the OS refuses to create it.
     */
    [[nodiscard]] static Expected<Socket, ErrorCode> CreateUdp(IpFamily family);

    /** @brief Constructs a socket that is not valid. */
    Socket() = default;
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    /** @brief True when the socket holds an open OS socket. */
    [[nodiscard]] bool IsValid() const;

    /** @brief Close the socket. Safe to call on a socket that is not valid. */
    void Close();

    /**
     * @brief Bind the socket to a local address and port. A port of zero picks a free ephemeral port, which
     * GetLocalEndpoint reads back.
     * @param endpoint The local endpoint to bind. Its family has to match the socket's.
     * @return ErrorCode::Success, ErrorCode::AddressInUse when the endpoint is already bound, or
     *         ErrorCode::OSFailure for any other failure.
     */
    [[nodiscard]] ErrorCode Bind(const SocketEndpoint& endpoint);

    /**
     * @brief Start listening for incoming TCP connections on a bound socket.
     * @param backlog How many pending connections the OS queues before refusing new ones.
     * @return ErrorCode::Success, or ErrorCode::OSFailure when the OS refuses.
     */
    [[nodiscard]] ErrorCode Listen(i32 backlog);

    /**
     * @brief Take one pending connection off a listening socket, waiting for one when the socket blocks.
     * @param out_remote When not null, receives the connecting peer's endpoint.
     * @return The connected socket, ErrorCode::WouldBlock on a non-blocking socket with nothing pending,
     *         ErrorCode::TimedOut when a receive timeout is set and expires, or ErrorCode::OSFailure for any other
     *         failure.
     */
    [[nodiscard]] Expected<Socket, ErrorCode> Accept(SocketEndpoint* out_remote = nullptr);

    /**
     * @brief Connect a TCP socket to a remote endpoint, or set the default destination of a UDP one.
     * @param endpoint The remote endpoint. Its family has to match the socket's.
     * @return ErrorCode::Success, ErrorCode::ConnectionRefused when nothing listens there,
     *         ErrorCode::TimedOut when the attempt expires, ErrorCode::WouldBlock on a non-blocking TCP socket -
     *         detecting when such a connect finishes is out of scope, prefer a blocking connect with timeouts - or
     *         ErrorCode::OSFailure for any other failure.
     */
    [[nodiscard]] ErrorCode Connect(const SocketEndpoint& endpoint);

    /**
     * @brief Send bytes on a connected socket.
     * @param data The bytes to send. May send fewer; the count says how many.
     * @return The number of bytes accepted, ErrorCode::WouldBlock on a non-blocking socket with no buffer space,
     *         ErrorCode::TimedOut when a send timeout expires, ErrorCode::ConnectionReset when the peer is gone, or
     *         ErrorCode::OSFailure for any other failure.
     */
    [[nodiscard]] Expected<u64, ErrorCode> Send(ArrayView<const u8> data);

    /**
     * @brief Receive bytes on a connected socket, waiting for some when the socket blocks.
     * @param buffer Where the bytes land. The count says how many arrived and is never zero.
     * @return The number of bytes received, ErrorCode::ConnectionClosed when the peer closed in an orderly way,
     *         ErrorCode::WouldBlock on a non-blocking socket with nothing pending, ErrorCode::TimedOut when a
     *         receive timeout expires, ErrorCode::ConnectionReset when the connection died, or
     *         ErrorCode::OSFailure for any other failure.
     */
    [[nodiscard]] Expected<u64, ErrorCode> Receive(ArrayView<u8> buffer);

    /**
     * @brief Send one datagram to an endpoint.
     * @param data The datagram's bytes. An empty view sends an empty datagram.
     * @param to Where it goes. The family has to match the socket's.
     * @return The number of bytes sent, or the same errors as Send.
     */
    [[nodiscard]] Expected<u64, ErrorCode> SendTo(ArrayView<const u8> data, const SocketEndpoint& to);

    /**
     * @brief Receive one datagram, waiting for one when the socket blocks. A datagram longer than the buffer is cut
     * to fit and the rest of it is gone.
     * @param buffer Where the datagram lands.
     * @param out_from When not null, receives the sender's endpoint.
     * @return The datagram's size, which for an empty datagram is zero, ErrorCode::WouldBlock on a non-blocking
     *         socket with nothing pending, ErrorCode::TimedOut when a receive timeout expires, or
     *         ErrorCode::OSFailure for any other failure.
     */
    [[nodiscard]] Expected<u64, ErrorCode> ReceiveFrom(ArrayView<u8> buffer, SocketEndpoint* out_from = nullptr);

    /**
     * @brief Switch the socket between blocking and non-blocking operation.
     * @param enable True for non-blocking.
     * @return ErrorCode::Success, or ErrorCode::OSFailure when the OS refuses.
     */
    [[nodiscard]] ErrorCode SetNonBlocking(bool enable);

    /**
     * @brief Allow binding an address that is in the process of being released.
     * @param enable True to allow.
     * @return ErrorCode::Success, or ErrorCode::OSFailure when the OS refuses.
     */
    [[nodiscard]] ErrorCode SetReuseAddress(bool enable);

    /**
     * @brief Turn off Nagle's algorithm on a TCP socket so small sends leave immediately.
     * @param enable True to send without coalescing.
     * @return ErrorCode::Success, or ErrorCode::OSFailure when the OS refuses.
     */
    [[nodiscard]] ErrorCode SetNoDelay(bool enable);

    /**
     * @brief Bound how long a blocking receive waits before giving up with ErrorCode::TimedOut.
     * @param milliseconds The wait limit. Zero restores waiting forever.
     * @return ErrorCode::Success, or ErrorCode::OSFailure when the OS refuses.
     */
    [[nodiscard]] ErrorCode SetReceiveTimeoutMs(u32 milliseconds);

    /**
     * @brief Bound how long a blocking send waits before giving up with ErrorCode::TimedOut.
     * @param milliseconds The wait limit. Zero restores waiting forever.
     * @return ErrorCode::Success, or ErrorCode::OSFailure when the OS refuses.
     */
    [[nodiscard]] ErrorCode SetSendTimeoutMs(u32 milliseconds);

    /**
     * @brief Read the endpoint the socket is bound to, ephemeral port included.
     * @return The local endpoint, or ErrorCode::OSFailure when the socket is not bound.
     */
    [[nodiscard]] Expected<SocketEndpoint, ErrorCode> GetLocalEndpoint() const;

    /**
     * @brief Read the endpoint of the connected peer.
     * @return The remote endpoint, or ErrorCode::OSFailure when the socket is not connected.
     */
    [[nodiscard]] Expected<SocketEndpoint, ErrorCode> GetRemoteEndpoint() const;

private:
    static constexpr u64 k_invalid_handle = ~static_cast<u64>(0);

    explicit Socket(u64 handle) : m_handle(handle) {}

    u64 m_handle = k_invalid_handle;
    bool m_is_non_blocking = false;
};

}  // namespace Opal
