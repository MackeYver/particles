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
// TODO
// - Add better error handling
//


#include "directX11_renderer.h"

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif

#include <stdio.h>



//------------------------------------------------------------------------------------------------------
//
// Shaders_
//


//
// Vertex shader
// TODO(Marcus): Pull out the input layout as well, currently this only allows one layout...
b32 CreateShader(directx_state *State, const char *PathAndName, 
                 D3D11_INPUT_ELEMENT_DESC *InputElements, u32 ElementCount,
                 vertex_shader *Shader)
{
    assert(State);
    assert(State->Device);
    assert(PathAndName);
    assert(Shader);
    
    ID3D11Device *Device = State->Device;
    
    // Open file
    FILE *ShaderFile;
    fopen_s(&ShaderFile, PathAndName, "rb");
    assert(ShaderFile);
    
    // Get size
    fseek(ShaderFile, 0L, SEEK_END);
    size_t FileSize = ftell(ShaderFile);
    rewind(ShaderFile);
    
    u8 *Bytes = (u8 *)malloc(FileSize);
    assert(Bytes);
    
    // Read file
    size_t BytesRead = fread_s(Bytes, FileSize, 1, FileSize, ShaderFile);
    fclose(ShaderFile);
    
    // Create shader
    HRESULT Result = Device->CreateVertexShader(Bytes,
                                                BytesRead,
                                                nullptr,
                                                &Shader->Program);
    if (FAILED(Result)) {
        if (Bytes)  free(Bytes);
        printf("Failed to create vertex shader from file!\n");
        return false;
    }
    
    
    //
    // Input layout
    Result = Device->CreateInputLayout(InputElements, ElementCount, Bytes, BytesRead, &Shader->InputLayout);
    
    if (Bytes)  free(Bytes);
    
    if (FAILED(Result)) {
        printf("Failed to create element layout!\n");
        return false;
    }
    
    return true;
}

void ReleaseShader(vertex_shader *Shader)
{
    if (Shader)
    {
        Shader->InputLayout ? Shader->InputLayout->Release() : 0;
        Shader->Program ? Shader->Program->Release() : 0;
    }
}

void SetShader(directx_state *State, vertex_shader *Shader)
{
    assert(State && State->DeviceContext);
    assert(Shader);
    assert(Shader->Program);
    assert(Shader->InputLayout);
    
    ID3D11DeviceContext *DeviceContext = State->DeviceContext;
    DeviceContext->VSSetShader(Shader->Program, nullptr, 0);
    DeviceContext->IASetInputLayout(Shader->InputLayout);
}

void SetVertexShaderResource(directx_state *State, u32 StartSlot, u32 Count, ID3D11ShaderResourceView * const *Views)
{
    assert(State && State->DeviceContext);
    State->DeviceContext->VSSetShaderResources(StartSlot, Count, Views);
}


//
// Pixel shader
b32 CreateShader(directx_state *State, const char *PathAndName, pixel_shader *Shader)
{
    assert(State);
    assert(State->Device);
    assert(PathAndName);
    assert(Shader);
    
    // Open file
    FILE *ShaderFile;
    fopen_s(&ShaderFile, PathAndName, "rb");
    assert(ShaderFile);
    
    // Get size
    fseek(ShaderFile, 0L, SEEK_END);
    size_t FileSize = ftell(ShaderFile);
    rewind(ShaderFile);
    
    u8 *Bytes = (u8 *)malloc(FileSize);
    assert(Bytes);
    
    // Read file
    size_t BytesRead = fread_s(Bytes, FileSize, 1, FileSize, ShaderFile);
    fclose(ShaderFile);
    
    // Create shader
    HRESULT Result = State->Device->CreatePixelShader(Bytes,
                                                      BytesRead,
                                                      nullptr,
                                                      &Shader->Program);
    
    if (Bytes)  free(Bytes);
    
    if (FAILED(Result)) {
        printf("Failed to create pixel shader from file!\n");
        return false;
    }
    
    return true;
}

void ReleaseShader(pixel_shader *Shader)
{
    
    if (Shader)
    {
        Shader->Program ? Shader->Program->Release() : 0;
    }
}

void SetShader(directx_state *State, pixel_shader *Shader)
{
    assert(State && State->DeviceContext);
    assert(Shader);
    assert(Shader->Program);
    
    State->DeviceContext->PSSetShader(Shader->Program, nullptr, 0);
}

void SetPixelShaderResource(directx_state *State, u32 StartSlot, u32 Count, ID3D11ShaderResourceView * const *Views)
{
    assert(State && State->DeviceContext);
    State->DeviceContext->PSSetShaderResources(StartSlot, Count, Views);
}

