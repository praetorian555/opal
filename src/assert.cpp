#include "opal/assert.h"

#include <cstdio>
#include <cstdlib>

namespace
{
[[noreturn]] void AbortOnContractViolation(const char* message)
{
    std::fprintf(stderr, "Opal contract violation: %s\n", message != nullptr ? message : "(no message)");
    std::fflush(stderr);
    std::abort();
}

Opal::ContractViolationHandler g_contract_handler = &AbortOnContractViolation;
}  // namespace

void Opal::SetContractViolationHandler(ContractViolationHandler handler)
{
    g_contract_handler = (handler != nullptr) ? handler : &AbortOnContractViolation;
}

Opal::ContractViolationHandler Opal::GetContractViolationHandler()
{
    return g_contract_handler;
}

void Opal::HandleContractViolation(const char* message)
{
    g_contract_handler(message);
    // A handler that returns would leave the caller running past a check it already failed, so this does not.
    AbortOnContractViolation(message);
}
