// 
// MIT License
// 
// Copyright (c) 2018 Marcus Larsson
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//


#define UNICODE
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <assert.h>
#include <stdio.h>
#include <process.h>

#include "types.h"
#include "mathematics.h"
#include "directX11_renderer.h"
#include "ply_loader.h"

#define kFrameTime (1.0f / 60.0f)
#define kFrameTimeMicroSeconds 1000000.0f * kFrameTime

#define kThreadCount 4
#define kParticleCount 2000
//#define kMultithreaded



//
// Particle
//
struct particle
{
    v3 P;
    v3 dP;
    f32 Duration = 4.0f;
    f32 Elapsed = 0.0f;
};


struct particle_system
{
    particle *Data = nullptr;
    
    v3 P = v3_zero;
    v3 ddPg = V3(0.0f, -9.8f, 0.0f); // Gravity acceleration
    
    f32 force = 5.0f;
    f32 dt = kFrameTime;
    b32 IsSimulating = true;
};


struct thread_context
{
    particle_system *ParticleSystem = nullptr;
    HANDLE Simulate;
    HANDLE Finished;
    HANDLE ThreadHandle;
    u32 ThreadID;
    u32 StartIndex = 0;
    u32 EndIndex = 0;
};


unsigned int __stdcall ParticleUpdate(void* Data)
{
    thread_context *Context = (thread_context *)Data;
    particle_system *ParticleSystem = Context->ParticleSystem;
    u32 ThreadID = GetCurrentThreadId();
    f32 dt = ParticleSystem->dt;
    
    printf("Thread# %u handles %u <= Index < %u\n", ThreadID, Context->StartIndex, Context->EndIndex);
    
    while (ParticleSystem->IsSimulating)
    {
        DWORD WaitResult = WaitForSingleObject(Context->Simulate, INFINITE);
        switch (WaitResult) 
        {
            case WAIT_OBJECT_0: 
            {
                for (u32 Index = Context->StartIndex; Index < Context->EndIndex; ++Index)
                {
                    particle *Particle = &ParticleSystem->Data[Index];
                    
                    Particle->Elapsed += dt;
                    if (Particle->Elapsed > Particle->Duration)
                    {
                        Particle->Elapsed = 0;
                        Particle->P = ParticleSystem->P;
                        
                        f32 Radius = 1.0f;
                        
                        static f32 Angle = 0.0f;
                        Angle += Tau32 / 60.0f;
                        if (Angle > Tau32)
                        {
                            Angle = Tau32 - Angle;
                        }
                        
                        v3 F = V3(Radius * Cos(Angle), 1.0f, -Radius * Sin(Angle)) -  ParticleSystem->P;
                        Particle->dP = ParticleSystem->force * F;
                    }
                    else
                    {
                        Particle->P += Particle->dP * dt;
                        Particle->dP += ParticleSystem->ddPg * dt;
                    }
                }
                
                //printf("Thread# %u is done!\n", ThreadID);
                ResetEvent(Context->Simulate);
                SetEvent(Context->Finished);
            } break; 
            
            default: 
            {
                printf("ThreadID %u, wait error (%d)\n", ThreadID, GetLastError()); 
                return 0; 
            } break;
        }
    }
    printf("ThreadID %u, exiting...\n", ThreadID);
    
    return 0;
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (uMsg) {
        case WM_CLOSE: {
        } break;
        
        case WM_ENTERSIZEMOVE: {
        } break;
        
        case WM_EXITSIZEMOVE: {
        } break;
        
        case WM_SIZE: {
        } break;
        
        case WM_KEYDOWN: {
            
        } break;
        
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        } break;
        
        default: {
            //DebugPrint(L"Message = %d\n", uMsg);
        } break;
    }
    
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstancePrev, LPSTR lpCmdLine, int nShowCmd) 
{
    //
    // Create and attach console
    //
#ifdef DEBUG
    FILE *FileStdOut;
    assert(AllocConsole());
    freopen_s(&FileStdOut, "CONOUT$", "w", stdout);
#endif
    
    
    
    //
    // TODO(Marcus): Handle resolution in a proper way
    u32 const Width = 1920;
    u32 const Height = 1080;
    
    
    
    //
    // Create window
    //
    HWND hWnd;
    {
        wchar_t const ClassName[] = L"Beacon";
        
        WNDCLASS WC;
        
        WC.style = CS_VREDRAW | CS_HREDRAW;
        WC.lpfnWndProc = WindowProc;
        WC.cbClsExtra = 0;
        WC.cbWndExtra = 0;
        WC.hInstance = hInstance;
        WC.hIcon = nullptr;
        WC.hCursor = LoadCursor(nullptr, IDC_ARROW);
        WC.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        WC.lpszMenuName = nullptr;
        WC.lpszClassName = ClassName;
        
        DWORD Style =  WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        
        RECT Rect;
        Rect.left = 0;
        Rect.top = 0;
        Rect.right = Width;
        Rect.bottom = Height;
        AdjustWindowRect(&Rect, Style, false);
        
        if (!RegisterClass(&WC)) 
        {
            return 1;
        }
        
        hWnd = CreateWindow(
            ClassName,                      /* Class Name */
            ClassName,                      /* Title */
            Style,                          /* Style */
            CW_USEDEFAULT, CW_USEDEFAULT,   /* Position */
            Rect.right - Rect.left, 
            Rect.bottom - Rect.top ,        /* Size */
            nullptr,                        /* Parent */
            nullptr,                        /* No menu */
            hInstance,                      /* Instance */
            0);                             /* No special parameters */
        
        if (!hWnd)
        {
            DWORD Error = GetLastError();
            OutputDebugString(L"Failed to create window!\n");
            return Error;
        }
    }
    
    
    
    //
    // Init DirectX 10
    //
    directx_state DirectXState = {};
    {
        directx_config Config;
        Config.Width = Width;
        Config.Height = Height;
        Config.hWnd = hWnd;
        
        SetupDirectX(&DirectXState, &Config);
    }
    
    
    
    //
    // Constant buffer, used by the shaders
    //
    struct shader_constants
    {
        m4 ViewToClipMatrix;
        m4 WorldToViewMatrix;
        m4 ObjectToWorldMatrix;
        v4 CameraP;
        v4 Colour;
    };
    
    directx_buffer ConstantBuffer;
    shader_constants ShaderConstants;
    {
        v4 CameraP = V4(2.0f, 2.0f, 10.0f, 1.0f);
        f32 AspectRatio = (f32)DirectXState.Width / (f32)DirectXState.Height;
        
        ShaderConstants.ViewToClipMatrix  = M4Perspective(Pi32_2, AspectRatio, 1.0f, 100.0f);
        ShaderConstants.WorldToViewMatrix = M4LookAt(CameraP.xyz(), v3_zero);
        ShaderConstants.ObjectToWorldMatrix = m4_identity;
        ShaderConstants.CameraP = CameraP;
        ShaderConstants.Colour = V4(0.65f, 0.65f, 0.7f, 1.0f);
        assert(sizeof(shader_constants) % 16 == 0);
        
        b32 Result = CreateConstantBuffer(&DirectXState, &ShaderConstants, sizeof(shader_constants), &ConstantBuffer);
        assert(Result);
        
        SetConstantBuffer(&DirectXState, &ConstantBuffer);
    }
    
    
    
    //
    // A quad
    //
    directx_renderable RenderablePlane;
    {
        f32 x = 10.0f;
        f32 y =  0.0f;
        f32 z = 10.0f;
        
        v3 Vertices[] =
        {
            {-x,  y,  z},
            {-x,  y, -z},
            { x,  y, -z},
            
            { x,  y, -z},
            { x,  y,  z},
            {-x,  y,  z},
        };
        b32 Result = CreateRenderable(&DirectXState, &RenderablePlane, 
                                      Vertices, sizeof(v3), sizeof(Vertices) / sizeof(*Vertices),
                                      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        assert(Result);
    }
    
    
    //
    // Particles
    //
    particle_system ParticleSystem;
    ParticleSystem.Data = (particle *)calloc(kParticleCount, sizeof(particle));
    assert(ParticleSystem.Data);
    
    for (u32 Index = 0; Index < kParticleCount; ++Index)
    {
        particle *Particle = &ParticleSystem.Data[Index];
        Particle->P = ParticleSystem.P;
        Particle->dP = ParticleSystem.force * V3(0.0f, 1.0f, 0.0f);
        Particle->Duration = 2.0f;
        Particle->Elapsed = 0.0f;
    }
    
    
    //
    // A renderable for all the particles
    // 
    directx_renderable RenderableParticles;
    {
        b32 Result = CreateRenderable(&DirectXState, &RenderableParticles,
                                      ParticleSystem.Data, sizeof(v3), kParticleCount,
                                      D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        assert(Result);
    }
    
    
#ifdef kMultithreaded
    //
    // Create threads
    thread_context ThreadContext[kThreadCount];
    {
        u32 AllocatedIndices = 0;
        u32 ParticlesPerThread = kParticleCount / kThreadCount;
        
        for (u32 Index = 0; Index < kThreadCount; ++Index)
        {
            ThreadContext[Index].ParticleSystem = &ParticleSystem;
            
            if (Index < kThreadCount - 1)
            {
                ThreadContext[Index].StartIndex = AllocatedIndices;
                ThreadContext[Index].EndIndex = AllocatedIndices + ParticlesPerThread;
                AllocatedIndices += ParticlesPerThread;
            }
            else
            {
                ThreadContext[Index].StartIndex = AllocatedIndices;
                ThreadContext[Index].EndIndex = kParticleCount;
            }
            
            ThreadContext[Index].Simulate = CreateEventA(nullptr, true, false, nullptr);
            if(ThreadContext[Index].Simulate == nullptr)
            {
                printf("CreateEvent, Simulate, error: %d\n", GetLastError());
                return 1;
            }
            
            ThreadContext[Index].Finished = CreateEventA(nullptr, true, false, nullptr);
            if(ThreadContext[Index].Finished == nullptr)
            {
                printf("CreateEvent, Finished, error: %d\n", GetLastError());
                return 1;
            }
            
            ThreadContext[Index].ThreadHandle = (HANDLE)_beginthreadex(0, 
                                                                       0, 
                                                                       &ParticleUpdate, 
                                                                       (void *)&ThreadContext[Index], 
                                                                       0,
                                                                       &ThreadContext[Index].ThreadID);
            if(ThreadContext[Index].ThreadHandle == nullptr)
            {
                printf("_beginthreadex error: %d\n", GetLastError());
                return 1;
            }
            else
            {
                printf("Created thread %u.\n", ThreadContext[Index].ThreadID);
            }
        }
    }
#endif
    
    
    
    //
    // DirectWrite
    //
    directwrite_state DirectWriteState;
    InitDirectWrite(&DirectXState, &DirectWriteState);
    
    
    
    //
    // Show and update window
    //
    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);
    
    
    
    //
    // Timing
    //
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency); 
    
    LARGE_INTEGER RunTime;
    RunTime.QuadPart = 0;
    u32 FrameCount = 0;
    
    LARGE_INTEGER ParticleTime;
    ParticleTime.QuadPart = 0;
    
    
    
    //
    // The main loop
    //
    b32 ShouldRun = true;
    MSG msg;
    while (ShouldRun) 
    {
        LARGE_INTEGER FrameStartingTime;
        QueryPerformanceCounter(&FrameStartingTime);
        
        //
        // Handle the messages from the OS
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (msg.message == WM_QUIT) 
            {
                ShouldRun = false;
            }
        }
        
        
        //
        // Particle simulation
        LARGE_INTEGER ParticleStartingTime;
        QueryPerformanceCounter(&ParticleStartingTime);
        
#ifdef kMultithreaded
        //
        // Signal threads to do their thing
        for (u32 Index = 0; Index < kThreadCount; ++Index)
        {
            b32 Result = SetEvent(ThreadContext[Index].Simulate);
            if(!Result)
            {
                printf("SetEvent, thread %u error: %d\n", ThreadContext[Index].ThreadID, GetLastError());
            }
        }
        
        //
        // Wait until all threads are finished
        for (u32 Index = 0; Index < kThreadCount; ++Index)
        {
            WaitForSingleObject(ThreadContext[Index].Finished, INFINITE);
            ResetEvent(ThreadContext[Index].Finished);
        }
#else
        
        f32 dt = ParticleSystem.dt;
        for (u32 Index = 0; Index < kParticleCount; ++Index)
        {
            particle *Particle = &ParticleSystem.Data[Index];
            
            Particle->Elapsed += dt;
            if (Particle->Elapsed > Particle->Duration)
            {
                Particle->Elapsed = 0;
                Particle->P = ParticleSystem.P;
                
                f32 Radius = 1.0f;
                
                static f32 Angle = 0.0f;
                Angle += Tau32 / 60.0f;
                
                if (Angle > Tau32)
                {
                    Angle = Angle = Tau32 - Angle;
                }
                
                v3 F = V3(Radius * Cos(Angle), 1.0f, -Radius * Sin(Angle)) - ParticleSystem.P;
                Particle->dP = ParticleSystem.force * F;
            }
            else
            {
                Particle->P += Particle->dP * dt;
                Particle->dP += ParticleSystem.ddPg * dt;
            }
        }
        
#endif
        LARGE_INTEGER ParticleEndingTime;
        QueryPerformanceCounter(&ParticleEndingTime);
        
        LARGE_INTEGER ElapsedMicroseconds;
        ElapsedMicroseconds.QuadPart = ParticleEndingTime.QuadPart - ParticleStartingTime.QuadPart;
        ElapsedMicroseconds.QuadPart *= 1000000;
        ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
        ParticleTime.QuadPart += ElapsedMicroseconds.QuadPart;
        
        
        //
        // Render
        SetRenderTarget(&DirectXState, &DirectXState.RenderTarget);
        SetShader(&DirectXState, &DirectXState.vShaderBasic);
        SetShader(&DirectXState, &DirectXState.pShaderBasic);
        BeginRendering(&DirectXState);
        
        
        //
        // Render plane
        {
            ShaderConstants.Colour = V4(0.65f, 0.65f, 0.7f, 1.0f);
            ShaderConstants.ObjectToWorldMatrix = m4_identity;
            UpdateBuffer(&DirectXState, &ConstantBuffer, &ShaderConstants, nullptr);
            
            RenderRenderable(&DirectXState, &RenderablePlane);
        }
        
        //
        // Render particles
        {
            UpdateBuffer(&DirectXState, &RenderableParticles.VertexBuffer, ParticleSystem.Data, nullptr);
            
            //v3 *P = &ParticleSystem.Data[0].P;
            //printf("(%5.2f, %5.2f, %5.2f)\n", P->x, P->y, P->z);
            //ShaderConstants.ObjectToWorldMatrix = M4Translation(*P);
            
            ShaderConstants.Colour = V4(1.0f, 0.0f, 0.0f, 1.0f);
            UpdateBuffer(&DirectXState, &ConstantBuffer, &ShaderConstants, nullptr);
            
            SetShader(&DirectXState, &DirectXState.gShader);
            RenderRenderable(&DirectXState, &RenderableParticles);
            SetShader(&DirectXState, (geometry_shader *)nullptr);
        }
        
        EndRendering(&DirectXState);
        
        
        //
        // Timing
        LARGE_INTEGER FrameEndingTime;
        QueryPerformanceCounter(&FrameEndingTime);
        
        ElapsedMicroseconds.QuadPart = FrameEndingTime.QuadPart - FrameStartingTime.QuadPart;
        ElapsedMicroseconds.QuadPart *= 1000000;
        ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
        
        while (ElapsedMicroseconds.QuadPart < kFrameTimeMicroSeconds)
        {
            QueryPerformanceCounter(&FrameEndingTime);
            
            ElapsedMicroseconds.QuadPart = FrameEndingTime.QuadPart - FrameStartingTime.QuadPart;
            ElapsedMicroseconds.QuadPart *= 1000000;
            ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
        }
        
        ++FrameCount;
        
        RunTime.QuadPart += ElapsedMicroseconds.QuadPart;
        if (RunTime.QuadPart > 1000000)
        {
            f32 ParticleFps = 1000000.0f * ((f32)FrameCount / (f32)ParticleTime.QuadPart);
            printf("fps: %u, particle fps = %f\n", FrameCount, ParticleFps);
            RunTime.QuadPart = 0;
            FrameCount = 0;
            ParticleTime.QuadPart = 0;
        }
    }
    
    
    
    //
    // Close threads
    //
