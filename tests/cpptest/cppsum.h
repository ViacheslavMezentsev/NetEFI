#pragma once

public ref class cppsum: public IComputable
{
public:

    virtual property FunctionInfo^ Info
    {
        FunctionInfo^ get()
        { 
            return gcnew FunctionInfo( "cppsum", "a,b", "complex sum of scalars a and b",
                Complex::typeid, gcnew array<Type^> {  Complex::typeid,  Complex::typeid } );
        }
    }

    virtual FunctionInfo^ GetFunctionInfo( String^ lang ) { return Info; }

    virtual bool NumericEvaluation( array< Object^ > ^ args, [Out] Object ^ % result, Context ^ context )
    {
        result = ( Complex ) args[0] + ( Complex ) args[1];

        return true;
    }
};
