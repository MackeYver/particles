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


//
// Note:
// - Un-optimized code...
// - Somewhat tested but could do with some more thorough testing.
//


#ifndef Mathematics__h
#define Mathematics__h

#include "types.h"
#include <math.h>

#define Pi32    3.141592653589793f
#define Pi32_2  1.570796326794897f
#define Pi32_4  0.785398163397448f
#define Tau32   6.283185307179586f

//
// Macros and "macro-like" utility functions
inline u8 Max(u8 x, u8 y) {return x > y ? x : y;}
inline u8 Min(u8 x, u8 y) {return x < y ? x : y;}



//
// Scalars
inline f32 Square(f32 const& x) {
    f32 Result = x * x;
    return Result;
}

inline f32 SquareRoot(f32 const& x) {
    f32 Result = sqrtf(x);
    return Result;
}

inline f32 Cos(f32 const& a) {
    f32 Result = cosf(a);
    return Result;
}

inline f32 Sin(f32 const& a) {
    f32 Result = sinf(a);
    return Result;
}

inline f32 Tan(f32 const& a) {
    f32 Result = tanf(a);
    return Result;
}

inline f32 Arctan2(f32 const& x, f32 const& y) {
    f32 Result = (f32)atan2(y, x);
    return Result;
}

inline b32 AlmostEqualRelative(f32 a, f32 b, f32 MaxRelDiff = FLT_EPSILON)
{
    // From: https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
    // Calculate the difference.
    float Diff = fabsf(a - b);
    a = fabsf(a);
    b = fabsf(b);
    float Largest = (b > a) ? b : a;
    
    if (Diff <= Largest * MaxRelDiff)  return true;
    return false;
}




//
// Vectors
//

//
// v2
#define v2_zero V2(0.0f, 0.0f)
#define v2_one  V2(1.0f, 1.0f)

union v2 {
    struct {
        f32 x;
        f32 y;
    };
    f32 E[2];
};

inline v2 V2(f32 x, f32 y) {
    v2 Result;
    Result.x = x;
    Result.y = y;
    return Result;
}

inline v2 V2(u32 x, u32 y) {
    v2 Result;
    Result.x = (f32)x;
    Result.y = (f32)y;
    return Result;
}

inline b32 AlmostEqualRelative(v2 A, v2 B, f32 MaxRelDiff = FLT_EPSILON)
{
    return AlmostEqualRelative(A.x, B.x) && AlmostEqualRelative(A.y, B.y);
}

//
// v2 vs v2
inline v2 operator + (v2 const& A, v2 const& B) {
    v2 Result = V2(A.x + B.x, A.y + B.y);
    return Result;
}

inline v2 operator += (v2& A, v2 const& B) {
    A = V2(A.x + B.x, A.y + B.y);
    return A;
}

inline v2 operator - (v2 const& A, v2 const& B) {
    v2 Result = V2(A.x - B.x, A.y - B.y);
    return Result;
}

inline v2 operator -= (v2& A, v2 const& B) {
    A = V2(A.x - B.x, A.y - B.y);
    return A;
}

inline v2 Hadamard(v2 const& A, v2 const& B) {
    v2 Result = V2(A.x * B.x, A.y * B.y);
    return Result;
}

inline f32 Dot(v2 const& A, v2 const& B) {
    f32 Result = A.x * B.x + A.y * B.y;
    return Result;
}

//
// v2 vs f32
inline v2 operator * (v2 const& A, float const& b) {
    v2 Result = V2(A.x * b, A.y * b);
    return Result;
}

inline v2 operator *= (v2& A, float const& b) {
    A= V2(A.x * b, A.y * b);
    return A;
}

inline v2 operator * (float const& b, v2 const& A) {
    v2 Result = A * b;
    return Result;
}

//
// v2 unary
inline v2 operator - (v2 const& A) {
    v2 Result = V2(-A.x, -A.y);
    return Result;
}

inline v2 Perp(v2 const& A) {
    v2 Result = V2(-A.y, A.x);
    return Result;
}

inline f32 LengthSq(v2 const& A) {
    f32 Result = A.x * A.x + A.y * A.y;
    return Result;
}

inline f32 Length(v2 const& A) {
    f32 Result = SquareRoot(A.x * A.x + A.y * A.y);
    return Result;
}

inline v2 Normalize(v2 const& A) {
    v2 Result = A * (1.0f / Length(A));
    return Result;
}

// Normalize or zero
inline v2 NOZ(v2 const& A) {
    v2 Result = {};
    
    f32 l = LengthSq(A);
    if(l > Square(0.0001f)) {
        Result = A * (1.0f / SquareRoot(l));
    }
    
    return Result;
}



