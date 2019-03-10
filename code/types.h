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

#ifndef Types__h
#define Types__h

#include <stdint.h>
#include <float.h>



//
// Typedefs
//
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int32_t  s32;
typedef int64_t  s64;

typedef float f32;

f32 constexpr f32Max = FLT_MAX;
f32 constexpr f32Min = FLT_MIN;
u32 constexpr u32Max = UINT32_MAX;

typedef u32 b32;




//
// string
//
struct string
{
    uintptr_t Count;
    u8 *Data;
};

static u8 *Advance(string *String, uintptr_t Count)
{
    u8 *Result = nullptr;
    
    if(String->Count >= Count)
    {
        Result = String->Data;
        String->Data  += Count;
        String->Count -= Count;
    }
    else
    {
        String->Data += String->Count;
        String->Count = 0;
    }
    
    return Result;
}

static b32 IsEndOfLine(char C)
{
    b32 Result = ((C == '\n') ||
                  (C == '\r'));
    
    return Result;
}

static b32 IsWhiteSpace(char C)
{
    b32 Result = ((C == ' ') ||
                  (C == '\t') ||
                  (C == '\v') ||
                  (C == '\f') ||
                  IsEndOfLine(C));
    
    return Result;
}

static b32 IsAlpha(char C)
{
    return ((C >= 'a' && C <= 'z') ||
            (C >= 'A' && C <= 'Z'));
}

static b32 IsNumber(char C)
{
    return (C >= '0' && C <= '9');
}

static b32 IsNumberSigned(char C, char D)
{
    b32 Sign = (C == '+' || C == '-');
    b32 Digit = (D >= '0' && D <= '9');
    b32 Result = Sign && Digit;
    
    return Result;;
}

static b32 StringsAreEqual(string A, char *B)
{
    if (*B)
    {
        char *String = B;
        for (u32 Index = 0; Index < A.Count; ++Index, ++String)
        {
            if ((*String == 0) || (A.Data[Index] != *String))
            {
                return false;
            }
        }
    }
    else
    {
        return (A.Count == 0);
    }
    
    return true;
}


#endif