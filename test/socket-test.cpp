#include "opal/socket.h"

#include <string.h>

#include "opal/threading/thread.h"

#include "test-helpers.h"

using namespace Opal;

namespace
{

/** Brackets a test case with the networking startup every socket function requires. */
struct NetworkingGuard
{
    NetworkingGuard() { REQUIRE(InitializeNetworking() == ErrorCode::Success); }
    ~NetworkingGuard() { ShutdownNetworking(); }
};

/** Unwrap the success case so the tests that are not about failure stay about what the socket does. */
Socket CreateOrFail(Expected<Socket, ErrorCode> result)
{
    REQUIRE(result.HasValue());
    return Move(result).GetValue();
}

SocketEndpoint LocalEndpointOrFail(const Socket& socket)
{
    Expected<SocketEndpoint, ErrorCode> result = socket.GetLocalEndpoint();
    REQUIRE(result.HasValue());
    return result.GetValue();
}

/** TCP hands back a stream, so a logical message can arrive in pieces; keep reading until it is whole. */
bool ReceiveAll(Socket& socket, u8* buffer, u64 size)
{
    u64 total = 0;
    while (total < size)
    {
        Expected<u64, ErrorCode> received = socket.Receive(ArrayView<u8>(buffer + total, size - total));
        if (!received.HasValue())
        {
            return false;
        }
        total += received.GetValue();
    }
    return true;
}

constexpr u32 k_test_timeout_ms = 5000;

}  // namespace

TEST_CASE("Socket address parsing", "[Socket]")
{
    NetworkingGuard guard;

    SECTION("IPv4 round trip")
    {
        Expected<IpAddress, ErrorCode> parsed = IpAddress::Parse("192.168.1.42");
        REQUIRE(parsed.HasValue());
        const IpAddress address = parsed.GetValue();
        CHECK(address.family == IpFamily::IpV4);
        CHECK(address.bytes[0] == 192);
        CHECK(address.bytes[1] == 168);
        CHECK(address.bytes[2] == 1);
        CHECK(address.bytes[3] == 42);
        CHECK(address.ToString() == StringUtf8("192.168.1.42"));
    }
    SECTION("IPv6 round trip")
    {
        Expected<IpAddress, ErrorCode> parsed = IpAddress::Parse("2001:db8::ff00:42:8329");
        REQUIRE(parsed.HasValue());
        const IpAddress address = parsed.GetValue();
        CHECK(address.family == IpFamily::IpV6);
        CHECK(address.ToString() == StringUtf8("2001:db8::ff00:42:8329"));
    }
    SECTION("Garbage is rejected")
    {
        CHECK(IpAddress::Parse("not an address").GetError() == ErrorCode::InvalidArgument);
        CHECK(IpAddress::Parse("256.0.0.1").GetError() == ErrorCode::InvalidArgument);
        CHECK(IpAddress::Parse("1.2.3").GetError() == ErrorCode::InvalidArgument);
        CHECK(IpAddress::Parse("").GetError() == ErrorCode::InvalidArgument);
    }
    SECTION("Well known addresses")
    {
        CHECK(IpAddress::AnyV4().ToString() == StringUtf8("0.0.0.0"));
        CHECK(IpAddress::LoopbackV4().ToString() == StringUtf8("127.0.0.1"));
        CHECK(IpAddress::AnyV6().ToString() == StringUtf8("::"));
        CHECK(IpAddress::LoopbackV6().ToString() == StringUtf8("::1"));
    }
    SECTION("Equality")
    {
        CHECK(IpAddress::LoopbackV4() == IpAddress::LoopbackV4());
        CHECK_FALSE(IpAddress::LoopbackV4() == IpAddress::AnyV4());
        CHECK_FALSE(IpAddress::LoopbackV4() == IpAddress::LoopbackV6());
        const SocketEndpoint a{IpAddress::LoopbackV4(), 80};
        const SocketEndpoint b{IpAddress::LoopbackV4(), 81};
        CHECK(a == a);
        CHECK_FALSE(a == b);
    }
}

