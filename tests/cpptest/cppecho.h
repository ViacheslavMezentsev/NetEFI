#pragma once

using namespace System;
using namespace System::Text;

using namespace NetEFI;

public ref class cppecho: public IFunction
{
public:

    virtual property FunctionInfo^ Info
    {
        FunctionInfo^ get()
        {             
            return gcnew FunctionInfo( "cppecho", "s", "return string",
                String::typeid, gcnew array<Type^> { String::typeid } );
        }
    }

    virtual FunctionInfo^ GetFunctionInfo(String^ lang)
    { 
        return Info;
    }

    virtual bool NumericEvaluation( array< Object^ > ^ args, [Out] Object ^ % result, Context ^ % context )
    {
        result = args[0];

        return true;
    }
};
