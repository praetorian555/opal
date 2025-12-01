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
    Delete(data, data->allocator);
    return 0;
}
#elif defined(OPAL_PLATFORM_LINUX)
void* ThreadFunction(void* param)
{
    Opal::Impl::ThreadDataBase* data = static_cast<Opal::Impl::ThreadDataBase*>(param);
    data->Invoke();
    Delete(data, data->allocator);
    return nullptr;
}
#endif

Opal::ThreadHandle Opal::Impl::CreateThread(ThreadDataBase* data)
{
#if defined(OPAL_PLATFORM_WINDOWS)
    DWORD thread_id = 0;
    HANDLE thread_handle = ::CreateThread(nullptr, 0, WindowsThread, data, 0, &thread_id);
    if (thread_handle == nullptr)
    {
        throw Exception("Failed to create thread!");
    }
    return {.native_handle = thread_handle, .native_id = reinterpret_cast<void*>(static_cast<u64>(thread_id))};
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_t native_handle;
    pthread_create(&native_handle, nullptr, ThreadFunction, data);
    ThreadHandle handle = {.native_handle = reinterpret_cast<void*>(native_handle)};
    return handle;
#else
    throw NotImplementedException(__FUNCTION__);
#endif

}

bool Opal::ThreadHandle::operator==(const ThreadHandle& other) const
{
#if defined(OPAL_PLATFORM_WINDOWS)
    return native_id == other.native_id;
#elif defined(OPAL_PLATFORM_LINUX)
    return native_handle == other.native_handle;
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
        ::CloseHandle(handle.native_handle);
#elif defined(OPAL_PLATFORM_LINUX)
        pthread_t native_handle = reinterpret_cast<pthread_t>(handle.native_handle);
        pthread_join(native_handle, nullptr);
#else
        throw NotImplementedException(__FUNCTION__);
#endif
    }
}

Opal::ThreadHandle Opal::GetCurrentThreadHandle()
{
#if defined(OPAL_PLATFORM_WINDOWS)
    const DWORD thread_id = ::GetCurrentThreadId();
    return { .native_id = reinterpret_cast<void*>(static_cast<u64>(thread_id))};
#elif defined(OPAL_PLATFORM_LINUX)
    pthread_t thread_handle = pthread_self();
    return {.native_handle = reinterpret_cast<void*>(thread_handle) };
#else
    throw NotImplementedException(__FUNCTION__);
#endif
}
