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


cbuffer ConstantBuffer : register(b0)
{
	float4x4 ViewToClip;
	float4x4 WorldToView;
	float4x4 ObjectToWorld;
	float4 CameraP;
	float4 Colour;
};


struct vs_output
{
	float4 P : SV_Position;
};

struct ps_input
{
	float4 P : SV_Position;
};



//
// Geometry shader
[maxvertexcount(1)]
void gMain(point vs_output Pi[1] : Position, inout PointStream<ps_input> OutputStream)
{   
//	float3 Z = CameraP - Pi[0].P;

	ps_input Result;
	Result.P = Pi[0].P;
	OutputStream.Append(Result);	
    
	OutputStream.RestartStrip();
}
