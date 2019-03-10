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

#include "mathematics.h"



//
// Multithreaded stuff
//
struct particle_system;
struct thread_context
{
    particle_system *ParticleSystem = nullptr;
    HANDLE Simulate;
    HANDLE Finished;
    HANDLE ThreadHandle;
    u32 ThreadID;
    u32 StartIndex;
    u32 EndIndex;
};


//
// Particle system
// 
struct particle_system
{
    v3 *P = nullptr;
    v3 *dP = nullptr;
    f32 *Duration = nullptr;
    f32 *Elapsed = nullptr;
    
    thread_context *ThreadContext;
    
    v3 Po = v3_zero;
    v3 ddPg = V3(0.0f, -9.8f, 0.0f); // Gravity acceleration
    
    u32 ThreadCount;
    u32 ParticleCount;
    
    f32 dt;
    f32 Force = 10.0f;
    b32 IsSimulating = true;
};

void Init(particle_system *ParticleSystem, u32 ParticleCount, u32 ThreadCount, f32 dt);
void Update(particle_system *ParticleSystem);
void Pause(particle_system *ParticleSystem);
void ShutDown(particle_system *ParticleSystem);
