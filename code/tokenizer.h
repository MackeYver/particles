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


#ifndef Tokenizer__h
#define Tokenizer__h

#include "types.h"



enum token_type
{
    Token_OpenParen,
    Token_CloseParen,
    Token_Colon,
    Token_Semicolon,
    Token_Asterisk,
    Token_OpenBracket,
    Token_CloseBracket,
    Token_OpenBrace,
    Token_CloseBrace,
    Token_Equals,
    Token_Comma,
    Token_Point,
    
    Token_String,
    Token_Identifier,
    Token_Number,
    
    Token_EndOfStream,
    Token_Unknown,
};


struct token
{
    char *FileName;
    s32 LineNumber;
    
    token_type Type;
    string Text;
    f32 f32;
    s32 s32;
};


struct tokenizer
{
    char *FileName;
    s32 LineNumber = 1;
    
    string Input;
    char At[2];
    
    b32 Error;
};


b32 Parsing(tokenizer *Tokenizer);
tokenizer Tokenize(string Input);

token GetToken(tokenizer *Tokenizer);

void SkipToEndOfLine(tokenizer *Tokenizer);
token GetFirstTokenOfNextLine(tokenizer *Tokenizer);

token RequireToken(tokenizer *Tokenizer, token_type DesiredType);
token RequireIdentifierNamed(tokenizer *Tokenizer, const char *Name);
token RequireNumberWithValue(tokenizer *Tokenizer, f32 Value);
token RequireNumber(tokenizer *Tokenizer);

b32 OptionalIdentifierNamed(tokenizer *Tokenizer, const char *Name);

void Error(tokenizer *Tokenizer, token OnToken, char *Format, ...);
void Error(tokenizer *Tokenizer, char *Format, ...);


#endif