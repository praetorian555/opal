#include "test-helpers.h"

#include "opal/assert.h"

using namespace Opal;

namespace
{
bool g_handler_called = false;
const char* g_handler_message = nullptr;

// Not const, so the comparison below is not a constant expression. MSVC reports one as C4127, which this build treats as an error.
u64 g_size = 4;

// Long-jumping out of a contract handler is not something the library promises. This one records and returns, which
// HandleContractViolation is documented to treat as a handler that did not do its job.
void RecordViolation(const char* message)
{
    g_handler_called = true;
    g_handler_message = message;
}

struct ScopedContractHandler
{
    explicit ScopedContractHandler(ContractViolationHandler handler) { SetContractViolationHandler(handler); }
    ~ScopedContractHandler() { SetContractViolationHandler(nullptr); }
};
}  // namespace

TEST_CASE("Contract violation handler", "[Assert]")
{
    SECTION("Defaults to a handler that is never null")
    {
        REQUIRE(GetContractViolationHandler() != nullptr);
    }
    SECTION("Can be installed and restored")
    {
        ContractViolationHandler original = GetContractViolationHandler();
        {
            const ScopedContractHandler scoped(&RecordViolation);
            REQUIRE(GetContractViolationHandler() == &RecordViolation);
        }
        REQUIRE(GetContractViolationHandler() == original);
    }
    SECTION("A passing OPAL_VERIFY leaves the handler alone in every build")
    {
        g_handler_called = false;
        const ScopedContractHandler scoped(&RecordViolation);
        OPAL_VERIFY(1 < g_size, "Index out of bounds");
        REQUIRE_FALSE(g_handler_called);
    }
}
