//===-- llbuild.cpp - llbuild Frontend Utillity ---------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "llbuild/Basic/Version.h"

#include "llbuild/Ninja/Lexer.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace llbuild;

static char hexdigit(unsigned Input) {
  return (Input < 10) ? '0' + Input : 'A' + Input;
}

static std::string escapedString(const char *Start, unsigned Length) {
  std::stringstream Result;
  for (unsigned i = 0; i != Length; ++i) {
    char c = Start[i];
    if (isprint(c)) {
      Result << c;
    } else if (c == '\n') {
      Result << "\\n";
    } else {
      Result << "\\x"
             << hexdigit(((unsigned char) c >> 4) & 0xF)
             << hexdigit((unsigned char) c & 0xF);
    }
  }
  return Result.str();
}
int main(int argc, const char **argv) {
    // Print the version and exit.
    printf("%s\n", getLLBuildFullVersion().c_str());

    // Check the Ninja lexer, for now.
    if (argc != 2) {
      fprintf(stderr, "error: %s: invalid number of arguments\n",
              getprogname());
      return 1;
    }

    // Open the input buffer and compute its size.
    FILE *fp = fopen(argv[1], "rb");
    fseek(fp, 0, SEEK_END);
    uint64_t Size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Read the file contents.
    std::unique_ptr<char[]> Data(new char[Size]);
    uint64_t Pos = 0;
    while (Pos < Size) {
      // Read data into the buffer.
      size_t Result = fread(Data.get() + Pos, 1, Size - Pos, fp);
      if (Result <= 0) {
        fprintf(stderr, "error: %s: unable to read input\n", getprogname());
        return 1;
      }

      Pos += Result;
    }

    // Create a Ninja lexer.
    fprintf(stderr, "note: %s: reading tokens from %s\n", getprogname(),
            argv[1]);
    ninja::Lexer Lexer(Data.get(), Size);
    ninja::Token Tok;
    do {
      // Get the next token.
      Lexer.lex(Tok);

      std::cerr << "(Token \"" << Tok.getKindName() << "\""
                << " String:\"" << escapedString(Tok.Start, Tok.Length) << "\""
                << " Length:" << Tok.Length
                << " Line:" << Tok.Line
                << " Column:" << Tok.Column << ")\n";
    } while (Tok.TokenKind != ninja::Token::Kind::EndOfFile);

    return 0;
}
