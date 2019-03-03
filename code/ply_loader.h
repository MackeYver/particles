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
// TODO(Marcus): Clean up this code, it works but it's a bit messy!
//

//
// NOTE(Marcus): Supported elements (only the mentioned elements and properties are supported):        
//               - vertex
//                 - Requiered property:
//                   - Positions, named x y z, type float
//
//                 - Optional properties:
//                   - normals, named nx ny nz, type float
//                   - texture coordinates, named s t, type float
//
//               - face
//                 - Required property
//                   - none
// 
//                 - Optional property
//                   - none
//



#ifndef ply_loader__h
#define ply_loader__h

#include <stdlib.h>
#include <stdio.h>
#include "tokenizer.h"
#include "mathematics.h"


#ifdef DEBUG
#include <assert.h>
#define Assertu16(x) assert(x < 65536)
#else
#define assert(x)
#define Assertu16(x)
#endif



//
// Ply
enum ply_token
{
    PlyToken_MagicNumber,
    PlyToken_Format,
    PlyToken_Comment,
    PlyToken_Element,
    PlyToken_Property,
    PlyToken_EndHeader,
    PlyToken_Unknown,
};


static ply_token TranslateToPlyToken(token *Token)
{
    if (StringsAreEqual(Token->Text, "ply"))
    {
        return PlyToken_MagicNumber;
    }
    else if (StringsAreEqual(Token->Text, "format"))
    {
        return PlyToken_Format;
    }
    else if (StringsAreEqual(Token->Text, "comment"))
    {
        return PlyToken_Comment;
    }
    else if (StringsAreEqual(Token->Text, "element"))
    {
        return PlyToken_Element;
    }
    else if (StringsAreEqual(Token->Text, "property"))
    {
        return PlyToken_Property;
    }
    else if (StringsAreEqual(Token->Text, "end_header"))
    {
        return PlyToken_EndHeader;
    }
    else
    {
        return PlyToken_Unknown;
    }
}


struct ply_state
{
    v3 *Positions  = nullptr;
    v3 *Normals   = nullptr;
    v2 *TexCoords = nullptr;
    u16 *Indices = nullptr;
    
    u32 PositionElementCount = 0;
    u32 NormalElementCount = 0;
    u32 TexCoordElementCount = 0;
    
    u32 VertexCount = 0;
    u32 VertexSize;
    u32 FaceCount = 0;
    u32 IndexCount = 0;
    
    b32 FinishedWithHeader = false;
};


static token ProcessFormat(tokenizer *Tokenizer, ply_state *State)
{
    token Token = RequireIdentifierNamed(Tokenizer, "ascii"); // ascii
    Token = RequireNumberWithValue(Tokenizer, 1.0f); // 1.0
    
    Token = GetToken(Tokenizer);
    return Token;
}


static token RequireFloatPropertyNamed(tokenizer *Tokenizer, char const *Name)
{
    token Token = RequireIdentifierNamed(Tokenizer, "property");
    Token = RequireIdentifierNamed(Tokenizer, "float");
    Token = RequireIdentifierNamed(Tokenizer, Name);
    
    return Token;
}


static token ProcessElement(tokenizer *Tokenizer, ply_state *State)
{
    token Token = GetToken(Tokenizer);
    
    if (StringsAreEqual(Token.Text, "vertex"))
    {
        Token = RequireNumber(Tokenizer);
        State->VertexCount = Token.s32;
        
        // NOTE(Marcus): We're requiring that the vertices has at least three properties:
        //               x, y and z (floats), the position
        //
        //               In addition to this they _can_ have (they are optional) two additional properties:
        //               - nx, ny and nz (floats), this is the normal of the vertex
        //               - s and t, this is the texture coordinate
        
        Token = RequireFloatPropertyNamed(Tokenizer, "x");
        Token = RequireFloatPropertyNamed(Tokenizer, "y");
        Token = RequireFloatPropertyNamed(Tokenizer, "z");
        
        State->VertexSize = sizeof(v3);
        State->PositionElementCount = 3;
        
        //
        // Check if there are any other properties of the vertex
        Token = GetToken(Tokenizer);
        ply_token PlyToken = TranslateToPlyToken(&Token);
        while (Parsing(Tokenizer) && (PlyToken == PlyToken_Property))
        {
            Token = RequireIdentifierNamed(Tokenizer, "float");
            Token = GetToken(Tokenizer);
            
            if (StringsAreEqual(Token.Text, "nx"))
            {
                Token = RequireFloatPropertyNamed(Tokenizer, "ny");
                Token = RequireFloatPropertyNamed(Tokenizer, "nz");
                
                State->VertexSize += sizeof(v3);
                State->NormalElementCount = 3;
            }
            else if (StringsAreEqual(Token.Text, "s"))
            {
                Token = RequireFloatPropertyNamed(Tokenizer, "t");
                
                State->VertexSize += sizeof(v2);
                State->TexCoordElementCount = 2;
            }
            Token = GetToken(Tokenizer);
            PlyToken = TranslateToPlyToken(&Token);
        }
    }
    else if (StringsAreEqual(Token.Text, "face"))
    {
        // NOTE(Marcus): We're requring the face to consist out of three indices, i.e. we're
        //               requiring the faces to be triangulated.
        Token = RequireNumber(Tokenizer);
        State->FaceCount = Token.s32;
        State->IndexCount = 3*State->FaceCount;
        Token = GetFirstTokenOfNextLine(Tokenizer);
    }
    else
    {
        // TODO(Marcus): Support arbitrary named elements? For now we'll stop parsing here because
        //               we did not expect this unknown element -- perhaps there were errors in 
        //               the export?
        Error(Tokenizer, "%d: Unsupported element", Token.LineNumber);
    }
    
    return Token;
}


