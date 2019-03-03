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


struct VSInput
{
	float2 P : POSITION0;
	float2 T : TEXCOORD0;
};

struct PSInput
{	
	float4 P : SV_Position;
	float2 T : TEXCOORD0;
};

Texture2D    gTexture : register(t0);
SamplerState gSampler : register(s0);


//
// Vertex shader

PSInput vMain(VSInput In)
{
	PSInput Result;
	Result.P = float4(In.P, 0.0f, 1.0f);
	Result.T = In.T;

	return Result;
}


//
// Pixel shader

float4 pMain(PSInput In) : SV_TARGET
{
	return gTexture.Sample(gSampler, In.T);
}


#if 0
cbuffer ConstantBuffer : register(b0)
{
    float4x4 TransformMatrix;
	float3 Colour;
	float Z;
};

struct VSInput
{
	float2 P : POSITION0;
//	float3 Normal : NORMAL0;
	float2 T : TEXCOORD0;
};

struct PSInput
{	
	float4 P : SV_Position;
//	float3 Normal : NORMAL;
	float2 T : TEXCOORD0;
//	float4 WorldPos : POSITION;
};

Texture2D    gTexture : register(t0);
SamplerState gSampler : register(s0);


//
// Vertex shader

PSInput vMain(VSInput In)
{
	PSInput Result;
	//Result.P = mul(TransformMatrix, float4(In.P, 1.0f));
	Result.P = float4(In.P, 0.0f, 1.0f);
	Result.T = In.T;

	return Result;
}


//
// Pixel shader

float4 pMain(PSInput In) : SV_TARGET
{
	return gTexture.Sample(gSampler, In.T) * float4(Colour, 1.0);
}
#endif