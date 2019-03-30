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

struct vs_in
{
	float3 P : POSITION;
	float3 N : NORMAL;
};

struct vs_out
{
	float4 Pc : SV_Position;
	float4 Nw : NORMAL;
	float4 Pw : POSITION;
	float4 Co : COL0;
};


//
// Vertex shader
vs_out vMain(vs_in In)
{
	float4x4 ObjectToView = mul(ObjectToWorld, WorldToView);
	float4x4 ObjectToClip = mul(ObjectToView, ViewToClip);

	vs_out Result;
	Result.Pc = mul(float4(In.P, 1.0f), ObjectToClip);
	Result.Pw = mul(float4(In.P, 1.0f), ObjectToWorld);
	Result.Nw = mul(float4(In.N, 0.0f), NormalToWorld);

	float3 Gray  = float3(0.55f, 0.55f, 0.5f);
	float3 Black = float3(0.25f, 0.25f, 0.25f);
	float f = (In.P.y - 94.0f) / 101.0f;
	Result.Co = float4(lerp(Gray, Black, f), 1.0f);

	return Result;
}


//
// Pixel shader
float4 pMain(vs_out In) : SV_Target
{
	//
	// All light calculations are done in world space
	float3 L = normalize(LightP.xyz - normalize(In.Pw.xyz));
	float3 N = normalize(In.Nw.xyz);
	float3 ResultantColour = (Colour.rgb * In.Co.rgb);


	//
	// Diffuse
	float DiffuseFactor = 0.5f;
	float DiffuseIntensity = DiffuseFactor * saturate(dot(N, L));
	float3 DiffuseLight = ResultantColour * DiffuseIntensity;


	//
	// Ambient
	float AmbientFactor = 0.5f;
	float AmbientIntensity = 0.0f;
#if 1
	L = float3(1.0f, 0.0, 0.0f);
	AmbientIntensity += AmbientFactor * saturate(dot(N, L));

	L = float3(-1.0f, 0.0, 0.0f);
	AmbientIntensity += AmbientFactor * saturate(dot(N, L));

	L = float3(0.0f, 1.0, 0.0f);
	AmbientIntensity += AmbientFactor * saturate(dot(N, L));

	L = float3(0.0f, -1.0, 0.0f);
	AmbientIntensity += AmbientFactor * saturate(dot(N, L));

	L = float3(0.0f, 0.0, 1.0f);
	AmbientIntensity += AmbientFactor * saturate(dot(N, L));		

	L = float3(0.0f, 0.0, -1.0f);
	AmbientIntensity += AmbientFactor * saturate(dot(N, L));		
#endif

	float3 AmbientLight = In.Co.rgb * AmbientIntensity;


	//
	// Result
#if 1
	return float4(AmbientLight + DiffuseLight, 1.0f);
#else
//	return float4(In.Nw.xyz, 1.0f);
	return float4(N, 1.0f);
#endif
}