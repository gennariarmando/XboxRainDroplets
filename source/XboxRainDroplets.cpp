//d3d8 version should be compiled separately, but the hook works regardless
//#define DIRECT3D_VERSION 0x0800

#include "kiero.h"
#include "sire.h"

#include "xrd11.h"
#undef DXGI_SAMPLE_DESC
#if(DIRECT3D_VERSION < 0x0900)
typedef LPDIRECT3DDEVICE8 LPDIRECT3DDEVICE9;
typedef LPDIRECT3DDEVICE8 LPDIRECT3DDEVICE9EX;
typedef void D3DDISPLAYMODEEX;
#endif

template<typename... Args>
class Event : public std::function<void(Args...)>
{
public:
    using std::function<void(Args...)>::function;

private:
    std::vector<std::function<void(Args...)>> handlers;

public:
    void operator+=(std::function<void(Args...)> handler)
    {
        handlers.push_back(handler);
    }

    void operator()(Args... args) const
    {
        if (!handlers.empty())
        {
            for (auto& handler : handlers)
            {
                handler(args...);
            }
        }
    }
};

class DX8Hook
{
public:
    static inline bool bInitialized = false;
    static inline uintptr_t presentOriginalPtr = 0;
    static inline uintptr_t resetOriginalPtr = 0;

    static inline Event<> onInitEvent = {};
    static inline Event<> onShutdownEvent = {};
#if KIERO_INCLUDE_D3D8
    static inline Event<D3D8_LPDIRECT3DDEVICE8> onPresentEvent = {};
    static inline Event<D3D8_LPDIRECT3DDEVICE8> onResetEvent = {};
#endif

    ~DX8Hook() {
        kiero::shutdown();
        onShutdownEvent();
    }

    static inline bool Init() {
#if KIERO_INCLUDE_D3D8
        if (!bInitialized && kiero::init(kiero::RenderType::D3D8) == kiero::Status::Success) {
            kiero::bind(IDirect3DDevice8VTBL::Present, (void**)&DX8Hook::presentOriginalPtr, DX8Hook::Present);
            kiero::bind(IDirect3DDevice8VTBL::Reset, (void**)&DX8Hook::resetOriginalPtr, DX8Hook::Reset);
            onInitEvent();
            bInitialized = true;
            return true;
        }
#endif
        return false;
    }

#if KIERO_INCLUDE_D3D8
    static inline HRESULT WINAPI Present(D3D8_LPDIRECT3DDEVICE8 pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
        onPresentEvent(pDevice);
        return ((HRESULT(WINAPI*)(D3D8_LPDIRECT3DDEVICE8, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*))presentOriginalPtr)(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }
    
    static inline HRESULT WINAPI Reset(D3D8_LPDIRECT3DDEVICE8 pDevice, D3DPRESENT_PARAMETERS_D3D8* pPresentationParameters)
    {
        onResetEvent(pDevice);
        return ((HRESULT(WINAPI*)(D3D8_LPDIRECT3DDEVICE8, D3DPRESENT_PARAMETERS_D3D8*))resetOriginalPtr)(pDevice, pPresentationParameters);
    }
#endif
};

class DX9Hook
{
public:
    static inline bool bInitialized = false;
    static inline uintptr_t presentOriginalPtr = 0;
    static inline uintptr_t presentExOriginalPtr = 0;
    static inline uintptr_t resetOriginalPtr = 0;
    static inline uintptr_t resetExOriginalPtr = 0;

    static inline Event<> onInitEvent = {};
    static inline Event<> onShutdownEvent = {};
#if KIERO_INCLUDE_D3D9
    static inline Event<LPDIRECT3DDEVICE9> onPresentEvent = {};
    static inline Event<LPDIRECT3DDEVICE9> onResetEvent = {};
#endif

    ~DX9Hook() {
        kiero::shutdown();
        onShutdownEvent();
    }