static token ProcessHeader(tokenizer *Tokenizer, ply_state *State)
{
    //
    // TODO(Marcus): Ugly solution this is, fix it I must!
    //
    if (State->VertexCount == 0 || State->PositionElementCount < 3)
    {
        Error(Tokenizer, "Finished parsing the header but found no vertices, or no vertices which also has positions");
    }
    else
    {
        if (State->PositionElementCount != 3)
        {
            Error(Tokenizer, "Expected 3 elements in the position coordinate, got %d", State->PositionElementCount);
        }
        else
        {
            State->Positions = (v3 *)malloc(State->VertexCount * sizeof(v3));
        }
        
        if (State->NormalElementCount > 0)
        {
            if (State->NormalElementCount != 3)
            {
                Error(Tokenizer, "Expected 3 elements in the normal coordinate, got %d", State->NormalElementCount);
            }
            else
            {
                State->Normals = (v3 *)malloc(State->VertexCount * sizeof(v3));
            }
        }
        
        if (State->TexCoordElementCount > 0)
        {
            if (State->TexCoordElementCount != 2)
            {
                Error(Tokenizer, "Expected 2 elements in the texture coordinate, got %d", State->TexCoordElementCount);
            }
            else
            {
                State->TexCoords = (v2 *)malloc(State->VertexCount * sizeof(v2));
            }
        }
        
        if (State->IndexCount > 0)
        {
            State->Indices = (u16 *)malloc(State->IndexCount * sizeof(u16));
        }
        
        State->FinishedWithHeader = true;
    }
    
    token Token = GetToken(Tokenizer);
    return Token;
}


static void Free(ply_state *State)
{
    if (State->Positions)
    {
        free(State->Positions);
        State->Positions = nullptr;
    }
    
    if (State->Normals)
    {
        free(State->Normals);
        State->Normals = nullptr;
    }
    
    if (State->TexCoords)
    {
        free(State->TexCoords);
        State->TexCoords = nullptr;
    }
    
    if (State->Indices)
    {
        free(State->Indices);
        State->Indices = nullptr;
    }
    
    State->PositionElementCount = 0;
    State->NormalElementCount = 0;
    State->TexCoordElementCount = 0;
    State->VertexCount = 0;
    State->VertexSize = 0;
    State->IndexCount = 0;
    State->FinishedWithHeader = false;
}



