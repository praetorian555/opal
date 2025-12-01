#include "test-helpers.h"

#include "opal/thread.h"

using namespace Opal;

TEST_CASE("Create a thread", "[Thread]")
{
    int n = 5;
    ThreadHandle handle = CreateThread([](int nn)
    {
        REQUIRE(nn == 5);
        nn++;
    }, n);

    JoinThread(handle);
    REQUIRE(n == 5);

    ThreadHandle out_handle;
    handle = CreateThread([](int& nn, ThreadHandle& inner_handle)
    {
        REQUIRE(nn == 5);
        nn++;
        inner_handle = GetCurrentThreadHandle();
    }, Ref(n), Ref(out_handle));

    JoinThread(handle);
    REQUIRE(n == 6);
    REQUIRE(out_handle == handle);
    REQUIRE(GetCurrentThreadHandle() != out_handle);
}

