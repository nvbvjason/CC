# CC

## Quick start guide
Linux x86_64 required

#### Clone repo:
`git clone https://github.com/nvbvjason/CC.git`

#### Build:
- `cd CC`
- `cmake -B build -S .`
- `cmake --build build  --target CC`

#### Build example file:
`./build/src/CC examples/hello_world.c`

#### Run it:
`./examples/hello_world`

## Language Specification

###### The compiler supports a substantial subset of C with the following grammar:

```ebnf
<program>               ::= { <declaration> }
<declaration>           ::= <function-declaration> | <variable-declaration>
<variable-declaration>  ::= { <specifier> }+ <declarator> [ "=" <initializer> ] ";"
<function-declaration>  ::= { <specifier> }+ <declarator> "(" <param-list> ")" ( <block> | ";" )
<declarator>            ::= "*" <declarator> | <direct-declarator>
<direct-declarator>     ::= <simple-declarator> [ <declarator-suffix> ]
<declarator-suffix>     ::= <param-list> | { "[" <const> "]" }+
<param-list>            ::= "(" "void" ")" | "(" <param> [ "," <param> ] ")"
<param>                 ::= { <type-specifier> }+ <declarator>
<simple-declarator>     ::= <identifier> | "(" <declarator> ")"
<type-specifier>        ::= "int" | "long" | "unsigned" | "signed" | "double" | "char"
<specifier>             ::= <type-specifier> | "static" | "extern"
<block>                 ::= "{" { <block-item> } "}"
<block-item>            ::= <statement> | <declaration>
<initializer>           ::= <exp> | "{" <initializer> { "," <initializer> } [ "," ] ")"
<for-init>              ::= <variable-declaration> | <exp> ";"
<statement>             ::= "return" <exp> ";"
                          | <exp> ";"
                          | "if" "(" <exp> ")" <statement> [ "else" <statement> ]
                          | "switch" "(" <exp> ")" <statement>
                          | "goto" <identifier> ";"
                          | <block>
                          | "break" ";"
                          | "continue" ";"
                          | <identifier> ":" <statement>
                          | "case" <exp> ":" <statement>
                          | "default" ":" <statement>
                          | "while" "(" <exp> ")" <statement>
                          | "do" <statement> "while" "(" <exp> ")" ";"
                          | "for" "(" <for-init> [ <exp> ] ";" [ <exp> ] ")" <statement>
                          | ";"
<exp>                   ::= <unary-exp>
                          | <exp> <binary-op> <exp>
                          | <exp> "?" <exp> ":" <exp>
<cast-exp>              ::= "(" { <type-specifier> }+ [ <abstract-declarator> ] ")" <cast-exp>
                          | <unary-exp>
<unary-exp>             ::= <postfix-exp> | <unary-op> <cast-exp>
<postfix-exp>           ::= <factor> | <postfix-exp> <postfix-op>
<factor>                ::= <const>
                          | { <string> }+
                          | <identifier>
                          | <identifier> "(" [ <argument-list> ] ")"
                          | "(" <exp> ")"
<argument-list>         ::= <exp> { "," <exp> }
<abstract-declarator>   ::= "*" [ <abstract-declarator> ]
                          | <direct-abstract-declarator>
<direct-abstract-declarator> ::= "(" <abstract-declarator> ")"
<unary-op>               ::= "+" | "-" | "~" | "!" | "--" | "++" | "*" | "&"
<postfix-op>             ::= "--" | "++"
<binary-op>              ::= "-" | "+" | "*" | "/" | "%" | "^" | "<<" | ">>" | "&" | "|"
                          | "&&" | "||" | "==" | "!=" | "<" | "<=" | ">" | ">=" | "="
                          | "+=" | "-=" | "*=" | "/=" | "%="
                          | "&=" | "|=" | "^=" | "<<=" | ">>="
<const>                 ::= <int> | <long> | <uint> | <ulong> | <double> | <char>
<identifier>            ::= ? An identifier token ?
<string>                ::= ? A string token ?
<char>                  ::= ? A char token ?
<int>                   ::= ? A int token ?
<long>                  ::= ? A long token ?
<uint>                  ::= ? An unsigned int token ?
<ulong>                 ::= ? An unsigned long token ?
<double>                ::= ? A floating-point constant token ?
```

Needs GCC for preprocessing and linking.

### 🏗️ Compiler Architecture and Pipeline

This compiler is built as a series of modular, interdependent passes to ensure separation of concerns and maintainability.

| Stage | Purpose | Key Accomplishments Demonstrated |
| :--- | :--- | :--- |
| **1. Lexer** | Converts the raw source code text into a stream of meaningful **Tokens** (e.g., identifiers, keywords, constants). | Handles different base constants, complex identifiers, and token stream storage. |
| **2. Parser** | Converts the token stream into an **Abstract Syntax Tree (AST)**, enforcing the grammar and operator precedence. | Mastery of recursive descent for complex C declarators, expressions, and control flow. |
| **3. Type Resolution** | Traverses the AST to perform semantic checks: verifying variable scope, confirming type validity, and handling **implicit/explicit type conversions**. | Implemented a robust **Symbol Table** to manage static/global/local scope and type system logic. |
| **4. IR Generation** | Translates the valid AST into a simpler **Intermediate Representation (IR)** for optimization and machine-independent processing. | Abstracted complex C concepts like `for`/`while` loops and switch statements into simple jump/label structures. |
| **5. Code Generation** | Converts the IR into **Assembly Code** (e.g., x86 or ARM) for the target architecture. | Handled register allocation, memory layout, and correct assembly generation for all control flow and function calls. |
| **6. Linker** | *Uses the external GCC toolchain to combine assembly with standard libraries into a final executable.* | *Indicates an understanding of the overall build process.* |

## Motivation

I knew that compilers are challenging projects and wanted to know more about them by building one for a real language.

## Usage

- `-h`               - Print help to the console.
- `--printTokens`    - Print the tokens produced by the lexer.
- `--printAst`       - Print the abstract syntax tree.
- `--printAstAfter`  - Print the converted abstract syntax tree afSemantic analysis.
- `--printTacky`     - Print the intermediate representation.
- `--printAsm`       - Print the assembly representation before register fixing.
- `--printAsmAfter`  - Print the assembly representation after register fixing.
- `--lex`            - Stop after the lexing stage.
- `--parse`          - Stop after the parsing stage.
- `--codegen`        - Stop after the writing the assembly file.