#ifdef Multithreaded
    ParticleSystem.IsSimulating = false;
    for (u32 Index = 0; Index < kThreadCount; ++Index)
    {
        WaitForSingleObject(ThreadContext[Index].ThreadHandle, INFINITE);
    }
    
    for (u32 Index = 0; Index < kThreadCount; ++Index)
    {
        b32 Result = CloseHandle(ThreadContext[Index].ThreadHandle);
        if(!Result)
        {
            printf("CloseHandle, thread %u error: %d\n", ThreadContext[Index].ThreadID, GetLastError());
        }
        else
        {
            printf("Closed thread %u.\n", ThreadContext[Index].ThreadID);
        }
    }
#endif
    
    
    
    //
    // @debug
    //
#ifdef DEBUG
    IDXGIDebug *DebugInterface;
    {
        HMODULE hModule = GetModuleHandleA("Dxgidebug.dll");
        if (!hModule) {
            OutputDebugString(L"Failed to get module handle to Dxgidebug.dll\n");
            return 1;
        }
        
        typedef HRESULT (*GETDEBUGINTERFACE)(REFIID, void **);
        GETDEBUGINTERFACE GetDebugInterface;
        GetDebugInterface = (GETDEBUGINTERFACE)GetProcAddress(hModule, "DXGIGetDebugInterface");
        
        HRESULT Result = GetDebugInterface(__uuidof(IDXGIDebug), (void **)&DebugInterface);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to get the debug interface!\n");
            return 1;
        }
    }
#endif
    
    
    
    //
    // Clean up
    //
    ReleaseDirectWrite(&DirectWriteState);
    ReleaseRenderable(&RenderablePlane);
    ReleaseRenderable(&RenderableParticles);
    ReleaseBuffer(&ConstantBuffer);
    ReleaseRenderTarget(&DirectXState.RenderTarget);
    ReleaseDirectXState(&DirectXState);
    
    
#ifdef DEBUG
    OutputDebugString(L"***** Begin ReportLiveObjects call *****\n");
    DebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
    OutputDebugString(L"***** End   ReportLiveObjects call *****\n\n");
    
    if (DebugInterface) {
        DebugInterface->Release();
    }
    
    FreeConsole();
#endif
    
    return msg.wParam;
}