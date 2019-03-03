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

#include "tokenizer.h"
#include <stdarg.h>
#include <stdio.h>



b32 Parsing(tokenizer *Tokenizer)
{
    return !Tokenizer->Error && Tokenizer->Input.Count > 0;
}


void Refill(tokenizer *Tokenizer)
{
    if (Tokenizer->Input.Count == 0)
    {
        Tokenizer->At[0] = 0;
        Tokenizer->At[1] = 0;
    }
    else if (Tokenizer->Input.Count == 1)
    {
        Tokenizer->At[0] = Tokenizer->Input.Data[0];
        Tokenizer->At[1] = 0;
    }
    else
    {
        Tokenizer->At[0] = Tokenizer->Input.Data[0];
        Tokenizer->At[1] = Tokenizer->Input.Data[1];
    }
}


void AdvanceAndRefill(tokenizer *Tokenizer, u32 Count)
{
    Advance(&Tokenizer->Input, Count);
    Refill(Tokenizer);
}


tokenizer Tokenize(string Input)
{
    tokenizer Result = {};
    Result.Input = Input;
    Refill(&Result);
    
    return Result;
}


void EatAllWhiteSpaces(tokenizer *Tokenizer)
{
    for(;;)
    {
        if (IsWhiteSpace(Tokenizer->At[0]))
        {
            if (IsEndOfLine(Tokenizer->At[0]))
            {
                ++Tokenizer->LineNumber;
            }
            AdvanceAndRefill(Tokenizer, 1);
        }
        else
        {
            return;
        }
    }
}


token GetToken(tokenizer *Tokenizer)
{
    EatAllWhiteSpaces(Tokenizer);
    
    token Result = {};
    Result.FileName   = Tokenizer->FileName;
    Result.LineNumber = Tokenizer->LineNumber;
    Result.Text       = Tokenizer->Input;
    if (Result.Text.Count)
    {
        Result.Text.Count = 1;
    }
    
    char C = Tokenizer->At[0];
    AdvanceAndRefill(Tokenizer, 1);
    
    switch(C)
    {
        case '(' : { Result.Type = Token_OpenParen;    } break;
        case ')' : { Result.Type = Token_CloseParen;   } break;
        case ':' : { Result.Type = Token_Colon;        } break;
        case ';' : { Result.Type = Token_Semicolon;    } break;
        case '*' : { Result.Type = Token_Asterisk;     } break;
        case '[' : { Result.Type = Token_OpenBracket;  } break;
        case ']' : { Result.Type = Token_CloseBracket; } break;
        case '{' : { Result.Type = Token_OpenBrace;    } break;
        case '}' : { Result.Type = Token_CloseBrace;   } break;
        case '=' : { Result.Type = Token_Equals;       } break;
        case ',' : { Result.Type = Token_Comma;        } break;
        case '.' : { Result.Type = Token_Point;        } break;
        
        case '\0': { Result.Type = Token_EndOfStream;  } break;
        
        case '"' :
        {
            Result.Type = Token_String;
            
            while (Tokenizer->At[0] && Tokenizer->At[0] != '"')
            {
                AdvanceAndRefill(Tokenizer, 1);
            }
            
            Result.Text.Count = Tokenizer->Input.Data - Result.Text.Data;
            if (Tokenizer->At[0] == '"')
            {
                AdvanceAndRefill(Tokenizer, 1);
            }
        } break;
        
        default:
        {
            if (IsAlpha(C))
            {
                Result.Type = Token_Identifier;
                while (IsAlpha(Tokenizer->At[0]) ||
                       IsNumber(Tokenizer->At[0]) ||
                       Tokenizer->At[0] == '_')
                {
                    AdvanceAndRefill(Tokenizer, 1);
                }
                
                Result.Text.Count = Tokenizer->Input.Data - Result.Text.Data;
            }
            else if(IsNumber(C) || IsNumberSigned(C, Tokenizer->At[0]))
            {
                s32 Sign = 1;
                
                if (C == '-')
                {
                    Sign = -1;
                    C = Tokenizer->At[0];
                    AdvanceAndRefill(Tokenizer, 1);
                }
                
                Result.Type = Token_Number;
                f32 Number = (f32)(C - '0');
                
                while(IsNumber(Tokenizer->At[0]))
                {
                    f32 Digit = (f32)(Tokenizer->At[0] - '0');
                    Number = 10.0f*Number + Digit;
                    AdvanceAndRefill(Tokenizer, 1);
                }
                
                if (Tokenizer->At[0] == '.')
                {
                    AdvanceAndRefill(Tokenizer, 1);
                    
                    f32 Coefficient = 0.1f;
                    while(IsNumber(Tokenizer->At[0]))
                    {
                        f32 Digit = (f32)(Tokenizer->At[0] - '0');
                        Number += Coefficient*Digit;
                        Coefficient *= 0.1f;
                        AdvanceAndRefill(Tokenizer, 1);
                    }
                }
                
                Number *= Sign;
                Result.f32 = Number;
                Result.s32 = (s32)(Number); // TODO(Marcus): Round?
                Result.Text.Count = Tokenizer->Input.Data - Result.Text.Data;
                
            }
            else
            {
                Result.Type = Token_Unknown;
            }
        }
    }
    
    return Result;
}


