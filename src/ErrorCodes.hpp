#pragma once

#ifndef CC_ERROR_CODES_HPP
#define CC_ERROR_CODES_HPP

enum class ErrorCode {
    OK = 0,
    NoInputFile,
    FileNotFound,
    InvalidCommandlineArgs,
    Lexer,
    Parser,
    LValueVerification,
    ValidateReturn,
    VariableResolution,
    TypeResolution,
    LabelsUnique,
    LoopLabeling,
    Switch,
    Codegen,
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
        case ErrorCode::Switch:                     return "Error Switch";
        case ErrorCode::Codegen:                    return "Error Codegen";
        case ErrorCode::TypeResolution:              return "Error TypeResolution";
        default:                                    return "Error Unknown";
    }
}

#endif //CC_ERROR_CODES_HPP
