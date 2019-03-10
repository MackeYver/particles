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

#include "common.h"


struct vs_output
{
	float4 P : Position;
};

struct ps_input
{
	float4 P : SV_Position;
};



//
// Vertex shader
vs_output vMain(float3 P : Position)
{
	vs_output Output;
	Output.P = mul(float4(P, 1.0f), ObjectToWorld);

	return Output;
}


//
// Geometry shader
[maxvertexcount(6)]
void gMain(point vs_output Pi[1] : Position, inout TriangleStream<ps_input> OutputStream)
{ 
	//
	// Calculate new vertices
	float3 Pw = Pi[0].P.xyz;
	float3 Z = normalize(CameraP.xyz - Pw);
	float3 Y = float3(0.0f, 1.0f, 0.0f);
	float3 X = cross(Y, Z);
	Y = cross(Z, X);

	float k = 1.0f;
	float k_2 = 0.5f;
	float4 P0 = float4(Pw + ((k_2 * X) - (k_2 * Y)), 1.0f);
	float4 P1 = float4(P0.xyz + (k * Y), 1.0f);
	float4 P2 = float4(P1.xyz - (k * X), 1.0f);
	float4 P3 = float4(P2.xyz - (k * Y), 1.0f);

	//
	// Transform to clip
	float4x4 WorldToClip = mul(WorldToView, ViewToClip);
	P0 = mul(P0, WorldToClip);
	P1 = mul(P1, WorldToClip);
	P2 = mul(P2, WorldToClip);
	P3 = mul(P3, WorldToClip);

	//
	// First triangle
	OutputStream.Append((ps_input)P0);
	OutputStream.Append((ps_input)P1);
	OutputStream.Append((ps_input)P2);
	OutputStream.RestartStrip();

	//
	// Second
	OutputStream.Append((ps_input)P0);
	OutputStream.Append((ps_input)P2);
	OutputStream.Append((ps_input)P3);
	OutputStream.RestartStrip();
}


//
// Pixel shader
float4 pMain() : SV_Target
{
	return Colour;
}