void SetPixelShaderSampler(directx_state *State, u32 StartSlot, u32 Count, ID3D11SamplerState * const *Samplers)
{
    assert(State && State->DeviceContext);
    State->DeviceContext->PSSetSamplers(StartSlot, Count, Samplers);
}


//
// Geomtry shader
b32 CreateShader(directx_state *State, const char *PathAndName, geometry_shader *Shader)
{
    assert(State);
    assert(State->Device);
    assert(PathAndName);
    assert(Shader);
    
    // Open file
    FILE *ShaderFile;
    fopen_s(&ShaderFile, PathAndName, "rb");
    assert(ShaderFile);
    
    // Get size
    fseek(ShaderFile, 0L, SEEK_END);
    size_t FileSize = ftell(ShaderFile);
    rewind(ShaderFile);
    
    u8 *Bytes = (u8 *)malloc(FileSize);
    assert(Bytes);
    
    // Read file
    size_t BytesRead = fread_s(Bytes, FileSize, 1, FileSize, ShaderFile);
    fclose(ShaderFile);
    
    // Create shader
    HRESULT Result = State->Device->CreateGeometryShader(Bytes,
                                                         BytesRead,
                                                         nullptr,
                                                         &Shader->Program);
    
    if (Bytes)
    {
        free(Bytes);
    }
    
    if (FAILED(Result)) {
        printf("Failed to create geometry shader from file!\n");
        return false;
    }
    
    return true;
}

void ReleaseShader(geometry_shader *Shader)
{
    
    if (Shader)
    {
        Shader->Program ? Shader->Program->Release() : 0;
    }
}

void SetShader(directx_state *State, geometry_shader *Shader)
{
    assert(State && State->DeviceContext);
    
    State->DeviceContext->GSSetShader(Shader ? Shader->Program : nullptr, nullptr, 0);
}



//------------------------------------------------------------------------------------------------------
//
// Buffers_
//
b32 CreateBuffer(directx_state *State, 
                 D3D11_BIND_FLAG BindFlag, 
                 void *Data, size_t ElementSize, u32 ElementCount,
                 directx_buffer *Buffer)
{
    assert(State);
    assert(State->Device);
    assert(ElementSize > 0 && ElementCount > 0);
    assert(Buffer);
    
    Buffer->BindFlag = BindFlag;
    Buffer->ElementSize = ElementSize;
    Buffer->ElementCount = ElementCount;
    
    size_t Size = ElementCount * ElementSize;
    if (Size % 16 != 0)
    {
        Size = 16 * (u32)ceilf((f32)Size / 16.0f);
    }
    
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.ByteWidth = Size;             // size of the buffer
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;  // only usable by the GPU
    BufferDesc.BindFlags = Buffer->BindFlag;
    BufferDesc.CPUAccessFlags = 0;           // No CPU access to the buffer
    BufferDesc.MiscFlags = 0;                // No other option
    BufferDesc.StructureByteStride = ElementSize;
    
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = Data;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    
    HRESULT Result = State->Device->CreateBuffer(&BufferDesc, Data ? &InitData : nullptr, &Buffer->Ptr);
    if (FAILED(Result)) 
    {
        printf("Failed to create the buffer\n");
        return false;
    }
    
    
    return true;
}

void UpdateBuffer(directx_state *State, directx_buffer *Buffer, void *Data, D3D11_BOX *Box)
{
    // TODO(Marcus): In some case we'll probably want to use mappable buffers. Find out in which cases we
    //               want that and in which we don't.
    assert(State);
    assert(State->DeviceContext);
    assert(Buffer && Buffer->Ptr);
    assert(Data);
    
    State->DeviceContext->UpdateSubresource(Buffer->Ptr, // Resource to update
                                            0,           // Subresource index
                                            Box,         // Destination box, nullptr for the entire buffer 
                                            Data,        // Pointer to the data                
                                            0,           // Row pitch (only for textures?) 
                                            0);          // Depth pitch (only for textures?)
}

void ReleaseBuffer(directx_buffer *Buffer)
{
    if (Buffer)
    {
        Buffer->Ptr ? Buffer->Ptr->Release() : 0;
        Buffer->ElementCount = 0;
        Buffer->ElementSize = 0;
    }
}



//
// Constant buffer
b32 CreateConstantBuffer(directx_state *State, void *Data, size_t DataSize, directx_buffer *Buffer)
{
    b32 Result = CreateBuffer(State, D3D11_BIND_CONSTANT_BUFFER, Data, DataSize, 1, Buffer);
    assert(Result);
    
    return Result;
}