TEST_CASE("Socket validity and moves", "[Socket]")
{
    NetworkingGuard guard;

    SECTION("A default socket is not valid and every call reports it")
    {
        Socket invalid;
        CHECK_FALSE(invalid.IsValid());
        CHECK(invalid.Bind({IpAddress::LoopbackV4(), 0}) == ErrorCode::InvalidArgument);
        CHECK(invalid.Listen(1) == ErrorCode::InvalidArgument);
        CHECK(invalid.Connect({IpAddress::LoopbackV4(), 80}) == ErrorCode::InvalidArgument);
        CHECK(invalid.Accept().GetError() == ErrorCode::InvalidArgument);
        u8 buffer[4] = {};
        CHECK(invalid.Send(ArrayView<const u8>(buffer, 4)).GetError() == ErrorCode::InvalidArgument);
        CHECK(invalid.Receive(ArrayView<u8>(buffer, 4)).GetError() == ErrorCode::InvalidArgument);
        CHECK(invalid.SetNonBlocking(true) == ErrorCode::InvalidArgument);
        CHECK(invalid.GetLocalEndpoint().GetError() == ErrorCode::InvalidArgument);
        invalid.Close();
        CHECK_FALSE(invalid.IsValid());
    }
    SECTION("Moving passes the socket along")
    {
        Socket original = CreateOrFail(Socket::CreateUdp(IpFamily::IpV4));
        REQUIRE(original.IsValid());

        Socket moved(Move(original));
        CHECK(moved.IsValid());
        CHECK_FALSE(original.IsValid());

        Socket assigned;
        assigned = Move(moved);
        CHECK(assigned.IsValid());
        CHECK_FALSE(moved.IsValid());
    }
}

TEST_CASE("Socket TCP over loopback", "[Socket]")
{
    NetworkingGuard guard;

    Socket listener = CreateOrFail(Socket::CreateTcp(IpFamily::IpV4));
    REQUIRE(listener.Bind({IpAddress::LoopbackV4(), 0}) == ErrorCode::Success);
    REQUIRE(listener.Listen(1) == ErrorCode::Success);
    const SocketEndpoint server_endpoint = LocalEndpointOrFail(listener);
    REQUIRE(server_endpoint.port != 0);
    CHECK(server_endpoint.address == IpAddress::LoopbackV4());

    // Catch assertions are not thread-safe, so the client thread only records what happened and the
    // checks run after the join.
    struct ClientOutcome
    {
        bool connected = false;
        bool sent = false;
        bool got_reply = false;
    };
    ClientOutcome outcome;
    Expected<ThreadHandle, ErrorCode> thread = CreateThread(
        [&outcome, server_endpoint]()
        {
            Expected<Socket, ErrorCode> client_result = Socket::CreateTcp(IpFamily::IpV4);
            if (!client_result.HasValue())
            {
                return;
            }
            Socket client = Move(client_result).GetValue();
            if (client.SetReceiveTimeoutMs(k_test_timeout_ms) != ErrorCode::Success ||
                client.Connect(server_endpoint) != ErrorCode::Success)
            {
                return;
            }
            outcome.connected = true;
            const u8 ping[4] = {'p', 'i', 'n', 'g'};
            Expected<u64, ErrorCode> sent = client.Send(ArrayView<const u8>(ping, 4));
            if (!sent.HasValue() || sent.GetValue() != 4)
            {
                return;
            }
            outcome.sent = true;
            u8 reply[4] = {};
            if (ReceiveAll(client, reply, 4))
            {
                outcome.got_reply = memcmp(reply, "pong", 4) == 0;
            }
        });
    REQUIRE(thread.HasValue());

    // The server half runs on this thread and records instead of asserting, so a failure still joins
    // the client before the test case unwinds.
    SocketEndpoint remote;
    Expected<Socket, ErrorCode> accepted = listener.Accept(&remote);
    bool got_ping = false;
    bool sent_pong = false;
    bool saw_close = false;
    ErrorCode close_code = ErrorCode::Success;
    if (accepted.HasValue())
    {
        Socket server = Move(accepted).GetValue();
        (void)server.SetReceiveTimeoutMs(k_test_timeout_ms);
        u8 request[4] = {};
        got_ping = ReceiveAll(server, request, 4) && memcmp(request, "ping", 4) == 0;
        const u8 pong[4] = {'p', 'o', 'n', 'g'};
        Expected<u64, ErrorCode> sent = server.Send(ArrayView<const u8>(pong, 4));
        sent_pong = sent.HasValue() && sent.GetValue() == 4;

        // The client closes after it reads the reply, which lands here as an orderly close.
        u8 leftover[1] = {};
        Expected<u64, ErrorCode> closed = server.Receive(ArrayView<u8>(leftover, 1));
        saw_close = !closed.HasValue();
        close_code = saw_close ? closed.GetError() : ErrorCode::Success;
    }
    JoinThread(thread.GetValue());

    REQUIRE(accepted.HasValue());
    CHECK(remote.address == IpAddress::LoopbackV4());
    CHECK(outcome.connected);
    CHECK(outcome.sent);
    CHECK(got_ping);
    CHECK(sent_pong);
    CHECK(outcome.got_reply);
    CHECK(saw_close);
    CHECK(close_code == ErrorCode::ConnectionClosed);
}

