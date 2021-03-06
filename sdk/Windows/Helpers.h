#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <stdarg.h>
#include <windows.h>
#include <functional>
#include <memory>
using namespace std;

namespace SMX
{
void Log(string s);

// Set a function to receive logs written by SMX::Log.  By default, logs are written
// to stdout.
void SetLogCallback(function<void(const string &log)> callback);

void SetThreadName(DWORD iThreadId, const string &name);
void StripCrnl(wstring &s);
wstring GetErrorString(int err);
string vssprintf(const char *szFormat, va_list argList);
string ssprintf(const char *fmt, ...);
string BinaryToHex(const void *pData_, int iNumBytes);
string BinaryToHex(const string &sString);
bool GetRandomBytes(void *pData, int iBytes);
double GetMonotonicTime();

// In order to be able to use smart pointers to fully manage an object, we need to get
// a shared_ptr to pass around, but also store a weak_ptr in the object itself.  This
// lets the object create shared_ptrs for itself as needed, without keeping itself from
// being deallocated.
//
// This helper allows this pattern:
//
// struct Class
// {
//    Class(shared_ptr<Class> &pSelf): m_pSelf(GetPointers(pSelf, this)) { }
//    const weak_ptr<Class> m_pSelf;
// };
//
// shared_ptr<Class> obj;
// new Class(obj);
//
// For a more convenient way to invoke this, see CreateObj() below.

template<typename T>
weak_ptr<T> GetPointers(shared_ptr<T> &pSharedPtr, T *pObj)
{
    pSharedPtr.reset(pObj);
    return pSharedPtr;
}

// Create a class that retains a weak reference to itself, returning a shared_ptr.
template<typename T, class... Args>
shared_ptr<T> CreateObj(Args&&... args)
{
    shared_ptr<typename T> pResult;
    new T(pResult, std::forward<Args>(args)...);
    return dynamic_pointer_cast<T>(pResult);
}

class AutoCloseHandle
{
public:
    AutoCloseHandle(HANDLE h);
    ~AutoCloseHandle();
    HANDLE value() const { return handle; }

private:
    AutoCloseHandle(const AutoCloseHandle &rhs);
    AutoCloseHandle &operator=(const AutoCloseHandle &rhs);
    HANDLE handle;
};

class Mutex
{
public:
    Mutex();
    ~Mutex();
    void Lock();
    void Unlock();

    void AssertNotLockedByCurrentThread();
    void AssertLockedByCurrentThread();

private:
    HANDLE m_hLock = INVALID_HANDLE_VALUE;
    DWORD m_iLockedByThread = 0;
};

// A local lock helper for Mutex.
class LockMutex
{
public:
    LockMutex(Mutex &mutex);
    ~LockMutex();

private:
    Mutex &m_Mutex;
};
}

#endif
