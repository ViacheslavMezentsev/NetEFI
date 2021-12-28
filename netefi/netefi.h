#pragma once

#include "stdafx.h"

namespace NetEFI
{
    public ref class EFIException : Exception
    {
    public:

        int ArgNum;
        int ErrNum;

        EFIException( int errNum, int argNum )
        {
            ErrNum = errNum;
            ArgNum = argNum;
        }
    };

    public ref class FunctionInfo
    {
    public:

        String ^ Name;
        String ^ Parameters;
        String ^ Description;
        Type ^ ReturnType;
        array < Type ^ > ^ ArgTypes;

        FunctionInfo( String ^ name, String ^ params, String ^ descr, Type ^ returnType, array < Type ^ > ^ argTypes )
        {
            Name = name;
            Parameters = params;
            Description = descr;
            ReturnType = returnType;
            ArgTypes = argTypes;
        }
    };

    ref class Context;

    public interface class IFunction
    {
    public:

        property FunctionInfo ^ Info { FunctionInfo ^ get(); }

        FunctionInfo ^ GetFunctionInfo( String ^ lang );
        bool NumericEvaluation( array < Object ^ > ^, [ Out ] Object ^%, Context ^% );
    };

    public ref class AssemblyInfo
    {
    public:
        String ^ Path;
        List < Object ^ > ^ Functions;

        AssemblyInfo( String ^ path )
        {
            Path = path;
            Functions = gcnew List< Object ^ >();
        }
    };
}
