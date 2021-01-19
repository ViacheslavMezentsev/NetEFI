#pragma once

using namespace System;
using namespace System::Text;

using namespace NetEFI;

public ref class cppsum: public IFunction
{
public:

    virtual property FunctionInfo^ Info
    {
        FunctionInfo^ get()
        { 
            return gcnew FunctionInfo( "cppsum", "a,b", "complex sum of scalars a and b",
                TComplex::typeid, gcnew array<Type^> {  TComplex::typeid,  TComplex::typeid } );
        }
    }

    virtual FunctionInfo^ GetFunctionInfo(String^ lang) { return Info; }

    virtual bool NumericEvaluation( array< Object^ > ^ args, [Out] Object ^ % result, Context ^ % context )
    {
        result = Evaluate( ( TComplex^ ) args[0], ( TComplex^ ) args[1] );

        return true;
    }

    TComplex^ Evaluate( TComplex^ a, TComplex^ b )
    {
        return gcnew TComplex( a->Real + b->Real, a->Imaginary + b->Imaginary );
    }
};