//
// v3
#define v3_zero V3(0.0f, 0.0f, 0.0f)
#define v3_one  V3(1.0f, 1.0f, 1.0f)

union v3 {
    struct {
        f32 x;
        f32 y;
        f32 z;
    };
    f32 E[3];
};

inline v3 V3(f32 x, f32 y, f32 z) {
    v3 Result;
    Result.x = x;
    Result.y = y;
    Result.z = z;
    return Result;
}

//
// v3 vs v3
inline v3 operator + (v3 const& A, v3 const& B) {
    v3 Result = V3(A.x + B.x, A.y + B.y, A.z + B.z);
    return Result;
}

inline v3 operator += (v3& A, v3 const& B) {
    A = V3(A.x + B.x, A.y + B.y, A.z + B.z);
    return A;
}

inline v3 operator - (v3 const& A, v3 const& B) {
    v3 Result = V3(A.x - B.x, A.y - B.y, A.z - B.z);
    return Result;
}

inline v3 Hadamard(v3 const& A, v3 const& B) {
    v3 Result = V3(A.x * B.x, A.y * B.y, A.z * B.z);
    return Result;
}

inline f32 Dot(v3 const& A, v3 const& B) {
    f32 Result = A.x * B.x + A.y * B.y + A.z * B.z;
    return Result;
}

inline v3 Cross(v3 const& A, v3 const& B) {
    v3 Result = V3(A.y * B.z - A.z * B.y, 
                   A.z * B.x - A.x * B.z,
                   A.x * B.y - A.y * B.x);
    return Result;
}

//
// v3 vs f32
inline v3 operator * (v3 const& A, float const& b) {
    v3 Result = V3(A.x * b, A.y * b, A.z * b);
    return Result;
}

inline v3 operator * (float const& b, v3 const& A) {
    v3 Result = A * b;
    return Result;
}

//
// v3 unary
inline v3 operator - (v3 const& A) {
    v3 Result = V3(-A.x, -A.y, -A.z);
    return Result;
}

inline f32 LengthSq(v3 const& A) {
    f32 Result = A.x * A.x + A.y * A.y + A.z * A.z;
    return Result;
}

inline f32 Length(v3 const& A) {
    f32 Result = SquareRoot(A.x * A.x + A.y * A.y + A.z * A.z);
    return Result;
}

inline v3 Normalize(v3 const& A) {
    v3 Result = A * (1.0f / Length(A));
    return Result;
}

// Normalize or zero
inline v3 NOZ(v3 const& A) {
    v3 Result = {};
    
    f32 l = LengthSq(A);
    if(l > Square(0.0001f)) {
        Result = A * (1.0f / SquareRoot(l));
    }
    
    return Result;
}



//
// v4
#define v4_zero V4(0.0f, 0.0f, 0.0f, 0.0f)
#define v4_one  V4(1.0f, 1.0f, 1.0f, 1.0f)

union v4 {
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    f32 E[4];
    v3 xyz() {return V3(x, y, z);}
};

inline v4 V4(f32 x, f32 y, f32 z, f32 w) {
    v4 Result = 
    {
        x, y, z, w
    };
    return Result;
}

inline v4 V4(v3 A, f32 w) {
    v4 Result = 
    {
        A.x, A.y, A.z, w
    };
    return Result;
}

//
// v4 vs v4
inline v4 operator + (v4 const& A, v4 const& B) {
    v4 Result = V4(A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w);
    return Result;
}

inline v4 operator - (v4 const& A, v4 const& B) {
    v4 Result = V4(A.x - B.x, A.y - B.y, A.z - B.z, A.w - B.w);
    return Result;
}

inline v4 Hadamard(v4 const& A, v4 const& B) {
    v4 Result = V4(A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w);
    return Result;
}

inline f32 Dot(v4 const& A, v4 const& B) {
    f32 Result = A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
    return Result;
}


//
// v4 vs f32
inline v4 operator * (v4 const& A, float const& b) {
    v4 Result = V4(A.x * b, A.y * b, A.z * b, A.w * b);
    return Result;
}

inline v4 operator * (float const& b, v4 const& A) {
    v4 Result = A * b;
    return Result;
}

inline v4 operator /= (v4& A, f32 const& b) {
    A = V4(A.x / b, A.y / b, A.z / b, A.w / b);
    return A;
}

//
// v4 unary
inline v4 operator - (v4 const& A) {
    v4 Result = V4(-A.x, -A.y, -A.z, -A.w);
    return Result;
}

inline f32 LengthSq(v4 const& A) {
    f32 Result = A.x * A.x + A.y * A.y + A.z * A.z + A.w * A.w;
    return Result;
}

