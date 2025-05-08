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

std::string to_string(ErrorCode code);

inline std::string to_string(ErrorCode code)
{
    switch (code) {
        case ErrorCode::OK:                         return "OK";
        case ErrorCode::NoInputFile:                return "Error No input file";
        case ErrorCode::FileNotFound:               return "Error File not found";
        case ErrorCode::InvalidCommandlineArgs:     return "Error Invalid commandline arguments";
        case ErrorCode::Lexer:                      return "Error Lexer";
        case ErrorCode::Parser:                     return "Error Parser";
        case ErrorCode::LValueVerification:         return "Error LValue verification";
        case ErrorCode::ValidateReturn:             return "Error Validate return";
        case ErrorCode::VariableResolution:         return "Error Variable resolution";
        case ErrorCode::LabelsUnique:               return "Error Labels unique";
        case ErrorCode::LoopLabeling:               return "Error Loop labeling";
        default:                                    return "Error Unknown";
    }
}

#endif //CC_ERROR_CODES_HPP
