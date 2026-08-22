#include "opal/socket.h"

#include <limits.h>
#include <string.h>

#if defined(OPAL_PLATFORM_WINDOWS)
#include <WinSock2.h>
#include <WS2tcpip.h>
#elif defined(OPAL_PLATFORM_LINUX)
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace
{

#if defined(OPAL_PLATFORM_WINDOWS)
using OsSocket = SOCKET;
using OsSockLen = int;
constexpr OsSocket k_os_invalid_socket = INVALID_SOCKET;
#elif defined(OPAL_PLATFORM_LINUX)
using OsSocket = int;
using OsSockLen = socklen_t;
constexpr OsSocket k_os_invalid_socket = -1;
#endif

OsSocket ToOs(Opal::u64 handle)
{
    return static_cast<OsSocket>(handle);
}

Opal::ErrorCode MapLastError([[maybe_unused]] bool is_non_blocking)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    switch (WSAGetLastError())
    {
        case WSAEWOULDBLOCK:
            return Opal::ErrorCode::WouldBlock;
        case WSAETIMEDOUT:
            return Opal::ErrorCode::TimedOut;
        case WSAECONNREFUSED:
            return Opal::ErrorCode::ConnectionRefused;
        case WSAECONNRESET:
        case WSAECONNABORTED:
            return Opal::ErrorCode::ConnectionReset;
        case WSAEADDRINUSE:
            return Opal::ErrorCode::AddressInUse;
        default:
            return Opal::ErrorCode::OSFailure;
    }
#elif defined(OPAL_PLATFORM_LINUX)
    const int error = errno;
    // A blocking socket only sees EAGAIN - the same value as EWOULDBLOCK on Linux - when a timeout set through
    // SO_RCVTIMEO or SO_SNDTIMEO ran out.
    if (error == EAGAIN)
    {
        return is_non_blocking ? Opal::ErrorCode::WouldBlock : Opal::ErrorCode::TimedOut;
    }
    switch (error)
    {
        case EINPROGRESS:
            return Opal::ErrorCode::WouldBlock;
        case ETIMEDOUT:
            return Opal::ErrorCode::TimedOut;
        case ECONNREFUSED:
            return Opal::ErrorCode::ConnectionRefused;
        case ECONNRESET:
        case EPIPE:
            return Opal::ErrorCode::ConnectionReset;
        case EADDRINUSE:
            return Opal::ErrorCode::AddressInUse;
        default:
            return Opal::ErrorCode::OSFailure;
    }
#endif
}

OsSockLen ToSockaddr(const Opal::SocketEndpoint& endpoint, sockaddr_storage& out_storage)
{
    memset(&out_storage, 0, sizeof(out_storage));
    if (endpoint.address.family == Opal::IpFamily::IpV4)
    {
        auto* address = reinterpret_cast<sockaddr_in*>(&out_storage);
        address->sin_family = AF_INET;
        address->sin_port = htons(endpoint.port);
        memcpy(&address->sin_addr, endpoint.address.bytes, 4);
        return sizeof(sockaddr_in);
    }
    auto* address = reinterpret_cast<sockaddr_in6*>(&out_storage);
    address->sin6_family = AF_INET6;
    address->sin6_port = htons(endpoint.port);
    memcpy(&address->sin6_addr, endpoint.address.bytes, 16);
    return sizeof(sockaddr_in6);
}

void FromSockaddr(const sockaddr_storage& storage, Opal::SocketEndpoint& out_endpoint)
{
    if (storage.ss_family == AF_INET)
    {
        const auto* address = reinterpret_cast<const sockaddr_in*>(&storage);
        out_endpoint.address = Opal::IpAddress();
        out_endpoint.address.family = Opal::IpFamily::IpV4;
        memcpy(out_endpoint.address.bytes, &address->sin_addr, 4);
        out_endpoint.port = ntohs(address->sin_port);
        return;
    }
    if (storage.ss_family == AF_INET6)
    {
        const auto* address = reinterpret_cast<const sockaddr_in6*>(&storage);
        out_endpoint.address.family = Opal::IpFamily::IpV6;
        memcpy(out_endpoint.address.bytes, &address->sin6_addr, 16);
        out_endpoint.port = ntohs(address->sin6_port);
    }
}

}  // namespace

Opal::Expected<Opal::IpAddress, Opal::ErrorCode> Opal::IpAddress::Parse(const StringUtf8& text)
{
    using Result = Expected<IpAddress, ErrorCode>;
    IpAddress address;
    in_addr v4;
    if (inet_pton(AF_INET, *text, &v4) == 1)
    {
        address.family = IpFamily::IpV4;
        memcpy(address.bytes, &v4, 4);
        return Result(address);
    }
    in6_addr v6;
    if (inet_pton(AF_INET6, *text, &v6) == 1)
    {
        address.family = IpFamily::IpV6;
        memcpy(address.bytes, &v6, 16);
        return Result(address);
    }
    return Result(ErrorCode::InvalidArgument);
}

