#pragma once

using namespace System;
using namespace System::Text;

using namespace NetEFI;

public ref class cpperror: public IFunction {

public:

    static array<String^>^ Errors = gcnew array<String^>(4) { 
        gcnew String( "cpperror 1" ), gcnew String( "cpperror 2" ), 
        gcnew String( "cpperror 3" ), gcnew String( "cpperror 4" ) };

    virtual property FunctionInfo^ Info {

        FunctionInfo^ get() { 
            
            return gcnew FunctionInfo( "cpperror", "s", "return string",
                String::typeid, gcnew array<Type^> { TComplex::typeid } );
        }
    }

    virtual FunctionInfo^ GetFunctionInfo(String^ lang) { return Info; }

    virtual bool NumericEvaluation( array< Object^ > ^ args, [Out] Object ^ % result, Context ^ % context ) {

        throw gcnew EFIException( ( int ) ( ( TComplex ^ ) args[0] )->Real, 1 );
    }

};