TEST_CASE("Socket UDP over loopback", "[Socket]")
{
    NetworkingGuard guard;

    Socket sender = CreateOrFail(Socket::CreateUdp(IpFamily::IpV4));
    Socket receiver = CreateOrFail(Socket::CreateUdp(IpFamily::IpV4));
    REQUIRE(sender.Bind({IpAddress::LoopbackV4(), 0}) == ErrorCode::Success);
    REQUIRE(receiver.Bind({IpAddress::LoopbackV4(), 0}) == ErrorCode::Success);
    REQUIRE(receiver.SetReceiveTimeoutMs(k_test_timeout_ms) == ErrorCode::Success);
    const SocketEndpoint sender_endpoint = LocalEndpointOrFail(sender);
    const SocketEndpoint receiver_endpoint = LocalEndpointOrFail(receiver);

    SECTION("A datagram arrives whole and names its sender")
    {
        const u8 data[5] = {'h', 'e', 'l', 'l', 'o'};
        Expected<u64, ErrorCode> sent = sender.SendTo(ArrayView<const u8>(data, 5), receiver_endpoint);
        REQUIRE(sent.HasValue());
        CHECK(sent.GetValue() == 5);

        u8 buffer[16] = {};
        SocketEndpoint from;
        Expected<u64, ErrorCode> received = receiver.ReceiveFrom(ArrayView<u8>(buffer, 16), &from);
        REQUIRE(received.HasValue());
        CHECK(received.GetValue() == 5);
        CHECK(memcmp(buffer, data, 5) == 0);
        CHECK(from == sender_endpoint);
    }
    SECTION("An empty datagram arrives as a zero count")
    {
        Expected<u64, ErrorCode> sent = sender.SendTo(ArrayView<const u8>(), receiver_endpoint);
        REQUIRE(sent.HasValue());
        CHECK(sent.GetValue() == 0);

        u8 buffer[4] = {};
        Expected<u64, ErrorCode> received = receiver.ReceiveFrom(ArrayView<u8>(buffer, 4));
        REQUIRE(received.HasValue());
        CHECK(received.GetValue() == 0);
    }
}