inline f32 Length(v4 const& A) {
    f32 Result = SquareRoot(A.x * A.x + A.y * A.y + A.z * A.z + A.w * A.w);
    return Result;
}

inline v4 Normalize(v4 const& A) {
    v4 Result = A * (1.0f / Length(A));
    return Result;
}

// Normalize or zero
inline v4 NOZ(v4 const& A) {
    v4 Result = {};
    
    f32 L = LengthSq(A);
    if(L > Square(0.0001f)) {
        Result = A * (1.0f / SquareRoot(L));
    }
    
    return Result;
}



//
// Matrix
//

//
// m3
union m3
{
    v3 V[3];
    f32 E[3][3];
    
    v3 Col(u32 Index) {
        return {E[0][Index], E[1][Index], E[2][Index]};
    }
    
    v3 Row(u32 Index) {
        return V[Index];
    }
};

inline m3 M3(v3 X, v3 Y, v3 Z)
{
    m3 Result =
    {
        X.x , X.y , X.z,
        Y.x , Y.y , Y.z,
        Z.x , Z.y , Z.z,
    };
    return Result;
}

inline m3 M3(v3 X, v3 Y, v3 Z, v3 W)
{
    m3 Result =
    {
        X.x , X.y , X.z,
        Y.x , Y.y , Y.z,
        Z.x , Z.y , Z.z,
    };
    return Result;
}

#define m3_identity {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}


inline f32 M3Determinant(m3 *M)
{
    f32 Result = 0.0f;
    Result += M->E[0][0] * (M->E[1][1] * M->E[2][2] - M->E[1][2] * M->E[2][1]);
    Result += M->E[0][1] * (M->E[1][2] * M->E[2][0] - M->E[1][0] * M->E[2][2]);
    Result += M->E[0][2] * (M->E[1][0] * M->E[2][1] - M->E[1][1] * M->E[2][0]);
    
    return Result;
}



//
// m4
union m4
{
    v4 V[4];
    f32 E[4][4];
    
    v4 Col(u32 Index) {
        return {E[0][Index], E[1][Index], E[2][Index], E[3][Index]};
    }
    
    v4 Row(u32 Index) {
        return V[Index];
    }
};

#define m4_identity {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}