    static inline bool Init() {
#if KIERO_INCLUDE_D3D9
        if (!bInitialized && kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success) {
            kiero::bind(IDirect3DDevice9VTBL::Present, (void**)&DX9Hook::presentOriginalPtr, DX9Hook::Present);
            kiero::bind(IDirect3DDevice9VTBL::PresentEx, (void**)&DX9Hook::presentExOriginalPtr, DX9Hook::PresentEx);
            kiero::bind(IDirect3DDevice9VTBL::Reset, (void**)&DX9Hook::resetOriginalPtr, DX9Hook::Reset);
            kiero::bind(IDirect3DDevice9VTBL::ResetEx, (void**)&DX9Hook::resetExOriginalPtr, DX9Hook::ResetEx);
            onInitEvent();
            bInitialized = true;
            return true;
        }
#endif
        return false;
    }

#if KIERO_INCLUDE_D3D9
    static inline HRESULT WINAPI Present(LPDIRECT3DDEVICE9 pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
        onPresentEvent(pDevice);
        return ((HRESULT(WINAPI*)(LPDIRECT3DDEVICE9, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*))presentOriginalPtr)(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }

    static inline HRESULT WINAPI PresentEx(LPDIRECT3DDEVICE9EX pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags) {
        onPresentEvent(pDevice);
        return ((HRESULT(WINAPI*)(LPDIRECT3DDEVICE9EX, const RECT*, const RECT*, HWND, const RGNDATA*, DWORD))presentExOriginalPtr)(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
    }

    static inline HRESULT WINAPI Reset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
    {
        onResetEvent(pDevice);
        return ((HRESULT(WINAPI*)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*))resetOriginalPtr)(pDevice, pPresentationParameters);
    }

    static inline HRESULT WINAPI ResetEx(LPDIRECT3DDEVICE9EX pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode)
    {
        onResetEvent(pDevice);
        return ((HRESULT(WINAPI*)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*, D3DDISPLAYMODEEX*))resetExOriginalPtr)(pDevice, pPresentationParameters, pFullscreenDisplayMode);
    }
#endif
};

class DX10Hook
{
public:
    static inline bool bInitialized = false;
    static inline uintptr_t presentOriginalPtr = 0;
    static inline uintptr_t resizeBuffersOriginalPtr = 0;

    static inline Event<> onInitEvent = {};
    static inline Event<> onShutdownEvent = {};
#if KIERO_INCLUDE_D3D10
    static inline Event<IDXGISwapChain*> onPresentEvent = {};
    static inline Event<IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT> onBeforeResizeEvent = {};
    static inline Event<IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT> onAfterResizeEvent = {};
#endif

    ~DX10Hook() {
        kiero::shutdown();
        onShutdownEvent();
    }

    static inline bool Init() {
#if KIERO_INCLUDE_D3D10
        if (!bInitialized && kiero::init(kiero::RenderType::D3D10) == kiero::Status::Success) {
            kiero::bind(IDirect3DDevice10VTBL::Present, (void**)&DX10Hook::presentOriginalPtr, DX10Hook::Present);
            kiero::bind(IDirect3DDevice10VTBL::ResizeBuffers, (void**)&DX10Hook::resizeBuffersOriginalPtr, DX10Hook::ResizeBuffers);
            onInitEvent();
            bInitialized = true;
            return true;
        }
#endif
        return false;
    }

#if KIERO_INCLUDE_D3D10
    static inline HRESULT WINAPI Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
        onPresentEvent(pSwapChain);
        return ((HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT))presentOriginalPtr)(pSwapChain, SyncInterval, Flags);
    }

    static inline HRESULT WINAPI ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
        onBeforeResizeEvent(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        HRESULT result = ((HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT))resizeBuffersOriginalPtr)(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        onAfterResizeEvent(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        return result;
    }
#endif
};

class DX11Hook
{
public:
    static inline bool bInitialized = false;
    static inline uintptr_t presentOriginalPtr = 0;
    static inline uintptr_t resizeBuffersOriginalPtr = 0;

    static inline Event<> onInitEvent = {};
    static inline Event<> onShutdownEvent = {};
#if KIERO_INCLUDE_D3D11
    static inline Event<IDXGISwapChain*> onPresentEvent = {};
    static inline Event<IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT> onBeforeResizeEvent = {};
    static inline Event<IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT> onAfterResizeEvent = {};
#endif

    ~DX11Hook() {
        kiero::shutdown();
        onShutdownEvent();
    }

