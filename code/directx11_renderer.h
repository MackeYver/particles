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


#ifndef DirectX11Renderer__h
#define DirectX11Renderer__h

#include <d3d11.h>
#include <dxgi.h>
#ifdef DEBUG
#include <dxgidebug.h>
#endif

#include <dwrite.h> // DirectWrite (Dwrite.lib)
#include <d2d1_1.h> // Direct2D    (D2d1_1.lib)

#include "mathematics.h"


//
// Forward declaration
struct directx_state;




//------------------------------------------------------------------------------------------------------
//
// Shaders_
//

//
// Vertex shader
struct vertex_shader
{
    ID3D11VertexShader *Program = nullptr;
    ID3D11InputLayout *InputLayout = nullptr;
};

b32 CreateShader(directx_state *State, const char *PathAndName, 
                 D3D11_INPUT_ELEMENT_DESC *InputElements, u32 ElementCount,
                 vertex_shader *Shader);
void SetShader(directx_state *State, vertex_shader *Shader);
void ReleaseShader(vertex_shader *Shader);
void SetVertexShaderResource(directx_state *State, u32 StartSlot, u32 Count, ID3D11ShaderResourceView * const *Views);


//
// Pixel shader
struct pixel_shader
{
    ID3D11PixelShader *Program = nullptr;
};

b32 CreateShader(directx_state *State, const char *PathAndName, pixel_shader *Shader);
void SetShader(directx_state *State, pixel_shader *Shader);
void ReleaseShader(pixel_shader *Shader);
void SetPixelShaderResource(directx_state *State, u32 StartSlot, u32 Count, ID3D11ShaderResourceView * const *Views);
void SetPixelShaderSampler(directx_state *State, u32 StartSlot, u32 Count, ID3D11SamplerState * const *Samplers);


//
// Geometry shader
struct geometry_shader
{
    ID3D11GeometryShader *Program = nullptr;
};

b32 CreateShader(directx_state *State, const char *PathAndName, geometry_shader *Shader);
void SetShader(directx_state *State, geometry_shader *Shader);
void ReleaseShader(geometry_shader *Shader);



//------------------------------------------------------------------------------------------------------
//
// Buffer_
//
struct directx_buffer
{
    ID3D11Buffer *Ptr = nullptr;
    size_t ElementSize = 0;
    u32 ElementCount = 0;
    D3D11_BIND_FLAG BindFlag;
};

b32 CreateBuffer(directx_state *State, 
                 D3D11_BIND_FLAG BindFlag, 
                 void *Data, size_t ElementSize, u32 ElementCount,
                 directx_buffer *Buffer);

void UpdateBuffer(directx_state *State, directx_buffer *Buffer, void *Data, D3D11_BOX *Box = nullptr);
void ReleaseBuffer(directx_buffer *Buffer);

b32 CreateConstantBuffer(directx_state *State, void *Data, size_t DataSize, directx_buffer *Buffer);
void SetConstantBuffer(directx_state *State, directx_buffer *Buffer, u32 SlotNumber = 0);

b32 CreateVertexBuffer(directx_state *State, void *Data, size_t VertexSize, u32 VertexCount, directx_buffer *Buffer);
b32 CreateIndexBuffer(directx_state *State, void *Data, size_t IndexSize, u32 IndexCount, directx_buffer *Buffer);




//------------------------------------------------------------------------------------------------------
//
// Renderables_
//
struct directx_renderable_indexed
{
    directx_buffer VertexBuffer;
    directx_buffer IndexBuffer;
    u32 Stride;
    D3D11_PRIMITIVE_TOPOLOGY Topology;
};

b32 CreateRenderable(directx_state *State,
                     directx_renderable_indexed *Renderable,
                     void *VertexData, size_t VertexSize, u32 VertexCount,
                     void *IndexData, size_t IndexSize, u32 IndexCount,
                     D3D11_PRIMITIVE_TOPOLOGY Topology);

void RenderRenderable(directx_state *State, directx_renderable_indexed *Renderable, u32 const Offset = 0);

