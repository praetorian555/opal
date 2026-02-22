#include "opal/defines.h"

#include "test-helpers.h"

#include "opal/allocator.h"

int main(int argc, char* argv[])
{
#if defined(OPAL_PLATFORM_WINDOWS)
    setlocale(LC_ALL, "");
#else
    setlocale(LC_ALL, "C.UTF-8");
#endif

    Opal::MallocAllocator malloc_allocator;
    Opal::PushDefaultAllocator(&malloc_allocator);

    Opal::LinearAllocator scratch_allocator(
        "Default Scratch Allocator",
        {.bytes_to_reserve = OPAL_GB(4), .bytes_to_initially_alloc = OPAL_MB(1), .commit_step_size = OPAL_MB(2)});
    Opal::PushScratchAllocator(&scratch_allocator);

    Catch::Session session;
    const int return_code = session.applyCommandLine(argc, argv);
    if (return_code != 0)
    {
        return return_code;
    }
    return session.run();
}
