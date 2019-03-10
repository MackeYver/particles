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

#include "particle_system.h"
#include <stdlib.h>
#include <process.h>

#if 0
#include <stdio.h>
#define UNUSED_VAR(x)
#else
#define printf(...)
#define UNUSED_VAR(x) x;
#endif



//
// Particle system
// 
unsigned int __stdcall ParticleUpdate(void* Data)
{
    thread_context *Context = (thread_context *)Data;
    particle_system *ParticleSystem = Context->ParticleSystem;
    u32 ThreadID = GetCurrentThreadId();
    UNUSED_VAR(ThreadID);
    
    printf("Thread# %u handles %u <= Index < %u\n", ThreadID, Context->StartIndex, Context->EndIndex);
    
    f32 dt = ParticleSystem->dt;
    u32 ParticleCount = ParticleSystem->ParticleCount;
    
    while (ParticleSystem->IsSimulating)
    {
        DWORD WaitResult = WaitForSingleObject(Context->Simulate, INFINITE);
        switch (WaitResult) 
        {
            case WAIT_OBJECT_0: 
            {
                f32 Radius = 0.5f;
                f32 Theta = Tau32 / (f32)ParticleCount;
                f32 Angle = (f32)Context->StartIndex * Theta;
                for (u32 Index = Context->StartIndex; Index < Context->EndIndex; ++Index)
                {
                    v3 *P = &ParticleSystem->P[Index];
                    v3 *dP = &ParticleSystem->dP[Index];
                    f32 *Duration = &ParticleSystem->Duration[Index];
                    f32 *Elapsed = &ParticleSystem->Elapsed[Index];
                    
                    *Elapsed += dt;
                    if (*Elapsed > *Duration)
                    {
                        *Elapsed = 0;
                        *P = ParticleSystem->Po;
                        
                        Angle += Theta;
                        if (Angle > Tau32)
                        {
                            Angle -= Tau32;
                        }
                        
                        v3 F = V3(Radius * Cos(Angle), 1.0f, -Radius * Sin(Angle));
                        F = Normalize(F);
                        *dP = ParticleSystem->Force * F;
                    }
                    else
                    {
                        *P  += *dP * dt;
                        *dP += ParticleSystem->ddPg * dt;
                    }
                }
                
                printf("Thread# %u is done!\n", ThreadID);
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


void Init(particle_system *ParticleSystem, u32 ParticleCount, u32 ThreadCount, f32 dt)
{
    ParticleSystem->P = (v3 *)calloc(ParticleCount, sizeof(v3));
    assert(ParticleSystem->P);
    
    ParticleSystem->dP = (v3 *)calloc(ParticleCount, sizeof(v3));
    assert(ParticleSystem->dP);
    
    ParticleSystem->Duration = (f32 *)calloc(ParticleCount, sizeof(f32));
    assert(ParticleSystem->Duration);
    
    ParticleSystem->Elapsed = (f32 *)calloc(ParticleCount, sizeof(f32));
    assert(ParticleSystem->Elapsed);
    
    f32 Radius = 0.5f;
    f32 Angle = 0.0f;
    f32 Theta = Tau32 / (f32)ParticleCount;
    for (u32 Index = 0; Index < ParticleCount; ++Index)
    {
        ParticleSystem->P[Index] = ParticleSystem->Po;
        ParticleSystem->Duration[Index] = 5.0f;
        ParticleSystem->Elapsed[Index] = 0.0f;
        
        Angle += Theta;
        if (Angle > Tau32)
        {
            Angle = Tau32 - Angle;
        }
        
        v3 F = V3(Radius * Cos(Angle), 1.0f, Radius * -Sin(Angle));
        F = Normalize(F);
        ParticleSystem->dP[Index]= ParticleSystem->Force * F;
    }
    
    ParticleSystem->dt = dt;
    ParticleSystem->ParticleCount = ParticleCount;
    
    
    //
    // Multithreaded stuff
    ThreadCount = ThreadCount > 0 ? ThreadCount : 1;
    ParticleSystem->ThreadCount = ThreadCount;
    ParticleSystem->ThreadContext = (thread_context *)calloc(ParticleSystem->ThreadCount, sizeof(thread_context));
    assert(ParticleSystem->ThreadContext);
    
    thread_context *ThreadContext = ParticleSystem->ThreadContext;
    
    u32 AllocatedIndices = 0;
    u32 ParticlesPerThread = ParticleCount / ThreadCount;
    
    for (u32 Index = 0; Index < ThreadCount; ++Index)
    {
        ThreadContext[Index].ParticleSystem = ParticleSystem;
        
        if (Index < ThreadCount - 1)
        {
            ThreadContext[Index].StartIndex = AllocatedIndices;
            ThreadContext[Index].EndIndex = AllocatedIndices + ParticlesPerThread;
            AllocatedIndices += ParticlesPerThread;
        }
        else
        {
            ThreadContext[Index].StartIndex = AllocatedIndices;
            ThreadContext[Index].EndIndex = ParticleCount;
        }
        
        ThreadContext[Index].Simulate = CreateEventA(nullptr, true, false, nullptr);
        if(ThreadContext[Index].Simulate == nullptr)
        {
            printf("CreateEvent, Simulate, error: %d\n", GetLastError());
            return;
        }
        
        ThreadContext[Index].Finished = CreateEventA(nullptr, true, false, nullptr);
        if(ThreadContext[Index].Finished == nullptr)
        {
            printf("CreateEvent, Finished, error: %d\n", GetLastError());
            return;
        }
        
        ThreadContext[Index].ThreadHandle = (HANDLE)_beginthreadex(0, 
                                                                   0, 
                                                                   (_beginthreadex_proc_type)&ParticleUpdate, 
                                                                   (void *)&ThreadContext[Index], 
                                                                   0,
                                                                   &ThreadContext[Index].ThreadID);
        if(ThreadContext[Index].ThreadHandle == nullptr)
        {
            printf("_beginthreadex error: %d\n", GetLastError());
            return;
        }
        else
        {
            printf("Created thread %u->\n", ThreadContext[Index].ThreadID);
        }
    }
}



void Update(particle_system *ParticleSystem)
{
    thread_context *ThreadContext = ParticleSystem->ThreadContext;
    for (u32 Index = 0; Index < ParticleSystem->ThreadCount; ++Index)
    {
        b32 Result = SetEvent(ThreadContext[Index].Simulate);
        if(!Result)
        {
            printf("SetEvent, thread %u error: %d\n", ThreadContext[Index].ThreadID, GetLastError());
        }
    }
    
    //
    // Wait until all threads are finished
    for (u32 Index = 0; Index < ParticleSystem->ThreadCount; ++Index)
    {
        WaitForSingleObject(ThreadContext[Index].Finished, INFINITE);
        ResetEvent(ThreadContext[Index].Finished);
    }
}


void ShutDown(particle_system *ParticleSystem)
{
    thread_context *ThreadContext = ParticleSystem->ThreadContext;
    
    ParticleSystem->IsSimulating = false;
    for (u32 Index = 0; Index < ParticleSystem->ThreadCount; ++Index)
    {
        WaitForSingleObject(ThreadContext[Index].ThreadHandle, INFINITE);
    }
    
    for (u32 Index = 0; Index < ParticleSystem->ThreadCount; ++Index)
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
    
    //
    // Free memory
    // 
    if (ParticleSystem->P)
    {
        free(ParticleSystem->P);
    }
    
    if (ParticleSystem->dP)
    {
        free(ParticleSystem->dP);
    }
    
    if (ParticleSystem->Duration)
    {
        free(ParticleSystem->Duration);
    }
    
    if (ParticleSystem->Elapsed)
    {
        free(ParticleSystem->Elapsed);
    }
}