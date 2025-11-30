#include "opal/thread.h"

#include "opal/container/scope-ptr.h"

#if defined(OPAL_PLATFORM_WINDOWS)
#include "Windows.h"
#elif defined(OPAL_PLATFORM_LINUX)
#include <pthread.h>
#endif

#if defined(OPAL_PLATFORM_WINDOWS)
DWORD WINAPI WindowsThread(LPVOID param)
{
    Opal::Impl::ThreadDataBase* data = static_cast<Opal::Impl::ThreadDataBase*>(param);
    data->Invoke();
    return 0;
}
#elif defined(OPAL_PLATFORM_LINUX)
void* ThreadFunction(void* param)
{
    Opal::Impl::ThreadDataBase* data = static_cast<Opal::Impl::ThreadDataBase*>(param);
    data->Invoke();
    return nullptr;
}
#endif

Opal::ThreadHandle Opal::Impl::CreateThread(ThreadDataBase* data)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    HANDLE thread_handle = ::CreateThread(nullptr, 0, WindowsThread, data, 0, nullptr);
    return {thread_handle};
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_t native_handle;
    pthread_create(&native_handle, nullptr, ThreadFunction, data);
    ThreadHandle handle = {.native_handle = reinterpret_cast<void*>(native_handle)};
    return handle;
#else
    throw NotImplementedException(__FUNCTION__);
#endif

}

void Opal::JoinThread(ThreadHandle handle)
{
    if (handle.native_handle != nullptr)
    {
#if defined(OPAL_PLATFORM_WINDOWS)
        ::WaitForSingleObject(handle.native_handle, INFINITE);
#elif defined(OPAL_PLATFORM_LINUX)
        pthread_t native_handle = reinterpret_cast<pthread_t>(handle.native_handle);
        pthread_join(native_handle, nullptr);
#else
        throw NotImplementedException(__FUNCTION__);
#endif
        // TODO: Save allocator and use it here
        Delete(handle.data);
    }
}