void ReleaseRenderable(directx_renderable_indexed *Renderable);


//
// Non-indexed
struct directx_renderable
{
    directx_buffer VertexBuffer;
    u32 Stride;
    D3D11_PRIMITIVE_TOPOLOGY Topology;
};

b32 CreateRenderable(directx_state *State,
                     directx_renderable *Renderable,
                     void *VertexData, size_t VertexSize, u32 VertexCount,
                     D3D11_PRIMITIVE_TOPOLOGY Topology);

void RenderRenderable(directx_state *State, directx_renderable *Renderable, u32 const Offset = 0);

void ReleaseRenderable(directx_renderable *Renderable);




//------------------------------------------------------------------------------------------------------
//
// Render target_
//
struct render_target
{
    ID3D11Texture2D *Texture = nullptr;
    ID3D11RenderTargetView *View = nullptr;
    ID3D11ShaderResourceView *ShaderView = nullptr;
    
    ID3D11Texture2D *DepthStencilBuffer = nullptr;
    ID3D11DepthStencilView *DepthStencilView = nullptr;
    ID3D11DepthStencilState *DepthStencilState = nullptr;
};

b32 CreateRenderTarget(directx_state *State, u32 Width, u32 Height, render_target *RenderTarget);
void SetRenderTarget(directx_state *State, render_target *RenderTarget);
void SetBackbufferAsRenderTarget(directx_state *State);
void UnbindRenderTarget(directx_state *State);
void ReleaseRenderTarget(render_target *RenderTarget);




//------------------------------------------------------------------------------------------------------
//
// Rasterizer_ state_
//
enum rasterizer_enum
{
    RasterizerState_Default,
    RasterizerState_Solid,
    RasterizerState_Wireframe,
};

struct rasterizer_states
{
    ID3D11RasterizerState *Ptr[3];;
};

b32 CreateRasterizerStates(directx_state *State);
void SetRasterizerState(directx_state *State, rasterizer_enum Enum);




//------------------------------------------------------------------------------------------------------
//
// Direct general state
//
struct directx_state
{
    ID3D11Device *Device = nullptr;
    ID3D11DeviceContext *DeviceContext = nullptr;
    
    IDXGISwapChain *SwapChain = nullptr;
    ID3D11Texture2D *Backbuffer = nullptr;
    ID3D11RenderTargetView *BackbufferView = nullptr;
    
    rasterizer_states RasterizerStates;
    
    directx_renderable FullscreenQuadRenderable;
    
    vertex_shader   vShaderBasic;
    pixel_shader    pShaderBasic;
    
    vertex_shader   vPointsToQuads;
    pixel_shader    pPointsToQuads;
    geometry_shader gPointsToQuads;
    
    vertex_shader vShaderFullscreenTexture;
    pixel_shader  pShaderFullscreenTexture;
    ID3D11SamplerState *Sampler;
    
    render_target RenderTarget;
    
    D3D11_VIEWPORT Viewport;
    
    u32 Width = 0;
    u32 Height = 0;
};

struct directx_config
{
    HWND hWnd;
    u32 Width = 0;
    u32 Height = 0;
};

b32 SetupDirectX(directx_state *State, directx_config *Config);
void ReleaseDirectXState(directx_state *State);

void BeginRendering(directx_state *State);
void EndRendering(directx_state *State);




//------------------------------------------------------------------------------------------------------
//
// DirectWrite
//
struct directwrite_state
{
    ID2D1Device *Device = nullptr;
    ID2D1DeviceContext *DeviceContext = nullptr;
    ID2D1Bitmap1 *RenderTarget = nullptr;
    ID2D1SolidColorBrush *Brush = nullptr;
    IDWriteTextFormat *TextFormat = nullptr;
};

b32 InitDirectWrite(directx_state *DirectXState, directwrite_state *State);
void ReleaseDirectWrite(directwrite_state *State);
void BeginDraw(directwrite_state *State);
HRESULT EndDraw(directwrite_state *State);

void DrawText(directwrite_state *State, WCHAR const *String, v2 P);



#endif // End DirectXRenderer_h