void SetConstantBuffer(directx_state *State, directx_buffer *Buffer, u32 SlotNumber)
{
    assert(State);
    assert(State->DeviceContext);
    assert(Buffer);
    assert(Buffer->BindFlag == D3D11_BIND_CONSTANT_BUFFER);
    
    State->DeviceContext->VSSetConstantBuffers(SlotNumber, 1, &Buffer->Ptr);
    State->DeviceContext->PSSetConstantBuffers(SlotNumber, 1, &Buffer->Ptr);
    State->DeviceContext->GSSetConstantBuffers(SlotNumber, 1, &Buffer->Ptr);
}



//
// Vertex and index buffers
b32 CreateVertexBuffer(directx_state *State, void *Data, size_t VertexSize, u32 VertexCount, directx_buffer *Buffer)
{
    b32 Result = CreateBuffer(State, D3D11_BIND_VERTEX_BUFFER, Data, VertexSize, VertexCount, Buffer);
    assert(Result);
    
    return Result;
}

b32 CreateIndexBuffer(directx_state *State, void *Data, size_t IndexSize, u32 IndexCount, directx_buffer *Buffer)
{
    b32 Result = CreateBuffer(State, D3D11_BIND_INDEX_BUFFER, Data, IndexSize, IndexCount, Buffer);
    assert(Result);
    
    return Result;
}




//------------------------------------------------------------------------------------------------------
//
// Renderables_
// 

//
// Indexed
b32 CreateRenderable(directx_state *State,
                     directx_renderable_indexed *Renderable,
                     void *VertexData, size_t VertexSize, u32 VertexCount,
                     void *IndexData, size_t IndexSize, u32 IndexCount,
                     D3D11_PRIMITIVE_TOPOLOGY Topology)
{
    //
    // Vertexbuffer
    b32 Result = CreateVertexBuffer(State, VertexData, VertexSize, VertexCount, &Renderable->VertexBuffer);
    assert(Result);
    
    Result = CreateIndexBuffer(State, IndexData, IndexSize, IndexCount, &Renderable->IndexBuffer);
    assert(Result);
    
    Renderable->Topology = Topology;
    Renderable->Stride = VertexSize;
    
    return true;
}


void RenderRenderable(directx_state *State, directx_renderable_indexed *Renderable, u32 const Offset)
{
    assert(State);
    assert(State->DeviceContext);
    assert(Renderable);
    assert(Renderable->VertexBuffer.Ptr);
    assert(Renderable->IndexBuffer.Ptr && Renderable->IndexBuffer.ElementCount > 0);
    
    ID3D11DeviceContext *DeviceContext = State->DeviceContext;
    DeviceContext->IASetVertexBuffers(0, 1, &Renderable->VertexBuffer.Ptr, &Renderable->Stride, &Offset);
    DeviceContext->IASetIndexBuffer(Renderable->IndexBuffer.Ptr, DXGI_FORMAT_R16_UINT, 0);
    DeviceContext->IASetPrimitiveTopology(Renderable->Topology);
    DeviceContext->DrawIndexed(Renderable->IndexBuffer.ElementCount, 0, 0);
}



void ReleaseRenderable(directx_renderable_indexed *Renderable)
{
    if (Renderable)
    {
        ReleaseBuffer(&Renderable->VertexBuffer);
        ReleaseBuffer(&Renderable->IndexBuffer);
    }
}


//
// Non-indexed
b32 CreateRenderable(directx_state *State,
                     directx_renderable *Renderable,
                     void *VertexData, size_t VertexSize, u32 VertexCount,
                     D3D11_PRIMITIVE_TOPOLOGY Topology)
{
    assert(State);
    assert(State->Device);
    assert(Renderable);
    assert(VertexData && VertexCount > 0);
    
    ID3D11Device *Device = State->Device;
    
    //
    // Vertexbuffer
    Renderable->VertexBuffer.ElementCount = VertexCount;
    Renderable->VertexBuffer.ElementSize = VertexSize;
    
    size_t Size = VertexCount * VertexSize;
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.ByteWidth = Size;                     // size of the buffer
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;          // only usable by the GPU
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // use in vertex shader
    BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
    BufferDesc.MiscFlags = 0;                        // No other option
    BufferDesc.StructureByteStride = VertexSize;
    
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = VertexData;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    
    HRESULT Result = Device->CreateBuffer(&BufferDesc, &InitData, &Renderable->VertexBuffer.Ptr);
    if (FAILED(Result)) 
    {
        printf("Failed to create the Vertexbuffer\n");
        return false;
    }
    
    Renderable->Topology = Topology;
    Renderable->Stride = VertexSize;
    
    return true;
}