inline m4 M4(v4 X, v4 Y, v4 Z)
{
    m4 Result =
    {
        X.x , X.y , X.z , X.w,
        Y.x , Y.y , Y.z , Y.w,
        Z.x , Z.y , Z.z , Z.w,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return Result;
}

inline m4 M4(v4 X, v4 Y, v4 Z, v4 W)
{
    m4 Result =
    {
        X.x , X.y , X.z , X.w,
        Y.x , Y.y , Y.z , Y.w,
        Z.x , Z.y , Z.z , Z.w,
        W.x , W.y , W.z , W.w,
    };
    return Result;
}

inline m4 M4Translation(v3 T)
{
    m4 Result = m4_identity;
    Result.E[3][0] = T.x;
    Result.E[3][1] = T.y;
    Result.E[3][2] = T.z;
    
    return Result;
}


//
// m4 * m4
//
// . = dot (scalar product)
//
//              0   1   2   3
//            0 A   B   C   D    
//            1 E   F   G   H
//            2 I   J   K   L
//            3 M   N   O   P
//   0 1 2 3
// 0 a b c d  0.0 0.1 0.2 0.3  
// 1 e f g h  1.0 1.1 1.2 1.3
// 2 i j k l  2.0 2.1 2.2 2.3
// 3 m n o p  3.0 3.1 3.2 3.3


inline m4 operator * (m4& A, m4& B)
{
    m4 Result;
    
    for (int R = 0; R < 4; ++R) 
    {
        for (int C = 0; C < 4; ++C) 
        {
            Result.E[R][C] = Dot(A.Row(R), B.Col(C));
        }
    }
    
    return Result;
}


// v4 * m4
//
// . = dot (scalar product)
//
//              0   1   2   3
//            0 A   B   C   D    
//            1 E   F   G   H
//            2 I   J   K   L
//            3 M   N   O   P
//   0 1 2 3
// 0 a b c d  0.0 0.1 0.2 0.3  

inline v4 operator * (v4& V, m4& M)
{
    v4 Result;
    for (int Index = 0; Index < 4; ++Index) 
    {
        Result.E[Index] = Dot(M.Col(Index), V);
    }
    
    return Result;
}

inline v4 operator *= (v4& V, m4& M)
{
    V = V * M;
    return V;
}


inline m4 M4Scale(f32 Sx, f32 Sy, f32 Sz)
{
    v4 X = {  Sx, 0.0f, 0.0f, 0.0f};
    v4 Y = {0.0f,   Sy, 0.0f, 0.0f};
    v4 Z = {0.0f, 0.0f,   Sz, 0.0f};
    v4 W = {0.0f, 0.0f, 0.0f, 1.0f};
    
    m4 Result = M4(X, Y, Z, W);
    return Result;
}


inline m4 M4Translate(f32 dx, f32 dy, f32 dz)
{
    m4 Result = m4_identity;
    Result.V[3] = {dx, dy, dz, 1.0f};
    return Result;
}

inline m4 M4Translate(v3 T)
{
    m4 Result = m4_identity;
    Result.V[3] = V4(T, 1.0f);
    return Result;
}


inline f32 M4Minor(m4 *M, u32 Row, u32 Col)
{
    m3 M3;
    
    u32 Ri = 0;
    u32 Ci = 0;
    
    for (u32 R = 0; R < 4; ++R)
    {
        if (R != Row)
        {
            
            for (u32 C = 0; C < 4; ++C)
            {
                if (C == Col)  continue;
                
                M3.E[Ri][Ci++] = M->E[R][C];
            }
            Ci = 0;
            ++Ri;
        }
    }
    
    return M3Determinant(&M3);
}

inline f32 M4Cofactor(m4 *M, u32 Row, u32 Col)
{
    f32 Factor = powf(-1.0f, (f32)(Row + Col)); // TODO(Marcus): A bit expensive, make it faster!
    f32 Minor = M4Minor(M, Row, Col);
    
    return Factor * Minor;
}


inline f32 M4Determinant(m4 *M)
{
    f32 Result = 0.0f;
    for (u32 C = 0; C < 4; ++C)
    {
        Result += M->E[0][C] * M4Cofactor(M, 0, C);
    }
    
    return Result;
}


inline m4 M4Transpose(m4 *A)
{
    m4 Result;
    for (u32 R = 0; R < 4; ++R)
    {
        for (u32 C = 0; C < 4; ++C)
        {
            Result.E[R][C] = A->E[C][R];
        }
    }
    
    return Result;
}


inline m4 M4Adjoint(m4 *M)
{
    m4 Result = {};
    
    for (u32 Row = 0; Row < 4; ++Row)
    {
        for (u32 Col = 0; Col < 4; ++Col)
        {
            Result.E[Row][Col] = M4Cofactor(M, Row, Col);
        }
    }
    
    Result = M4Transpose(&Result);
    
    return Result;
}

inline m4 M4Inverse(m4 *M, b32 *Invertible = nullptr)
{
    m4 Result = m4_identity;
    Invertible ? *Invertible = true : 0;
    
    f32 Determinant = M4Determinant(M);
    
    if (AlmostEqualRelative(Determinant, 0.0f))
    {
        Invertible ? *Invertible = false : 0;
        return Result;
    }
    
    Result = M4Adjoint(M);
    f32 InvertedDeterminant = 1.0f / Determinant;
    
    for (u32 Row = 0; Row < 4; ++Row)
    {
        for (u32 Col = 0; Col < 4; ++Col)
        {
            Result.E[Row][Col] *= InvertedDeterminant;
        }
    }
    
    return Result;
}


inline m4 M4Perspective(f32 Fovx, f32 AspectRatio, f32 Near, f32 Far)
{
    m4 Result = {};
    
    f32 Zx = 1.0f / Tan(0.5f * Fovx);
    f32 Zy = AspectRatio * Zx;
    
    f32 InvertedDenominator = 1.0f / (Far - Near);
    f32 a = -Far * InvertedDenominator;
    f32 b = -(Near * Far) * InvertedDenominator;
    
    Result.V[0].x = Zx;
    Result.V[1].y = Zy;
    Result.V[2].z = a;
    Result.V[2].w = -1.0f;
    Result.V[3].z = b;
    
    return Result;
}


// @debug
#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif

inline m4 M4LookAt(v3 CameraP, v3 LookAtP) // TODO(Marcus): Sloooow!
{
    v3 Z = Normalize(CameraP - LookAtP);
    v3 X = Normalize(Cross(V3(0.0f, 1.0f, 0.0f), Z));
    v3 Y = Cross(Z, X);
    
    m4 Rotate = {V4(X, 0.0f), V4(Y, 0.0f), V4(Z, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f)};
    m4 Translate = M4Translate(CameraP);
    m4 Result = Rotate * Translate; 
    
    b32 Invertible = false;
    Result = M4Inverse(&Result, &Invertible);
    assert(Invertible);
    
    return Result;
}



#endif /* Mathematics_h */
