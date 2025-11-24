#include "opal/defines.h"

#include "test-helpers.h"

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
