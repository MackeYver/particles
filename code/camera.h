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
    display_metrics *Metrics;
    mouse_state *MouseState;
    
    v3 P;
    f32 OrbitAngle;
    f32 PitchAngle;
    f32 Distance;
};


static m4 GetWorldToViewMatrix(camera *Camera)
{
    m4 Result = M4LookAt(Camera->P, v3_zero);
    
    return Result;
}


static void UpdateCamera(camera *Camera)
{
    mouse_state *MouseState = Camera->MouseState;
    
    if (MouseState->RBDown)
    {
        display_metrics *Metrics = Camera->Metrics;
        
        v2 M = MouseState->P - MouseState->PrevP;
        
        Camera->OrbitAngle += -Tau32 * (M.x / Metrics->ScreenWidth);
        Camera->PitchAngle +=  Tau32 * (M.y / Metrics->ScreenHeight);
    }
    
    //
    // Calculate position
    Camera->Distance = Min(250.0f, Max(5.0f, Camera->Distance));
    
    v3 P = Normalize(V3(Cos(Camera->OrbitAngle),
                        Sin(Camera->PitchAngle),
                        -Sin(Camera->OrbitAngle)));
    Camera->P = Camera->Distance * P;
}
