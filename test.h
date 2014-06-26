#pragma once

#include "netefi.h"

using namespace System;
using namespace System::IO;
using namespace System::Text;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Globalization;
using namespace System::Collections::Generic;

using namespace std;


namespace NetEFI {

    public ref class test: public IFunction {

    private:
        FunctionInfo ^ info;

    protected:

        ~test() {}

    public:

        virtual property FunctionInfo^ Info {

            FunctionInfo^ get() { return info; }
        }

        !test() {}

        test() {

			info = gcnew FunctionInfo(
                
                "test", "x", "test(x)", 
                array<TComplex^,2>::typeid,
                gcnew array<Type^> { String::typeid }
            );
        }

        virtual FunctionInfo^ GetFunctionInfo(String^ lang) {

            return info;
        }

        virtual bool NumericEvaluation( array< Object^ > ^ args, [Out] Object ^ % result ) {

            //TComplex^ arg1 = ( TComplex^ ) args[0];

            //result = gcnew TComplex( arg1->Real + 10, arg1->Imaginary + 10 );

            String^ text = ( String^ ) args[0];

            Manager::LogInfo( text );

            //result = gcnew String( text );

            //Manager::LogInfo( String::Format( "{0}, {1}", arg1->Real, arg1->Imaginary ) );

            array<TComplex^,2>^ m = gcnew array<TComplex^,2>( 2, 2 );

            m[0,0] = gcnew TComplex( 1, 0 );
            m[0,1] = gcnew TComplex( 2, 0 );
            m[1,0] = gcnew TComplex( 3, 0 );
            m[1,1] = gcnew TComplex( 4, 0 );

            result = m;

            return true;
        }

    };

}