    static inline bool Init() {
#if KIERO_INCLUDE_D3D11
        if (!bInitialized && kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
            kiero::bind(IDirect3DDevice11VTBL::Present, (void**)&DX11Hook::presentOriginalPtr, DX11Hook::Present);
            kiero::bind(IDirect3DDevice11VTBL::ResizeBuffers, (void**)&DX11Hook::resizeBuffersOriginalPtr, DX11Hook::ResizeBuffers);
            onInitEvent();
            bInitialized = true;
            return true;
        }
#endif
        return false;
    }

#if KIERO_INCLUDE_D3D11
    static inline HRESULT WINAPI Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
        onPresentEvent(pSwapChain);
        return ((HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT))presentOriginalPtr)(pSwapChain, SyncInterval, Flags);
    }

    static inline HRESULT WINAPI ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
        onBeforeResizeEvent(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        HRESULT result = ((HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT))resizeBuffersOriginalPtr)(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        onAfterResizeEvent(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        return result;
    }
#endif
};

class DX12Hook
{
public:
    static inline bool bInitialized = false;
    static inline uintptr_t presentOriginalPtr = 0;
    static inline uintptr_t resizeBuffersOriginalPtr = 0;

    static inline Event<> onInitEvent = {};
    static inline Event<> onShutdownEvent = {};
#if KIERO_INCLUDE_D3D12
    static inline Event<IDXGISwapChain*> onPresentEvent = {};
    static inline Event<IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT> onBeforeResizeEvent = {};
    static inline Event<IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT> onAfterResizeEvent = {};
#endif

    ~DX12Hook() {
        kiero::shutdown();
        onShutdownEvent();
    }

    static inline bool Init() {
#if KIERO_INCLUDE_D3D12
        if (!bInitialized && kiero::init(kiero::RenderType::D3D12) == kiero::Status::Success) {
            kiero::bind(IDirect3DDevice12VTBL::Present, (void**)&DX12Hook::presentOriginalPtr, DX12Hook::Present);
            kiero::bind(IDirect3DDevice12VTBL::ResizeBuffers, (void**)&DX12Hook::resizeBuffersOriginalPtr, DX12Hook::ResizeBuffers);
            onInitEvent();
            bInitialized = true;
            return true;
        }
#endif
        return false;
    }

#if KIERO_INCLUDE_D3D12
    static inline HRESULT WINAPI Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
        onPresentEvent(pSwapChain);
        return ((HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT))presentOriginalPtr)(pSwapChain, SyncInterval, Flags);
    }

    static inline HRESULT WINAPI ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
        onBeforeResizeEvent(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        HRESULT result = ((HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT))resizeBuffersOriginalPtr)(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        onAfterResizeEvent(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        return result;
    }
#endif
};

class OpenGLHook
{
public:
    static inline bool bInitialized = false;
    static inline uintptr_t wglSwapBuffersOriginalPtr = 0;

    static inline Event<> onInitEvent = {};
    static inline Event<> onShutdownEvent = {};
#if KIERO_INCLUDE_OPENGL
    static inline Event<HDC> onSwapBuffersEvent = {};
#endif

    ~OpenGLHook() {
        kiero::shutdown();
        onShutdownEvent();
    }

    static inline bool Init() {
#if KIERO_INCLUDE_OPENGL
        if (!bInitialized && kiero::init(kiero::RenderType::OpenGL) == kiero::Status::Success) {
            kiero::bind(OpenGLVTBL::wglSwapBuffers, (void**)&OpenGLHook::wglSwapBuffersOriginalPtr, OpenGLHook::wglSwapBuffers);
            onInitEvent();
            bInitialized = true;
            return true;
        }
#endif
        return false;
    }

#if KIERO_INCLUDE_OPENGL
    static inline BOOL __stdcall wglSwapBuffers(HDC hDc) {
        onSwapBuffersEvent(hDc);
        return ((BOOL(__stdcall*)(HDC hDc))wglSwapBuffersOriginalPtr)(hDc);
    }
#endif
};

void Init()
{
//#ifdef DEBUG
//    AllocConsole();
//    freopen("conin$", "r", stdin);
//    freopen("conout$", "w", stdout);
//    freopen("conout$", "w", stderr);
//    std::setvbuf(stdout, NULL, _IONBF, 0);
//#endif

    CallbackHandler::CreateThreadAutoClose(nullptr, 0, [](LPVOID data) -> DWORD {
        HANDLE hTimer = NULL;
        LARGE_INTEGER liDueTime;
        liDueTime.QuadPart = -30 * 10000000LL;
        hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
        if (hTimer != NULL) {
            SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);
            while (true)
            {
                DX8Hook::Init();
                DX9Hook::Init();
                DX10Hook::Init();
                DX11Hook::Init();
                DX12Hook::Init();
                OpenGLHook::Init();

                Sleep(0);
                if (WaitForSingleObject(hTimer, 0) == WAIT_OBJECT_0) {
                    CloseHandle(hTimer);
                    return 1;
                }
            };
        }
        return 0;
    }, nullptr, 0, nullptr);

    WaterDrops::ReadIniSettings();