void RenderRenderable(directx_state *State, directx_renderable *Renderable, u32 const Offset)
{
    assert(State);
    assert(State->DeviceContext);
    assert(Renderable);
    assert(Renderable->VertexBuffer.Ptr);
    assert(Renderable->VertexBuffer.ElementCount > 0);
    
    ID3D11DeviceContext *DeviceContext = State->DeviceContext;
    DeviceContext->IASetVertexBuffers(0, 1, &Renderable->VertexBuffer.Ptr, &Renderable->Stride, &Offset);
    DeviceContext->IASetPrimitiveTopology(Renderable->Topology);
    DeviceContext->Draw(Renderable->VertexBuffer.ElementCount, 0);
}



void ReleaseRenderable(directx_renderable *Renderable)
{
    if (Renderable)
    {
        ReleaseBuffer(&Renderable->VertexBuffer);
    }
}




//------------------------------------------------------------------------------------------------------
//
// Render target_
// 
b32 CreateRenderTarget(directx_state *State, u32 Width, u32 Height, render_target *RenderTarget)
{
    assert(State);
    assert(State->Device);
    assert(State->SwapChain);
    assert(State->Backbuffer);
    assert(State->DeviceContext);
    assert(RenderTarget);
    
    ID3D11Device *Device = State->Device;
    
    
    //
    // Texture
    D3D11_TEXTURE2D_DESC TexDesc;
    ZeroMemory(&TexDesc, sizeof(D3D11_TEXTURE2D_DESC));
    
    TexDesc.Width = Width;
    TexDesc.Height = Height;
    TexDesc.MipLevels = 1;
    TexDesc.ArraySize = 1;
    TexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    TexDesc.SampleDesc.Count = 1;
    TexDesc.SampleDesc.Quality = 0;
    TexDesc.Usage = D3D11_USAGE_DEFAULT;
    TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    TexDesc.CPUAccessFlags = 0;
    TexDesc.MiscFlags = 0;
    
    HRESULT Result = Device->CreateTexture2D(&TexDesc, nullptr, &RenderTarget->Texture);
    if (FAILED(Result)) 
    {
        printf("Failed to create the texture for the render_target.\n");
        return false;
    }
    
    
    //
    // View
    D3D11_RENDER_TARGET_VIEW_DESC ViewDesc;
    ViewDesc.Format = TexDesc.Format;
    ViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    ViewDesc.Texture2D.MipSlice = 0;
    
    Result = Device->CreateRenderTargetView(RenderTarget->Texture, &ViewDesc, &RenderTarget->View);
    if (FAILED(Result)) 
    {
        printf("Failed to create the view for the render_target.\n");
        return false;
    }
    
    
    //
    // Shader view
    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderViewDesc;
    ShaderViewDesc.Format = TexDesc.Format;
    ShaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ShaderViewDesc.Texture2D.MostDetailedMip = 0;
    ShaderViewDesc.Texture2D.MipLevels = 1;
    
    Result = Device->CreateShaderResourceView(RenderTarget->Texture, &ShaderViewDesc, &RenderTarget->ShaderView);
    if (FAILED(Result)) 
    {
        printf("Failed to create the shader's view for the render_target.\n");
        return false;
    }
    
    
    
    //
    // Create the texture for the depthstencilbuffer
    ZeroMemory(&TexDesc, sizeof(D3D11_TEXTURE2D_DESC));
    RenderTarget->Texture->GetDesc(&TexDesc);
    TexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    TexDesc.Usage = D3D11_USAGE_DEFAULT;
    TexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    TexDesc.MiscFlags = 0;
    
    Result = Device->CreateTexture2D(&TexDesc, nullptr, &RenderTarget->DepthStencilBuffer);
    if (FAILED(Result)) 
    {
        printf("Failed to create the depth-/stencil-buffer!\n");
        return false;
    }
    
    
    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
    DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DepthStencilViewDesc.Texture2D.MipSlice = 0;
    DepthStencilViewDesc.Flags = 0;
    
    Result = Device->CreateDepthStencilView(RenderTarget->DepthStencilBuffer, 
                                            &DepthStencilViewDesc, 
                                            &RenderTarget->DepthStencilView);
    if (FAILED(Result)) 
    {
        printf("Failed to create the depth-/stencil-view!\n");
        return false;
    }
    
    
    //
    // Depth-/stencil-state
    D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
    DepthStencilDesc.DepthEnable = true;
    DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    DepthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;
    //DepthStencilDesc.DepthFunc      = D3D11_COMPARISON_ALWAYS;
    
    // Stencil test parameters
    DepthStencilDesc.StencilEnable = false;
    DepthStencilDesc.StencilReadMask = 0xFF;
    DepthStencilDesc.StencilWriteMask = 0xFF;
    
    // Stencil operations if pixel is front-facing
    DepthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    DepthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    
    // Stencil operations if pixel is back-facing
    DepthStencilDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    DepthStencilDesc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    
    // Create depth stencil state
    Result = Device->CreateDepthStencilState(&DepthStencilDesc, &RenderTarget->DepthStencilState);
    if (FAILED(Result)) 
    {
        printf("Failed to create the depth-/stencil-state!\n");
        return false;
    }
    
    return true;
}