Opal::StringUtf8 Opal::IpAddress::ToString() const
{
    char8 buffer[INET6_ADDRSTRLEN] = {};
    const int os_family = family == IpFamily::IpV4 ? AF_INET : AF_INET6;
    if (inet_ntop(os_family, bytes, buffer, sizeof(buffer)) == nullptr)
    {
        return StringUtf8();
    }
    return StringUtf8(buffer);
}

Opal::IpAddress Opal::IpAddress::AnyV4()
{
    return IpAddress();
}

Opal::IpAddress Opal::IpAddress::LoopbackV4()
{
    IpAddress address;
    address.bytes[0] = 127;
    address.bytes[3] = 1;
    return address;
}

Opal::IpAddress Opal::IpAddress::AnyV6()
{
    IpAddress address;
    address.family = IpFamily::IpV6;
    return address;
}

Opal::IpAddress Opal::IpAddress::LoopbackV6()
{
    IpAddress address;
    address.family = IpFamily::IpV6;
    address.bytes[15] = 1;
    return address;
}

bool Opal::IpAddress::operator==(const IpAddress& other) const
{
    return family == other.family && memcmp(bytes, other.bytes, sizeof(bytes)) == 0;
}

bool Opal::SocketEndpoint::operator==(const SocketEndpoint& other) const
{
    return address == other.address && port == other.port;
}

Opal::ErrorCode Opal::InitializeNetworking()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0 ? ErrorCode::Success : ErrorCode::OSFailure;
#elif defined(OPAL_PLATFORM_LINUX)
    return ErrorCode::Success;
#endif
}

void Opal::ShutdownNetworking()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    WSACleanup();
#endif
}

Opal::Expected<Opal::Socket, Opal::ErrorCode> Opal::Socket::CreateTcp(IpFamily family)
{
    using Result = Expected<Socket, ErrorCode>;
    const int os_family = family == IpFamily::IpV4 ? AF_INET : AF_INET6;
    const OsSocket handle = socket(os_family, SOCK_STREAM, IPPROTO_TCP);
    if (handle == k_os_invalid_socket)
    {
        return Result(MapLastError(false));
    }
    return Result(Socket(static_cast<u64>(handle)));
}

Opal::Expected<Opal::Socket, Opal::ErrorCode> Opal::Socket::CreateUdp(IpFamily family)
{
    using Result = Expected<Socket, ErrorCode>;
    const int os_family = family == IpFamily::IpV4 ? AF_INET : AF_INET6;
    const OsSocket handle = socket(os_family, SOCK_DGRAM, IPPROTO_UDP);
    if (handle == k_os_invalid_socket)
    {
        return Result(MapLastError(false));
    }
    return Result(Socket(static_cast<u64>(handle)));
}

Opal::Socket::~Socket()
{
    Close();
}

Opal::Socket::Socket(Socket&& other) noexcept : m_handle(other.m_handle), m_is_non_blocking(other.m_is_non_blocking)
{
    other.m_handle = k_invalid_handle;
    other.m_is_non_blocking = false;
}

Opal::Socket& Opal::Socket::operator=(Socket&& other) noexcept
{
    if (this != &other)
    {
        Close();
        m_handle = other.m_handle;
        m_is_non_blocking = other.m_is_non_blocking;
        other.m_handle = k_invalid_handle;
        other.m_is_non_blocking = false;
    }
    return *this;
}

bool Opal::Socket::IsValid() const
{
    return m_handle != k_invalid_handle;
}