#if KIERO_INCLUDE_D3D8
    DX8Hook::onInitEvent += []()
    {

    };

    DX8Hook::onPresentEvent += [](D3D8_LPDIRECT3DDEVICE8 pDevice)
    {
#if(DIRECT3D_VERSION < 0x0900)
        WaterDrops::Process((LPDIRECT3DDEVICE8)pDevice);
        WaterDrops::Render((LPDIRECT3DDEVICE8)pDevice);
#endif
    };

    DX8Hook::onResetEvent += [](D3D8_LPDIRECT3DDEVICE8 pDevice)
    {
#if(DIRECT3D_VERSION < 0x0900)
        WaterDrops::Reset();
#endif
    };

    DX8Hook::onShutdownEvent += []()
    {

    };
#endif

#if KIERO_INCLUDE_D3D9
    DX9Hook::onInitEvent += []()
    {

    };

    DX9Hook::onPresentEvent += [](LPDIRECT3DDEVICE9 pDevice)
    {
     
    };

    DX9Hook::onResetEvent += [](LPDIRECT3DDEVICE9 pDevice)
    {
        WaterDrops::Reset();
    };

    DX9Hook::onShutdownEvent += []()
    {

    };
#endif

#if KIERO_INCLUDE_D3D10
    DX10Hook::onInitEvent += []()
    {

    };

    DX10Hook::onPresentEvent += [](IDXGISwapChain* pSwapChain)
    {
        //MessageBox(0, 0, 0, 0);
    };

    DX10Hook::onShutdownEvent += []()
    {

    };

    DX10Hook::onBeforeResizeEvent += [](IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)
    {

    };

    DX10Hook::onAfterResizeEvent += [](IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)
    {

    };
#endif
    
#if KIERO_INCLUDE_D3D11
    DX11Hook::onInitEvent += []()
    {

    };
    
    DX11Hook::onPresentEvent += [](IDXGISwapChain* pSwapChain)
    {
        WaterDrops::Process(pSwapChain);
        WaterDrops::Render(pSwapChain);
    };
    
    DX11Hook::onShutdownEvent += []()
    {
        WaterDrops::Shutdown();
    };

    DX11Hook::onBeforeResizeEvent += [](IDXGISwapChain* pSwapChain, UINT, UINT, UINT, DXGI_FORMAT, UINT)
    {
        WaterDrops::BeforeResize(pSwapChain);
    };

    DX11Hook::onAfterResizeEvent += [](IDXGISwapChain* pSwapChain, UINT, UINT width, UINT height, DXGI_FORMAT, UINT)
    {
        WaterDrops::AfterResize(pSwapChain, width, height);
    };
#endif

#if KIERO_INCLUDE_D3D12
    DX12Hook::onInitEvent += []()
    {

    };

    DX12Hook::onPresentEvent += [](IDXGISwapChain* pSwapChain)
    {
        //MessageBox(0, 0, 0, 0);
    };

    DX12Hook::onShutdownEvent += []()
    {

    };

    DX12Hook::onBeforeResizeEvent += [](IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)
    {

    };

    DX12Hook::onAfterResizeEvent += [](IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)
    {

    };
#endif

#if KIERO_INCLUDE_OPENGL
    OpenGLHook::onInitEvent += []()
    {

    };

    OpenGLHook::onSwapBuffersEvent += [](HDC hDc)
    {
        //MessageBox(0, 0, 0, 0);
    };

    OpenGLHook::onShutdownEvent += []()
    {

    };
#endif
}

extern "C" __declspec(dllexport) void InitializeASI()
{
    std::call_once(CallbackHandler::flag, []()
    {
        CallbackHandler::RegisterCallback(Init);
    });
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID /*lpReserved*/)
{
    DisableThreadLibraryCalls(hModule);

    if (reason == DLL_PROCESS_ATTACH)
    {
        if (!IsUALPresent()) { InitializeASI(); }
    }
    return TRUE;
}