void SetRenderTarget(directx_state *State, render_target *RenderTarget)
{
    assert(State);
    assert(State->DeviceContext);
    assert(RenderTarget);
    
    State->DeviceContext->OMSetDepthStencilState(RenderTarget->DepthStencilState, 1);
    State->DeviceContext->OMSetRenderTargets(1, &RenderTarget->View, RenderTarget->DepthStencilView);
}


void SetBackbufferAsRenderTarget(directx_state *State)
{
    assert(State);
    assert(State->DeviceContext);
    
    State->DeviceContext->OMSetDepthStencilState(nullptr, 1);
    State->DeviceContext->OMSetRenderTargets(1, &State->BackbufferView, nullptr);
}


void UnbindRenderTarget(directx_state *State)
{
    assert(State);
    assert(State->DeviceContext);
    
    State->DeviceContext->OMSetDepthStencilState(nullptr, 1);
    State->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}


void ReleaseRenderTarget(render_target *RenderTarget)
{
    assert(RenderTarget);
    
    RenderTarget->DepthStencilState->Release();
    RenderTarget->DepthStencilView->Release();
    RenderTarget->DepthStencilBuffer->Release();
    RenderTarget->ShaderView->Release();
    RenderTarget->View->Release();
    RenderTarget->Texture->Release();
}



//------------------------------------------------------------------------------------------------------
//
// Rasterizer_ state_
//
b32 CreateRasterizerState(directx_state *State)
{
    assert(State);
    assert(State->Device);
    
    D3D11_RASTERIZER_DESC Desc;
    
    //
    // Solid rasterization
    ZeroMemory(&Desc, sizeof(Desc));
    Desc.FillMode = D3D11_FILL_SOLID;
    Desc.CullMode = D3D11_CULL_BACK;
    Desc.FrontCounterClockwise = true;
    Desc.DepthClipEnable = true;
    Desc.MultisampleEnable = false;
    Desc.AntialiasedLineEnable = true;
    
    HRESULT Result = State->Device->CreateRasterizerState(&Desc, &State->RasterizerStates.Ptr[1]);
    if (FAILED(Result))
    {
        printf("Failed to create solid rasterizer state!\n");
        return false;
    }
    
    
    //
    // Wireframe 
    ZeroMemory(&Desc, sizeof(Desc));
    Desc.FillMode = D3D11_FILL_WIREFRAME;
    Desc.CullMode = D3D11_CULL_NONE;
    Desc.MultisampleEnable = false;
    Desc.AntialiasedLineEnable = true;
    
    Result = State->Device->CreateRasterizerState(&Desc, &State->RasterizerStates.Ptr[2]);
    if (FAILED(Result))
    {
        printf("Failed to create wireframe rasterizer state!\n");
        return false;
    }
    
    return true;
}

void SetRasterizerState(directx_state *State, rasterizer_enum Enum)
{
    assert(State);
    assert(State->DeviceContext);
    assert(State->RasterizerStates.Ptr[Enum]);
    State->DeviceContext->RSSetState(State->RasterizerStates.Ptr[Enum]);
}




//------------------------------------------------------------------------------------------------------
//
// Direct general state_
//

//
// Forward declarations
b32 CreateDevice(directx_state *State);
b32 CreateSwapChain(directx_state *State, directx_config *Config);
b32 CreateDepthAndStencilbuffers(directx_state *State);
b32 CreateFullScreenQuad(directx_state *State);
void SetViewport(directx_state *State, v2 TopLeft = v2_zero, v2 Size = v2_zero, v2 MinMaxDepth = V2(0.0f, 1.0f)); 


