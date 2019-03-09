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



struct camera
{
    v4 P;
    f32 Distance;
};


static m4 GetWorldToViewMatrix(camera *Camera)
{
    v3 P = Camera->P.xyz();
    m4 Result = M4LookAt(Camera->Distance * P, v3_zero);
    
    return Result;
}


static void UpdateCamera(camera *Camera, mouse_state *MouseState, display_metrics *Metrics)
{
    if (MouseState->RBDown)
    {
        v2 M = MouseState->P - MouseState->RBPrevP;
        f32 ry = -Tau32 * (M.x / Metrics->ScreenWidth);
        f32 rx =  Tau32 * (M.y / Metrics->ScreenHeight);
        
        m4 Rx = M4RotationX(rx);
        m4 Ry = M4RotationY(ry);
        
        Camera->P *= Ry * Rx;
        
        MouseState->RBPrevP = MouseState->P;
    }
}
