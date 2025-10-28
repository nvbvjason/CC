Generates x86_64 for Linux
needs gcc for preprocessing and assembling

The grammar

<program>               ::= { <declaration> }
<declaration>           ::= <function_declaration> | <variable_declaration>
<variable_declaration>  ::= { <specifier> }+ <declarator> [ "=" <exp> ] ";"
<function_declaration>  ::= { <specifier> }+ <declarator> "(" <param-list> ")" ( <block> | ";" )
<declarator>            ::= "*" <declarator> | <direct-declarator>
<direct-declarator>     ::= <simple-declarator> [ <param-list> ]
<param-list>            ::= "(" "void ")" | "(" <param> [ "," <param> ] ")"
<param>                 ::= { <type-specifier> }+ <declarator>
<simple-declarator>     ::= <identifier> | "(" <declarator> ")"
<type-specifier>        ::= "int" | "long" | "unsigned" | "signed" | "double"
<specifier>             ::= <type-specifier> | "static" | "extern"
<block>                 ::= "{" { <block-item> } "}"
<block_item>            ::= <statement> | <declaration>
<for-init>              ::= <variable-declaration> | <exp> ";"
<statement>             ::= "return" <exp> ";"
                          | <exp> ";"
                          | "if" "(" <exp> ")" <statement> [ "else" <statement> ]
                          | "switch" ( <exp> ")" <statement>
                          | "goto" <identifier> ";"
                          | <block>
                          | "break" ";"
                          | "continue" ";"
                          | <identifier> ":" <statement>
                          | "case" <exp> ":" <statement>
                          | default ":" <statement>
                          | "while" "(" <exp> ")" <statement>
                          | "do" <statement> "while" "(" <exp> ")" ";"
                          | "for" "(" <for-inti> [ <exp> ] ";" [ <exp> ] ")" <statement>
                          | ";"
<exp>                   ::= <unary_exp>
                          | <exp> <binop> <exp>
                          | <exp> "?" <exp> ":" <exp>
<cast-exp>              ::= "(" { <type-specifier> }+ [ <abstract-declarator> ] ")" <cast-exp>
                          | <unary-exp>
<unary-exp>             ::= <postfix-exp> | <unop> <unary-exp>
<postfix-exp>           ::= <factor> | <postfix_exp> <postfixop>
<factor>                ::= <const>
                          | <identifier>
                          | <identifier> "(" [ <argument-list> ] ")"
                          | "(" <exp> ")"
<argument-list>         ::= <exp> { "," <exp> }
<abstract-declarator>   ::= "*" [ <abstract-declarator> ]
                          | <direct-abstract-declarator>
<direct-abstarct-declarator> ::= "(" <abstract-declarator> ")"
<unop>                  ::= "+" | "-" | "~" | "!" | "--" | "++" | "*" | "&"
<postfixop>             ::= "--" | "++"
<binop>                 ::= "-" | "+" | "*" | "/" | "%" | "^" | "<<" | ">>" | "&" | "|"
                          | "&&" | "||" | "==" | "!=" | "<" | "<=" | ">" | ">=" | "="
                          | "+=" | "-=" | "*=" | "/=" | "%="
                          | "&=" | "|=" | "^=" | "<<=" | ">>="
<const>                 ::= <int> | <long> | <uint> | <ulong> | <double>
<identifier>            ::= ? An identifier token ?
<int>                   ::= ? A int token ?
<long>                  ::= ? A int or long token ?
<uint>                  ::= ? An unsigned int token ?
<ulong>                 ::= ? An unsigned int or unsigned long token ?
<double>                ::= ? A floating-point constant token ?