TEST_CASE("Socket error reporting", "[Socket]")
{
    NetworkingGuard guard;

    SECTION("A non-blocking receive with nothing pending reports WouldBlock")
    {
        Socket socket = CreateOrFail(Socket::CreateUdp(IpFamily::IpV4));
        REQUIRE(socket.Bind({IpAddress::LoopbackV4(), 0}) == ErrorCode::Success);
        REQUIRE(socket.SetNonBlocking(true) == ErrorCode::Success);
        u8 buffer[4] = {};
        Expected<u64, ErrorCode> received = socket.ReceiveFrom(ArrayView<u8>(buffer, 4));
        REQUIRE_FALSE(received.HasValue());
        CHECK(received.GetError() == ErrorCode::WouldBlock);
    }
    SECTION("A receive timeout expires as TimedOut")
    {
        Socket socket = CreateOrFail(Socket::CreateUdp(IpFamily::IpV4));
        REQUIRE(socket.Bind({IpAddress::LoopbackV4(), 0}) == ErrorCode::Success);
        REQUIRE(socket.SetReceiveTimeoutMs(50) == ErrorCode::Success);
        u8 buffer[4] = {};
        Expected<u64, ErrorCode> received = socket.ReceiveFrom(ArrayView<u8>(buffer, 4));
        REQUIRE_FALSE(received.HasValue());
        CHECK(received.GetError() == ErrorCode::TimedOut);
    }
    SECTION("Connecting where nothing listens reports ConnectionRefused")
    {
        // Grab an ephemeral port and let it go again, so the connect has a port with nothing on it.
        u16 free_port = 0;
        {
            Socket placeholder = CreateOrFail(Socket::CreateTcp(IpFamily::IpV4));
            REQUIRE(placeholder.Bind({IpAddress::LoopbackV4(), 0}) == ErrorCode::Success);
            free_port = LocalEndpointOrFail(placeholder).port;
        }
        Socket client = CreateOrFail(Socket::CreateTcp(IpFamily::IpV4));
        CHECK(client.Connect({IpAddress::LoopbackV4(), free_port}) == ErrorCode::ConnectionRefused);
    }
    SECTION("Binding a taken port reports AddressInUse")
    {
        Socket first = CreateOrFail(Socket::CreateTcp(IpFamily::IpV4));
        REQUIRE(first.Bind({IpAddress::LoopbackV4(), 0}) == ErrorCode::Success);
        const u16 taken_port = LocalEndpointOrFail(first).port;

        Socket second = CreateOrFail(Socket::CreateTcp(IpFamily::IpV4));
        CHECK(second.Bind({IpAddress::LoopbackV4(), taken_port}) == ErrorCode::AddressInUse);
    }
}

TEST_CASE("Socket IPv6 over loopback", "[Socket]")
{
    NetworkingGuard guard;

    // A runner does not have to offer IPv6, so its absence is a skip rather than a failure.
    Expected<Socket, ErrorCode> sender_result = Socket::CreateUdp(IpFamily::IpV6);
    if (!sender_result.HasValue())
    {
        SUCCEED("IPv6 sockets are not available, skipping");
        return;
    }
    Socket sender = Move(sender_result).GetValue();
    Socket receiver = CreateOrFail(Socket::CreateUdp(IpFamily::IpV6));
    if (sender.Bind({IpAddress::LoopbackV6(), 0}) != ErrorCode::Success ||
        receiver.Bind({IpAddress::LoopbackV6(), 0}) != ErrorCode::Success)
    {
        SUCCEED("The IPv6 loopback is not available, skipping");
        return;
    }
    REQUIRE(receiver.SetReceiveTimeoutMs(k_test_timeout_ms) == ErrorCode::Success);
    const SocketEndpoint sender_endpoint = LocalEndpointOrFail(sender);
    const SocketEndpoint receiver_endpoint = LocalEndpointOrFail(receiver);
    CHECK(sender_endpoint.address == IpAddress::LoopbackV6());

    const u8 data[3] = {'v', '6', '!'};
    Expected<u64, ErrorCode> sent = sender.SendTo(ArrayView<const u8>(data, 3), receiver_endpoint);
    REQUIRE(sent.HasValue());
    CHECK(sent.GetValue() == 3);

    u8 buffer[8] = {};
    SocketEndpoint from;
    Expected<u64, ErrorCode> received = receiver.ReceiveFrom(ArrayView<u8>(buffer, 8), &from);
    REQUIRE(received.HasValue());
    CHECK(received.GetValue() == 3);
    CHECK(memcmp(buffer, data, 3) == 0);
    CHECK(from == sender_endpoint);
}