b32 SetupDirectX(directx_state *State, directx_config *Config)
{
    assert(State);
    
    CreateDevice(State);
    CreateSwapChain(State, Config);
    SetViewport(State);
    
    
    //
    // Basic shader (no texture or lighting)
    D3D11_INPUT_ELEMENT_DESC E = {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};
    
    CreateShader(State, "..\\build\\shaders\\vbasic.cso", &E, 1, &State->vBasic);
    CreateShader(State, "..\\build\\shaders\\pbasic.cso", &State->pBasic);
    
    CreateShader(State, "..\\build\\shaders\\vpoints_to_quads.cso", &E, 1, &State->vPointsToQuads);
    CreateShader(State, "..\\build\\shaders\\gpoints_to_quads.cso", &State->gPointsToQuads);
    CreateShader(State, "..\\build\\shaders\\ppoints_to_quads.cso", &State->pPointsToQuads);
    
    
    //
    // Shader with lighting
    D3D11_INPUT_ELEMENT_DESC El[] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    CreateShader(State, "..\\build\\shaders\\vbasic_lighting.cso", El, 2, &State->vBasicLighting);
    CreateShader(State, "..\\build\\shaders\\pbasic_lighting.cso", &State->pBasicLighting);
    
    
    //
    // Texture shader (texture but no lighting), renders a texture to the entire screen
    D3D11_INPUT_ELEMENT_DESC Es[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    CreateShader(State, "..\\build\\shaders\\vfullscreen_texture.cso", Es, 2, &State->vFullscreenTexture);
    CreateShader(State, "..\\build\\shaders\\pfullscreen_texture.cso", &State->pFullscreenTexture);
    
    //
    // Sampler
    D3D11_SAMPLER_DESC SamplerDesc;
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.MipLODBias = 0.0f;
    SamplerDesc.MaxAnisotropy = 1;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    SamplerDesc.BorderColor[0] = 1.0f;
    SamplerDesc.BorderColor[1] = 0.0f;
    SamplerDesc.BorderColor[2] = 1.0f;
    SamplerDesc.BorderColor[3] = 1.0f;
    SamplerDesc.MinLOD = 0.0f;
    SamplerDesc.MaxLOD = 0.0f;
    
    HRESULT Result = State->Device->CreateSamplerState(&SamplerDesc, &State->Sampler);
    if (FAILED(Result))
    {
        printf("Failed to create sampler!\n");
        return false;
    }
    
    
    //
    // Create the render target
    b32 BoolResult = CreateRenderTarget(State, State->Width, State->Height, &State->RenderTarget);
    assert(BoolResult);
    
    
    
    //
    // Fullscreen quad, is used for rendering the texture in RenderTarget to the screen 
    BoolResult = CreateFullScreenQuad(State);
    assert(BoolResult);
    
    
    //
    // Set rasterizer state
    Result = CreateRasterizerState(State);
    assert(Result);
    SetRasterizerState(State, RasterizerState_Solid);
    
    return true;
}


b32 CreateFullScreenQuad(directx_state *State)
{
    ReleaseRenderable(&State->FullscreenQuadRenderable);
    
    struct t
    {
        v2 P;
        v2 T;
    };
    
    t Vertices[] =
    {
#if 0
        {{-1.0f, -1.0f}, {0.0f, 1.0f}},
        {{-1.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 0.0f}},
        
        {{-1.0f, -1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 1.0f}},
#else
        {{-1.0f, -1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f}, {0.0f, 0.0f}},
        
        {{-1.0f, -1.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 0.0f}},
#endif
    };
    
    b32 Result = CreateRenderable(State, &State->FullscreenQuadRenderable, 
                                  Vertices, sizeof(t), sizeof(Vertices) / sizeof(t),
                                  D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    return Result;
}


b32 CreateDevice(directx_state *State)
{
    assert(State);
    
    // This flag adds support for surfaces with a color-channel ordering different
    // from the API default. It is required for compatibility with Direct2D.
    // UINT DeviceFlags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
    // TODO(Marcus): Handle pixel format a bit more professionaly, check which formats that are
    //               supported etc...
    //               -- 2018-12-16
    //
    UINT DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    
#ifdef DEBUG
    // Requires D3D11_1SDKLayers.dll
    //DeviceFlags = DeviceFlags | D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE;
    DeviceFlags = DeviceFlags | D3D11_CREATE_DEVICE_DEBUG;
#endif
    
    
    //
    // Create device (default GPU)
    D3D_FEATURE_LEVEL FeatureLevels = D3D_FEATURE_LEVEL_11_0;
    HRESULT Result = D3D11CreateDevice(nullptr,                   // Adapter, nullptr = default
                                       D3D_DRIVER_TYPE_HARDWARE,  // Driver type
                                       nullptr,                   // HMODULE
                                       DeviceFlags,               // Flags
                                       &FeatureLevels,            // Feature level
                                       1,
                                       D3D11_SDK_VERSION,         // Version of the SDK
                                       &State->Device,            // Pointer to the device
                                       nullptr,                   // Returns the used feature level (we don't need that)
                                       &State->DeviceContext);
    if (FAILED(Result) || !State->Device)
    {
        printf("Failed to create device!\n");
        return false;
    }
    
    return true;
}


b32 CreateSwapChain(directx_state *State, directx_config *Config)
{
    assert(State);
    assert(Config);
    assert(State->Device);
    
    if (Config->Width <= 0)  return false;
    if (Config->Height <= 0)  return false;
    
    State->Width = Config->Width;
    State->Height = Config->Height;
    
    
    //
    // SwapChain
    //
    //Fill out a structure describing the SwapChain
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    SwapChainDesc.BufferDesc.Width = State->Width;
    SwapChainDesc.BufferDesc.Height = State->Height;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    
    SwapChainDesc.SampleDesc.Count = 1;      // multisampling setting
    SwapChainDesc.SampleDesc.Quality = 0;    // vendor-specific flag
    
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.OutputWindow = Config->hWnd;
    SwapChainDesc.Windowed = true;           
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // The Flip model is probably better but requires win 10
    SwapChainDesc.Flags = 0;
    
    
    ID3D11Device *Device = State->Device;
    
    
    //
    // Create the SwapChain
    IDXGIFactory *Factory;
    HRESULT Result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory);
    if (FAILED(Result)) 
    {
        printf("Failed to create DXGIFactory!\n");
        return false;
    }
    
    Result = Factory->CreateSwapChain(Device, &SwapChainDesc, &State->SwapChain);
    if (FAILED(Result)) 
    {
        printf("Failed to create the swap chain!\n");
        return false;
    }
    
    Factory->Release();
    
    
    //
    // Get a pointer to the backbuffer
    Result = State->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&State->Backbuffer);
    if (FAILED(Result)) 
    {
        printf("Failed to get the backbuffer!\n");
        return false;
    }
    
    
    //
    // RenderTargetView
    Result = Device->CreateRenderTargetView(State->Backbuffer, nullptr, &State->BackbufferView);
    if (FAILED(Result)) 
    {
        printf("Failed to create the RenderTargetView!\n");
        return false;
    }
    
    return true;
}