void SkipToEndOfLine(tokenizer *Tokenizer)
{
    while (!IsEndOfLine(Tokenizer->At[0]))
    {
        AdvanceAndRefill(Tokenizer, 1);
    }
    
    //token Token = GetToken(Tokenizer);
    //return Token;
}


token GetFirstTokenOfNextLine(tokenizer *Tokenizer)
{
    SkipToEndOfLine(Tokenizer);
    token Token = GetToken(Tokenizer);
    
    return Token;
}


token RequireToken(tokenizer *Tokenizer, token_type DesiredType)
{
    token Token = GetToken(Tokenizer);
    if(Token.Type != DesiredType)
    {
        Error(Tokenizer, Token, "Unexpected token type");
    }
    
    return(Token);
}


token RequireIdentifierNamed(tokenizer *Tokenizer, char const *Name)
{
    token Token = GetToken(Tokenizer);
    if(Token.Type != Token_Identifier || !StringsAreEqual(Token.Text, (char *)Name))
    {
        Error(Tokenizer, Token, "Expected an identifier named %s", Name);
    }
    
    return(Token);
}


token RequireNumberWithValue(tokenizer *Tokenizer, f32 Value)
{
    token Token = GetToken(Tokenizer);
    if(Token.Type != Token_Number || Token.f32 != Value)
    {
        Error(Tokenizer, Token, "Expected a number with value %f", Value);
    }
    
    return(Token);
}


token RequireNumber(tokenizer *Tokenizer)
{
    token Token = GetToken(Tokenizer);
    if(Token.Type != Token_Number)
    {
        Error(Tokenizer, Token, "Expected a number");
    }
    
    return(Token);
}


b32 OptionalIdentifierNamed(tokenizer *Tokenizer, const char *Name)
{
    token Token = GetToken(Tokenizer);
    if (Token.Type == Token_Identifier && StringsAreEqual(Token.Text, (char *)Name))
    {
        return true;
    }
    
    return false;
}


void Error(tokenizer *Tokenizer, token OnToken, char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    
    //Outf_(OnToken.FileName, OnToken.LineNumber, Tokenizer->ErrorStream, Format, ArgList);
    printf("%d: ", OnToken.LineNumber);
    vprintf(Format, ArgList);
    printf("\n");
    
    va_end(ArgList);
    
    Tokenizer->Error = true;
}


void Error(tokenizer *Tokenizer, char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    
    //Outf_(Tokenizer->FileName, Tokenizer->LineNumber, Tokenizer->ErrorStream, Format, ArgList);
    vprintf(Format, ArgList);
    printf("\n");
    
    va_end(ArgList);
    
    Tokenizer->Error = true;
}