//
// Main
static b32 LoadPlyFile(char const *FileName, ply_state *State)
{
    //
    // Read file
    FILE *File;
    fopen_s(&File, FileName, "r");
    fseek(File, 0L, SEEK_END);
    size_t Size = ftell(File);
    rewind(File);
    
    string FileString;
    FileString.Data = (u8 *)malloc(Size + 1);
    fread(FileString.Data, Size, 1, File);
    fclose(File);
    
    
    //
    // Alloc and copy
    FileString.Count = Size;
    tokenizer Tokenizer = Tokenize(FileString);
    size_t FileNameSize = sizeof(FileName) + 1;
    Tokenizer.FileName = (char *)malloc(FileNameSize);
    snprintf(Tokenizer.FileName, FileNameSize, FileName);
    
    
    //
    // Tokenize and parse
    token Token;
    Token = RequireIdentifierNamed(&Tokenizer, "ply");
    if (!Parsing(&Tokenizer))
    {
        Error(&Tokenizer, "First line did not contain the magic number, is this really a .ply file?");
    }
    Token = GetToken(&Tokenizer);
    
    while(Parsing(&Tokenizer))
    {
        //Token = GetToken(&Tokenizer);
        // NOTE(Marcus): Each process function in the switch case below will advance the tokenizer
        //               and return the next token (i.e. the token that comes after the last token
        //               the function processed).
        
        switch (Token.Type)
        {
            case Token_Identifier:
            {
                ply_token PlyToken = TranslateToPlyToken(&Token); 
                switch (PlyToken)
                {
                    case PlyToken_Format   : { Token = ProcessFormat(&Tokenizer, State);      } break;
                    case PlyToken_Element  : { Token = ProcessElement(&Tokenizer, State);     } break;
                    case PlyToken_EndHeader: { Token = ProcessHeader(&Tokenizer, State);      } break;
                    case PlyToken_Comment  : { Token = GetFirstTokenOfNextLine(&Tokenizer);    } break;
                    case PlyToken_Property : { Token = GetFirstTokenOfNextLine(&Tokenizer);    } break;
                    case PlyToken_Unknown  : { Error(&Tokenizer, Token, "Unknown identifier"); } break;
                }
            } break;
            
            case Token_Number:
            {
                if (!State->FinishedWithHeader)
                {
                    Error(&Tokenizer, Token, "Encountered free form numbers before the header was parsed");
                }
                else
                {
                    // NOTE(Marcus): We're assuming that the first time we encounter a number, _and_
                    //               the header was parsed, then it is the start of the vertex data.
                    for (u32 Index = 0; Index < State->VertexCount && Parsing(&Tokenizer); ++Index)
                    {
                        f32 x = Token.f32;
                        Token = RequireNumber(&Tokenizer);
                        f32 y = Token.f32;
                        Token = RequireNumber(&Tokenizer);
                        f32 z = Token.f32;
                        State->Positions[Index] = V3(x, y, z);
                        
                        Token = GetToken(&Tokenizer);
                        if (State->NormalElementCount == 3 && Token.Type == Token_Number)
                        {
                            f32 nx = Token.f32;
                            Token = RequireNumber(&Tokenizer);
                            f32 ny = Token.f32;
                            Token = RequireNumber(&Tokenizer);
                            f32 nz = Token.f32;
                            State->Normals[Index] = V3(nx, ny, nz);
                        }
                        
                        Token = GetToken(&Tokenizer);
                        if (State->TexCoordElementCount == 2 && Token.Type == Token_Number)
                        {
                            f32 s = Token.f32;
                            Token = RequireNumber(&Tokenizer);
                            f32 t = Token.f32;
                            State->TexCoords[Index] = V2(s, t);
                            
                            Token = GetToken(&Tokenizer);
                        }
                        
                    }
                    
                    for (u32 Index = 0; Index < State->FaceCount && Parsing(&Tokenizer); ++Index)
                    {
                        if (Token.s32 != 3)
                        {
                            Error(&Tokenizer, Token, "Expected 3 indices per face but got %d", Token.s32);
                        }
                        else
                        {
                            Token = RequireNumber(&Tokenizer);
                            Assertu16(Token.s32);
                            State->Indices[3*Index] = (u16)Token.s32;
                            
                            Token = RequireNumber(&Tokenizer);
                            Assertu16(Token.s32);
                            State->Indices[3*Index + 1] = (u16)Token.s32;
                            
                            Token = RequireNumber(&Tokenizer);
                            Assertu16(Token.s32);
                            State->Indices[3*Index + 2] = (u16)Token.s32;
                            
                            if (Index < State->FaceCount - 1)
                            {
                                Token = RequireNumber(&Tokenizer);
                            } else
                            {
                                Token = GetToken(&Tokenizer);
                            }
                        }
                    }
                }
            } break;
        }
    }
    
#if 0
    if (!Tokenizer.Error)
    {
        for (u32 Index = 0; Index < State->VertexCount; ++Index)
        {
            printf("%2d. P = (%6.2f, %6.2f, %6.2f)",  Index,
                   State->Positions[Index].x, State->Positions[Index].y, State->Positions[Index].z);
            
            if (State->NormalElementCount > 0)
            {
                printf(", N = (%6.2f, %6.2f, %6.2f)",
                       State->Normals[Index].x, State->Normals[Index].y, State->Normals[Index].z);
            }
            
            if (State->TexCoordElementCount > 0)
            {
                printf(", T = (%6.2f, %6.2f)",
                       State->TexCoords[Index].x, State->TexCoords[Index].y);
            }
            
            printf("\n");
        }
        
        for (u32 Index = 0; Index < State->IndexCount; Index += 3)
        {
            printf("%2d. %3d - %3d - %3d\n", Index,
                   State->Indices[Index], State->Indices[Index + 1], State->Indices[Index + 2]);
        }
    }
#endif
    
    free(Tokenizer.FileName);
    free(FileString.Data);
    
    if (Tokenizer.Error)
    {
        Free(State);
        return false;
    }
    else
    {
        return true;
    }
}

#endif