void SetViewport(directx_state *State, v2 TopLeft, v2 Size, v2 MinMaxDepth)
{
    assert(State);
    assert(State->DeviceContext);
    
    D3D11_VIEWPORT *Viewport = &State->Viewport;
    
    Viewport->TopLeftX = TopLeft.x;
    Viewport->TopLeftY = TopLeft.y;
    Viewport->Width    = Size.x == 0 ? State->Width  : Size.x;
    Viewport->Height   = Size.y == 0 ? State->Height : Size.y;
    Viewport->MinDepth = MinMaxDepth.x;
    Viewport->MaxDepth = MinMaxDepth.y;
    
    State->DeviceContext->RSSetViewports(1, &State->Viewport);
}


void ReleaseDirectXState(directx_state *State)
{
    if (State)
    {
        ReleaseRenderable(&State->FullscreenQuadRenderable);
        ReleaseShader(&State->pFullscreenTexture);
        ReleaseShader(&State->vFullscreenTexture);
        State->Sampler->Release();
        
        ReleaseShader(&State->pBasic);
        ReleaseShader(&State->vBasic);
        
        ReleaseShader(&State->pBasicLighting);
        ReleaseShader(&State->vBasicLighting);
        
        ReleaseShader(&State->vPointsToQuads);
        ReleaseShader(&State->gPointsToQuads);
        ReleaseShader(&State->pPointsToQuads);
        
        State->BackbufferView->Release();
        State->Backbuffer->Release();
        State->SwapChain->Release();
        State->DeviceContext->Release(); 
        State->Device->Release();
    }
}


