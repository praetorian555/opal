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

    handle = CreateThread([](int& nn)
    {
        REQUIRE(nn == 5);
        nn++;
    }, Ref<int>(n));

    JoinThread(handle);
    REQUIRE(n == 6);
}

