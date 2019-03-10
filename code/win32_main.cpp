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
#include <windowsx.h>

#include <assert.h>
#include <stdio.h>

#include "types.h"
#include "mathematics.h"
#include "directX11_renderer.h"
#include "ply_loader.h"
#include "particle_system.h"

constexpr f32 kFrameTime = 1.0f / 60.0f;
constexpr f32 kFrameTimeMicroSeconds = 1000000.0f * kFrameTime;
constexpr u32 kThreadCount = 4;
constexpr u32 kParticleCount = 1000;

struct display_metrics
{
    u32 WindowWidth;
    u32 WindowHeight;
    
    u32 ScreenWidth;
    u32 ScreenHeight;
};

struct mouse_state
{
    display_metrics *Metrics;
    
    v2  P = v2_zero;
    v2  PrevP = v2_zero;
    b32 RBDown = false;
};

#include "camera.h"

struct app_state
{
    mouse_state MouseState;
    camera Camera;
    
    display_metrics Metrics; 
    
    HWND hWnd;
};

#include "win32_utilities.h"



//
// MAIN
//
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
    // Initial state
    app_state AppState;
    AppState.Metrics.WindowWidth = 1920;
    AppState.Metrics.WindowHeight = 1080;
    AppState.Metrics.ScreenWidth  = GetSystemMetrics(SM_CXSCREEN);
    AppState.Metrics.ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    AppState.MouseState.Metrics = &AppState.Metrics;
    
    AppState.Camera.P = V3(0.0f, 1.0f, 1.0f);
    AppState.Camera.Distance = 200.0f;
    AppState.Camera.OrbitAngle = 3.0f * Pi32_2;
    AppState.Camera.PitchAngle = 3.0f * Pi32_4;
    AppState.Camera.Metrics = &AppState.Metrics;
    AppState.Camera.MouseState = &AppState.MouseState;
    UpdateCamera(&AppState.Camera);
    
    
    //
    // Create window
    //
    {
        u32 Error = CreateAppWindow(&AppState, hInstance, WindowProc);
        if (Error)
        {
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
        Config.Width = AppState.Metrics.WindowWidth;
        Config.Height = AppState.Metrics.WindowHeight;
        Config.hWnd = AppState.hWnd;
        
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
        m4 NormalToWorldMatrix;
        v4 Colour;
        v4 CameraP;
        v4 LightP;
    };
    
    directx_buffer ConstantBuffer;
    shader_constants ShaderConstants;
    {
        f32 AspectRatio = (f32)DirectXState.Width / (f32)DirectXState.Height;
        
        ShaderConstants.ViewToClipMatrix  = M4Perspective(1.5f * Pi32_4, AspectRatio, 1.0f, 1000.0f);
        ShaderConstants.WorldToViewMatrix = GetWorldToViewMatrix(&AppState.Camera);
        ShaderConstants.ObjectToWorldMatrix = m4_identity;
        ShaderConstants.NormalToWorldMatrix = m4_identity;
        ShaderConstants.CameraP = V4(AppState.Camera.P, 1.0f);
        ShaderConstants.Colour = V4(0.65f, 0.65f, 0.7f, 1.0f);
        ShaderConstants.LightP = V4(-80.0f, 100.0f, -80.0f, 1.0f);
        assert(sizeof(shader_constants) % 16 == 0);
        
        b32 Result = CreateConstantBuffer(&DirectXState, &ShaderConstants, sizeof(shader_constants), &ConstantBuffer);
        assert(Result);
    }
    
    
    
    //
    // X, Y and Z axis in world space (the fourth is light direction)
    // @debug
    directx_renderable RenderableAxisInWorld[3];
    {
        f32 l = 5.0f;
        v3 Vertices[] =
        {
            {0.0f, 0.0f, 0.0f},
            {   l, 0.0f, 0.0f},
            
            {0.0f, 0.0f, 0.0f},
            {0.0f,    l, 0.0f},
            
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f,    l},
        };
        
        b32 Result;
        for (u32 Index = 0; Index < 3; ++Index)
        {
            Result = CreateRenderable(&DirectXState, &RenderableAxisInWorld[Index],
                                      Vertices + (2 * Index), sizeof(v3), 2,
                                      D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
            assert(Result);
        }
    }
    
    directx_renderable RenderableLightDirection;
    {
        v3 Vertices[] =
        {
            {0.0f, 0.0f, 0.0f},
            ShaderConstants.LightP.xyz(),
        };
        
        b32 Result;
        Result = CreateRenderable(&DirectXState, &RenderableLightDirection,
                                  Vertices, sizeof(v3), 2,
                                  D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        assert(Result);
    }
    
    
    
    //
    // Load the terrain, height data is in a text file
    //
    directx_renderable_indexed RenderableTerrain;
    directx_buffer TerrainNormals;
#if 0
    {
        ply_state PlyState;
        b32 Result = LoadPlyFile("..\\data\\monkey.ply", &PlyState);
        assert(Result);
        
        Result = CreateRenderable(&DirectXState, &RenderableTerrain, 
                                  PlyState.Positions, sizeof(v3), PlyState.VertexCount,
                                  PlyState.Indices , sizeof(u16), PlyState.IndexCount,
                                  D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        assert(Result);
    }
#else
    {
        // Source text file is 87 x 61 = 5307
        u32 constexpr w = 61;
        u32 constexpr h = 87;
        u32 constexpr t = w * h;
        
        u32 *Heights = nullptr; // NOTE(Marcus): A bit of a waste, the max number is less than 255... 
        {
            Heights = (u32 *)malloc(t * sizeof(u32)); 
            assert(Heights);
            
            FILE *File;
            errno_t Error = fopen_s(&File, "..\\data\\volcano.txt", "r");
            if (Error)
            {
                printf("Failed to open file, error: %u\n", Error);
                assert(0);
            }
            
            for (u32 Index = 0; Index < t; ++Index)
            {
                int ArgumentsScanned = fscanf_s(File, "%u", &Heights[Index]);
                assert(ArgumentsScanned == 1);
            }
            fclose(File);
        }
        
        u32 VertexCount = t;
        v3 *Vertices = (v3 *)malloc(t * sizeof(v3));
        assert(Vertices);
        
        u32 Index = 0;
        for (u32 z = 0; z < h; ++z)
        {
            for (u32 x = 0; x < w; ++x)
            {
                // The minimum height is 94 and the maximum is 195, we bring it up with 94 so
                // that the new min is 0
                Vertices[Index++] = V3((f32)x, (f32)(Heights[Index] - 94), (f32)z);
            }
        }
        assert(Index == VertexCount);
        
        u16 *Indices = nullptr;
        u32 IndexCount = 6 * (w - 1) * (h - 1);
        size_t Size = IndexCount * sizeof(u16);
        Indices = (u16 *)malloc(Size);
        assert(Indices);
        
        Index = 0;
        for (u32 z = 0; z < (h - 1); ++z)
        {
            for (u32 x = 0; x < (w - 1); ++x)
            {
                //
                // Indices
                Indices[Index] = (u16)((w * z) + x);
                Indices[Index + 1] = (u16)((w * (z + 1)) + x);
                Indices[Index + 2] = (u16)((w * (z + 1)) + x + 1);
                
                Indices[Index + 3] = (u16)((w * z) + x);
                Indices[Index + 4] = (u16)((w * (z + 1)) + x + 1);
                Indices[Index + 5] = (u16)((w * z) + x + 1);
                
                Index += 6;
            }
        }
        assert(Index == IndexCount);
        
        b32 Result = CreateRenderable(&DirectXState, &RenderableTerrain, 
                                      Vertices, sizeof(v3) , VertexCount,
                                      Indices , sizeof(u16), IndexCount,
                                      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        assert(Result);
        
        //
        // Normals
        v3 *Normals = (v3 *)malloc(VertexCount * sizeof(v3));
        assert(Normals);
        
        Index = 0;
        for (u32 z = 0; z < h; ++z)
        {
            for (u32 x = 0; x < w; ++x)
            {
                v3  V[8] = {};
                b32 B[8] = {};
                v3 Vo = Vertices[Index];
                
                if (x > 0) // Left
                {
                    V[0] = Vertices[(w * z) + x - 1];
                    B[0] = true;
                }
                if (x < w - 1) // Right
                {
                    V[1] = Vertices[(w * z) + x + 1];
                    B[1] = true;
                }
                if (z > 0) // Up
                {
                    V[2] = Vertices[(w * (z - 1)) + x];
                    B[2] = true;
                }
                if (z < h - 1) // Down
                {
                    V[3] = Vertices[(w * (z + 1)) + x];
                    B[3] = true;
                }
                if (B[2] && B[0]) // Up left
                {
                    V[4] = Vertices[(w * (z - 1)) + x - 1];
                    B[4] = true;
                }
                if (B[2] && B[1]) // Up right
                {
                    V[5] = Vertices[(w * (z - 1)) + x + 1];
                    B[5] = true;
                }
                if (B[3] && B[1]) // Down right
                {
                    V[6] = Vertices[(w * (z + 1)) + x + 1];
                    B[6] = true;
                }
                if (B[3] && B[0]) // Down left
                {
                    V[7] = Vertices[(w * (z + 1)) + x - 1];
                    B[7] = true;
                }
                
                v3 N = v3_zero;
                for (u32 i = 0; i < 8; ++i)
                {
                    if (B[i])
                    {
                        v3 Va = V[i];
                        v3 Vb = V[(i - 1) % 8];
                        N += Cross(Va - Vo, Vb - Vo);
                    }
                }
                Normals[Index++] = Normalize(N);
            }
        }
        
        
        
        Result = CreateBuffer(&DirectXState, D3D11_BIND_VERTEX_BUFFER,
                              Normals, sizeof(v3), VertexCount,
                              &TerrainNormals);
        assert(Result);
        
        
        free(Heights);
        free(Vertices);
        free(Indices);
        free(Normals);
    }
#endif
    
    
    
    //
    // Init particle system
    particle_system ParticleSystem;
    ParticleSystem.Po = V3(-2.0f, 35.0f, 12.0f);
    ParticleSystem.Force = 25.0f;
    Init(&ParticleSystem, kParticleCount, kThreadCount, kFrameTime);
    
    
    //
    // A renderable for all the particles
    // 
    directx_renderable RenderableParticles;
    {
        b32 Result = CreateRenderable(&DirectXState, &RenderableParticles,
                                      ParticleSystem.P, sizeof(v3), kParticleCount,
                                      D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        assert(Result);
    }
    
    
    
    //
    // DirectWrite
    //
    directwrite_state DirectWriteState;
    InitDirectWrite(&DirectXState, &DirectWriteState);
    
    
    
    //
    // Show and update window
    //
    ShowWindow(AppState.hWnd, nShowCmd);
    UpdateWindow(AppState.hWnd);
    
    
    
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
        
        
        UpdateMouseState(&AppState.MouseState);
        UpdateCamera(&AppState.Camera);
        
        
        //
        // Particle simulation
        LARGE_INTEGER ParticleStartingTime;
        QueryPerformanceCounter(&ParticleStartingTime);
        
        Update(&ParticleSystem);
        
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
        SetShader(&DirectXState, &DirectXState.vBasic);
        SetShader(&DirectXState, &DirectXState.pBasic);
        BeginRendering(&DirectXState);
        
        ShaderConstants.WorldToViewMatrix = GetWorldToViewMatrix(&AppState.Camera);
        ShaderConstants.CameraP = V4(AppState.Camera.P, 1.0f);
        
        // Axis
        {
            SetShader(&DirectXState, &DirectXState.vBasic);
            SetShader(&DirectXState, &DirectXState.pBasic);
            SetConstantBuffer(&DirectXState, &ConstantBuffer, 0);
            
            v4 Colours[] = 
            {
                {1.0f, 0.0f, 0.0f, 1.0f},
                {0.0f, 1.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, 1.0f, 1.0f},
            };
            
            for (u32 Index = 0; Index < 3; ++Index)
            {
                ShaderConstants.Colour = Colours[Index];
                UpdateBuffer(&DirectXState, &ConstantBuffer, &ShaderConstants, nullptr);
                
                RenderRenderable(&DirectXState, &RenderableAxisInWorld[Index]);
            }
        }
        
        
        // Light directions
        {
            ShaderConstants.Colour = V4(1.0f, 1.0f, 0.0f, 1.0f);
            UpdateBuffer(&DirectXState, &ConstantBuffer, &ShaderConstants, nullptr);
            
            SetShader(&DirectXState, &DirectXState.vBasic);
            SetShader(&DirectXState, &DirectXState.pBasic);
            SetConstantBuffer(&DirectXState, &ConstantBuffer, 0);
            
            v3 v[] = 
            {
                {0.0f, 0.0f, 0.0f},
                ShaderConstants.LightP.xyz(),
            };
            UpdateBuffer(&DirectXState, &RenderableLightDirection.VertexBuffer, v, nullptr);
            
            RenderRenderable(&DirectXState, &RenderableLightDirection);
        }
        
        
#if 1
        // Render Terrain
        {
            ShaderConstants.Colour = V4(0.65f, 0.65f, 0.7f, 1.0f);
            ShaderConstants.ObjectToWorldMatrix = M4Translation(V3(-30.5f, -40.0f, -43.5f));
            
            b32 Invertible;
            m4 Mi = M4Inverse(&ShaderConstants.ObjectToWorldMatrix, &Invertible);
            assert(Invertible);
            Mi = M4Transpose(&Mi);
            ShaderConstants.NormalToWorldMatrix = Mi;
            
            UpdateBuffer(&DirectXState, &ConstantBuffer, &ShaderConstants, nullptr);
            
            SetShader(&DirectXState, &DirectXState.vBasicLighting);
            SetShader(&DirectXState, &DirectXState.pBasicLighting);
            SetConstantBuffer(&DirectXState, &ConstantBuffer, 0);
            
            u32 Offset = 0;
            DirectXState.DeviceContext->IASetVertexBuffers(1, 1, &TerrainNormals.Ptr, &RenderableTerrain.Stride, &Offset);
            
            RenderRenderable(&DirectXState, &RenderableTerrain);
        }
#endif
        
        
        // Render particles
        {
            UpdateBuffer(&DirectXState, &RenderableParticles.VertexBuffer, ParticleSystem.P, nullptr);
            
            ShaderConstants.Colour = V4(0.8f, 0.5f, 0.2f, 1.0f);
            ShaderConstants.ObjectToWorldMatrix = m4_identity;
            UpdateBuffer(&DirectXState, &ConstantBuffer, &ShaderConstants, nullptr);
            
            SetShader(&DirectXState, &DirectXState.vPointsToQuads);
            SetShader(&DirectXState, &DirectXState.gPointsToQuads);
            SetShader(&DirectXState, &DirectXState.pPointsToQuads);
            SetConstantBuffer(&DirectXState, &ConstantBuffer, 0);
            
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
#if 0
            f32 ParticleFps = 1000000.0f * ((f32)FrameCount / (f32)ParticleTime.QuadPart);
            printf("fps: %u, particle fps = %f\n", FrameCount, ParticleFps);
#endif
            RunTime.QuadPart = 0;
            FrameCount = 0;
            ParticleTime.QuadPart = 0;
        }
    }
    
    
    
    //
    // Close threads
    //
    ShutDown(&ParticleSystem);
    
    
    
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
    for (u32 Index = 0; Index < 3; ++Index)
    {
        ReleaseRenderable(&RenderableAxisInWorld[Index]);
    }
    ReleaseRenderable(&RenderableLightDirection);
    ReleaseRenderable(&RenderableTerrain);
    ReleaseRenderable(&RenderableParticles);
    ReleaseBuffer(&ConstantBuffer);
    ReleaseBuffer(&TerrainNormals);
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