void BeginRendering(directx_state *State)
{
    assert(State);
    assert(State->DeviceContext);
    assert(State->BackbufferView);
    
    //
    // Clear
    ID3D11DeviceContext *DC = State->DeviceContext;
    DC->ClearDepthStencilView(State->RenderTarget.DepthStencilView, 
                              D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    v4 Colour = V4(0.45f, 0.55f, 0.65f, 1.0f);
    DC->ClearRenderTargetView(State->RenderTarget.View, (float const *)&Colour);
}

void EndRendering(directx_state *State)
{
    assert(State);
    assert(State->vFullscreenTexture.Program);
    assert(State->pFullscreenTexture.Program);
    assert(State->RenderTarget.ShaderView);
    assert(State->Sampler);
    assert(State->SwapChain);
    
    UnbindRenderTarget(State);
    
    //
    /// Render texture to screen
    SetBackbufferAsRenderTarget(State);
    SetShader(State, &State->vFullscreenTexture);
    SetShader(State, &State->pFullscreenTexture);
    SetPixelShaderResource(State, 0, 1, &State->RenderTarget.ShaderView);
    SetPixelShaderSampler(State, 0, 1, &State->Sampler);
    
    RenderRenderable(State, &State->FullscreenQuadRenderable);
    
    ID3D11ShaderResourceView *NullView = nullptr;
    SetPixelShaderResource(State, 0, 1, &NullView);
    
    
    //
    // Update
    State->SwapChain->Present(0, 0);
}




//------------------------------------------------------------------------------------------------------
//
// DirectWrite
//
b32 InitDirectWrite(directx_state *DirectXState, directwrite_state *State)
{
    assert(DirectXState);
    assert(DirectXState->Device);
    assert(DirectXState->SwapChain);
    assert(State);
    
    IDWriteFactory *DWriteFactory;
    HRESULT Result = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                         __uuidof(IDWriteFactory), 
                                         (IUnknown **)&DWriteFactory);
    if (FAILED(Result))
    {
        // TODO(Marcus): Add better error handling
        printf("Failed to create the DirectWrite factory!\n");
        return false;
    }
    
    
    //
    // Direct2D
    D2D1_FACTORY_OPTIONS Options;
    Options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
    ID2D1Factory1 *D2DFactory;
    Result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, // Will only be used in this thread
                               __uuidof(ID2D1Factory),
                               &Options,
                               (void **)&D2DFactory);
    if (FAILED(Result))
    {
        DWriteFactory->Release();
        printf("Failed to create the Direct2D factory!\n");
        return false;
    }
    
    IDXGIDevice *DXGIDevice;
    Result = DirectXState->Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&DXGIDevice);
    if (FAILED(Result))
    {
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to get the DXGIDevice from the D3D10Device1!\n");
        return false;
    }
    
    Result = D2DFactory->CreateDevice(DXGIDevice, &State->Device);
    if (FAILED(Result))
    {
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DDevice!\n");
        return false;
    }
    
    ID2D1Device *Device = State->Device;
    Result = Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &State->DeviceContext);
    if (FAILED(Result))
    {
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DDeviceContext!\n");
        return false;
    }
    
    IDXGISurface *D2DBuffer;
    Result = DirectXState->SwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void **)&D2DBuffer);
    if (FAILED(Result))
    {
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to get the backbuffer as IDXGISurface!\n");
        return false;
    }
    
    D2D1_BITMAP_PROPERTIES1 Properties;
    Properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    Properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
    Properties.dpiX = 0;
    Properties.dpiX = 0;
    Properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    Properties.colorContext = nullptr;
    
    ID2D1DeviceContext *DeviceContext = State->DeviceContext;
    Result = DeviceContext->CreateBitmapFromDxgiSurface(D2DBuffer, Properties, &State->RenderTarget);
    if (FAILED(Result))
    {
        D2DBuffer->Release();
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DBitmap!\n");
        return false;
    }
    DeviceContext->SetTarget(State->RenderTarget);
    
    Result = DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &State->Brush);
    if (FAILED(Result))
    {
        State->RenderTarget->Release();
        D2DBuffer->Release();
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DBrush!\n");
        return false;
    }
    
    Result = DWriteFactory->CreateTextFormat(L"Microsoft New Tai Lue",
                                             nullptr,
                                             DWRITE_FONT_WEIGHT_NORMAL,
                                             DWRITE_FONT_STYLE_NORMAL,
                                             DWRITE_FONT_STRETCH_NORMAL,
                                             96.0f/5.0f,
                                             L"en-GB",
                                             &State->TextFormat);
    if (FAILED(Result))
    {
        State->Brush->Release();
        State->RenderTarget->Release();
        D2DBuffer->Release();
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DTextFormat!\n");
        return false;
    }
    
    State->TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    State->TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    
    D2DBuffer->Release();
    DXGIDevice->Release();
    D2DFactory->Release();
    DWriteFactory->Release();
    
    return true;
}



void ReleaseDirectWrite(directwrite_state *State)
{
    assert(State);
    State->TextFormat ? State->TextFormat->Release() : 0;
    State->Brush ? State->Brush->Release() : 0;
    State->RenderTarget ? State->RenderTarget->Release() : 0;
    State->DeviceContext? State->DeviceContext->Release() : 0;
    State->Device ? State->Device->Release() : 0;
}



void BeginDraw(directwrite_state *State)
{
    assert(State && State->DeviceContext);
    State->DeviceContext->BeginDraw();
}


HRESULT EndDraw(directwrite_state *State)
{
    assert(State && State->DeviceContext);
    HRESULT Result = State->DeviceContext->EndDraw();
    
    return Result;
}


void DrawTextW(directwrite_state *State, WCHAR const *String, v2 P)
{
    assert(State && State->DeviceContext);
    assert(String);
    
    v2 Offset = V2(60.0f, 20.0f);
    D2D1_RECT_F LayoutRect;
    LayoutRect.top    = P.y - Offset.y;
    LayoutRect.left   = P.x - Offset.x;
    LayoutRect.bottom = P.y + Offset.y;
    LayoutRect.right  = P.x + Offset.x;
    
    State->DeviceContext->DrawText(String,
                                   wcslen(String),
                                   State->TextFormat,
                                   &LayoutRect,
                                   State->Brush,
                                   D2D1_DRAW_TEXT_OPTIONS_NONE,
                                   DWRITE_MEASURING_MODE_NATURAL);
}
