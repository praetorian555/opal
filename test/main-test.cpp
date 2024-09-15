#include "opal/defines.h"

OPAL_START_DISABLE_WARNINGS
OPAL_DISABLE_WARNING(-Wnon-virtual-dtor)
#define CATCH_CONFIG_RUNNER
#include "catch2/catch2.hpp"
OPAL_END_DISABLE_WARNINGS

int main(int argc, char* argv[])
{
#if defined(OPAL_PLATFORM_WINDOWS)
    setlocale(LC_ALL, "");
#else
    setlocale(LC_ALL, "C.UTF-8");
#endif

    Catch::Session session;
    const int return_code = session.applyCommandLine(argc, argv);
    if (return_code != 0)
    {
        return return_code;
    }
    return session.run();
}
