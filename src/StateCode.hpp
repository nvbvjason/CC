#pragma once

enum class StateCode {
    Done = 0,
    Continue,
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
    AsmFileWrite,
    ERROR_UNKNOWN
};

std::string to_string(StateCode code);

inline std::string to_string(StateCode code)
{
    switch (code) {
        case StateCode::Done:                       return "Done";
        case StateCode::Continue:                   return "Continue";
        case StateCode::NoInputFile:                return "Error No input file";
        case StateCode::FileNotFound:               return "Error File not found";
        case StateCode::InvalidCommandlineArgs:     return "Error Invalid commandline arguments";
        case StateCode::Lexer:                      return "Error Lexer";
        case StateCode::Parser:                     return "Error Parser";
        case StateCode::LValueVerification:         return "Error LValue verification";
        case StateCode::ValidateReturn:             return "Error Validate return";
        case StateCode::VariableResolution:         return "Error Variable resolution";
        case StateCode::LabelsUnique:               return "Error Labels unique";
        case StateCode::LoopLabeling:               return "Error Loop labeling";
        case StateCode::Switch:                     return "Error Switch";
        case StateCode::Codegen:                    return "Error Codegen";
        case StateCode::TypeResolution:             return "Error TypeResolution";
        case StateCode::AsmFileWrite:               return "Error Assembly File Write";
        default:                                    return "Error Unknown";
    }
}