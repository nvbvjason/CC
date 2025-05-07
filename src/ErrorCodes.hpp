#pragma once

#ifndef CC_ERROR_CODES_HPP
#define CC_ERROR_CODES_HPP

enum class ErrorCode {
    OK = 0,
    NoInputFile = 1,
    FileNotFound = 2,
    InvalidCommandlineArgs = 3,
    Lexer = 4,
    Parser = 5,
    LValueVerification = 6,
    ValidateReturn = 7,
    VariableResolution = 8,
    LabelsUnique = 9,
    LoopLabeling = 10,
    Switch = 11,
    Codegen = 12,
    ERROR_UNKNOWN
};

#endif //CC_ERROR_CODES_HPP