void Opal::Socket::Close()
{
    if (!IsValid())
    {
        return;
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    closesocket(ToOs(m_handle));
#elif defined(OPAL_PLATFORM_LINUX)
    close(ToOs(m_handle));
#endif
    m_handle = k_invalid_handle;
    m_is_non_blocking = false;
}

Opal::ErrorCode Opal::Socket::Bind(const SocketEndpoint& endpoint)
{
    if (!IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
    sockaddr_storage storage;
    const OsSockLen length = ToSockaddr(endpoint, storage);
    if (bind(ToOs(m_handle), reinterpret_cast<const sockaddr*>(&storage), length) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
    return ErrorCode::Success;
}

Opal::ErrorCode Opal::Socket::Listen(i32 backlog)
{
    if (!IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
    if (listen(ToOs(m_handle), backlog) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
    return ErrorCode::Success;
}

Opal::Expected<Opal::Socket, Opal::ErrorCode> Opal::Socket::Accept(SocketEndpoint* out_remote)
{
    using Result = Expected<Socket, ErrorCode>;
    if (!IsValid())
    {
        return Result(ErrorCode::InvalidArgument);
    }
    sockaddr_storage storage = {};
    OsSockLen length = sizeof(storage);
    const OsSocket accepted = accept(ToOs(m_handle), reinterpret_cast<sockaddr*>(&storage), &length);
    if (accepted == k_os_invalid_socket)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
    if (out_remote != nullptr)
    {
        FromSockaddr(storage, *out_remote);
    }
    return Result(Socket(static_cast<u64>(accepted)));
}

Opal::ErrorCode Opal::Socket::Connect(const SocketEndpoint& endpoint)
{
    if (!IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
    sockaddr_storage storage;
    const OsSockLen length = ToSockaddr(endpoint, storage);
    if (connect(ToOs(m_handle), reinterpret_cast<const sockaddr*>(&storage), length) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
    return ErrorCode::Success;
}

Opal::Expected<Opal::u64, Opal::ErrorCode> Opal::Socket::Send(ArrayView<const u8> data)
{
    using Result = Expected<u64, ErrorCode>;
    if (!IsValid())
    {
        return Result(ErrorCode::InvalidArgument);
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    const int count = data.GetSize() > INT_MAX ? INT_MAX : static_cast<int>(data.GetSize());
    const int sent = send(ToOs(m_handle), reinterpret_cast<const char*>(data.GetData()), count, 0);
    if (sent == SOCKET_ERROR)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
#elif defined(OPAL_PLATFORM_LINUX)
    const ssize_t sent = send(ToOs(m_handle), data.GetData(), data.GetSize(), MSG_NOSIGNAL);
    if (sent < 0)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
#endif
    return Result(static_cast<u64>(sent));
}

Opal::Expected<Opal::u64, Opal::ErrorCode> Opal::Socket::Receive(ArrayView<u8> buffer)
{
    using Result = Expected<u64, ErrorCode>;
    if (!IsValid() || buffer.GetSize() == 0)
    {
        return Result(ErrorCode::InvalidArgument);
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    const int count = buffer.GetSize() > INT_MAX ? INT_MAX : static_cast<int>(buffer.GetSize());
    const int received = recv(ToOs(m_handle), reinterpret_cast<char*>(buffer.GetData()), count, 0);
    if (received == SOCKET_ERROR)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
#elif defined(OPAL_PLATFORM_LINUX)
    const ssize_t received = recv(ToOs(m_handle), buffer.GetData(), buffer.GetSize(), 0);
    if (received < 0)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
#endif
    if (received == 0)
    {
        return Result(ErrorCode::ConnectionClosed);
    }
    return Result(static_cast<u64>(received));
}

Opal::Expected<Opal::u64, Opal::ErrorCode> Opal::Socket::SendTo(ArrayView<const u8> data, const SocketEndpoint& to)
{
    using Result = Expected<u64, ErrorCode>;
    if (!IsValid())
    {
        return Result(ErrorCode::InvalidArgument);
    }
    sockaddr_storage storage;
    const OsSockLen address_length = ToSockaddr(to, storage);
    const auto* address = reinterpret_cast<const sockaddr*>(&storage);
#if defined(OPAL_PLATFORM_WINDOWS)
    const int count = data.GetSize() > INT_MAX ? INT_MAX : static_cast<int>(data.GetSize());
    const int sent = sendto(ToOs(m_handle), reinterpret_cast<const char*>(data.GetData()), count, 0, address, address_length);
    if (sent == SOCKET_ERROR)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
#elif defined(OPAL_PLATFORM_LINUX)
    const ssize_t sent = sendto(ToOs(m_handle), data.GetData(), data.GetSize(), MSG_NOSIGNAL, address, address_length);
    if (sent < 0)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
#endif
    return Result(static_cast<u64>(sent));
}

Opal::Expected<Opal::u64, Opal::ErrorCode> Opal::Socket::ReceiveFrom(ArrayView<u8> buffer, SocketEndpoint* out_from)
{
    using Result = Expected<u64, ErrorCode>;
    if (!IsValid())
    {
        return Result(ErrorCode::InvalidArgument);
    }
    sockaddr_storage storage = {};
    OsSockLen address_length = sizeof(storage);
    auto* address = reinterpret_cast<sockaddr*>(&storage);
#if defined(OPAL_PLATFORM_WINDOWS)
    const int count = buffer.GetSize() > INT_MAX ? INT_MAX : static_cast<int>(buffer.GetSize());
    const int received = recvfrom(ToOs(m_handle), reinterpret_cast<char*>(buffer.GetData()), count, 0, address, &address_length);
    if (received == SOCKET_ERROR)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
#elif defined(OPAL_PLATFORM_LINUX)
    const ssize_t received = recvfrom(ToOs(m_handle), buffer.GetData(), buffer.GetSize(), 0, address, &address_length);
    if (received < 0)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
#endif
    if (out_from != nullptr)
    {
        FromSockaddr(storage, *out_from);
    }
    return Result(static_cast<u64>(received));
}

Opal::ErrorCode Opal::Socket::SetNonBlocking(bool enable)
{
    if (!IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    u_long mode = enable ? 1 : 0;
    if (ioctlsocket(ToOs(m_handle), FIONBIO, &mode) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
#elif defined(OPAL_PLATFORM_LINUX)
    const int flags = fcntl(ToOs(m_handle), F_GETFL, 0);
    if (flags < 0)
    {
        return MapLastError(m_is_non_blocking);
    }
    const int new_flags = enable ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    if (fcntl(ToOs(m_handle), F_SETFL, new_flags) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
#endif
    m_is_non_blocking = enable;
    return ErrorCode::Success;
}

Opal::ErrorCode Opal::Socket::SetReuseAddress(bool enable)
{
    if (!IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
    const int value = enable ? 1 : 0;
    if (setsockopt(ToOs(m_handle), SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&value), sizeof(value)) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
    return ErrorCode::Success;
}

Opal::ErrorCode Opal::Socket::SetNoDelay(bool enable)
{
    if (!IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
    const int value = enable ? 1 : 0;
    if (setsockopt(ToOs(m_handle), IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&value), sizeof(value)) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
    return ErrorCode::Success;
}

Opal::ErrorCode Opal::Socket::SetReceiveTimeoutMs(u32 milliseconds)
{
    if (!IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    const DWORD value = milliseconds;
    if (setsockopt(ToOs(m_handle), SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&value), sizeof(value)) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
#elif defined(OPAL_PLATFORM_LINUX)
    timeval value;
    value.tv_sec = static_cast<time_t>(milliseconds / 1000);
    value.tv_usec = static_cast<suseconds_t>((milliseconds % 1000) * 1000);
    if (setsockopt(ToOs(m_handle), SOL_SOCKET, SO_RCVTIMEO, &value, sizeof(value)) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
#endif
    return ErrorCode::Success;
}

Opal::ErrorCode Opal::Socket::SetSendTimeoutMs(u32 milliseconds)
{
    if (!IsValid())
    {
        return ErrorCode::InvalidArgument;
    }
#if defined(OPAL_PLATFORM_WINDOWS)
    const DWORD value = milliseconds;
    if (setsockopt(ToOs(m_handle), SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&value), sizeof(value)) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
#elif defined(OPAL_PLATFORM_LINUX)
    timeval value;
    value.tv_sec = static_cast<time_t>(milliseconds / 1000);
    value.tv_usec = static_cast<suseconds_t>((milliseconds % 1000) * 1000);
    if (setsockopt(ToOs(m_handle), SOL_SOCKET, SO_SNDTIMEO, &value, sizeof(value)) != 0)
    {
        return MapLastError(m_is_non_blocking);
    }
#endif
    return ErrorCode::Success;
}

Opal::Expected<Opal::SocketEndpoint, Opal::ErrorCode> Opal::Socket::GetLocalEndpoint() const
{
    using Result = Expected<SocketEndpoint, ErrorCode>;
    if (!IsValid())
    {
        return Result(ErrorCode::InvalidArgument);
    }
    sockaddr_storage storage = {};
    OsSockLen length = sizeof(storage);
    if (getsockname(ToOs(m_handle), reinterpret_cast<sockaddr*>(&storage), &length) != 0)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
    SocketEndpoint endpoint;
    FromSockaddr(storage, endpoint);
    return Result(endpoint);
}

Opal::Expected<Opal::SocketEndpoint, Opal::ErrorCode> Opal::Socket::GetRemoteEndpoint() const
{
    using Result = Expected<SocketEndpoint, ErrorCode>;
    if (!IsValid())
    {
        return Result(ErrorCode::InvalidArgument);
    }
    sockaddr_storage storage = {};
    OsSockLen length = sizeof(storage);
    if (getpeername(ToOs(m_handle), reinterpret_cast<sockaddr*>(&storage), &length) != 0)
    {
        return Result(MapLastError(m_is_non_blocking));
    }
    SocketEndpoint endpoint;
    FromSockaddr(storage, endpoint);
    return Result(endpoint);
}
