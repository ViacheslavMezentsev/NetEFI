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
                
                "test", "", "test(x, y)",
                array<TComplex^,2>::typeid,
                gcnew array<Type^> { TComplex::typeid, String::typeid }
            );
        }

        virtual FunctionInfo^ GetFunctionInfo(String^ lang) {

            return info;
        }

        virtual bool NumericEvaluation( array< Object^ > ^ args, [Out] Object ^ % result ) {

            //TComplex^ arg1 = ( TComplex^ ) args[0];

            //result = gcnew TComplex( arg1->Real + 10, arg1->Imaginary + 10 );

            array<TComplex^,2>^ m;

            try {

                TComplex^ n = ( TComplex^ ) args[0];
                String^ text = ( String^ ) args[1];

                Manager::LogInfo( String::Format( "{0}, {1}", n->Real, text ) );

                m = gcnew array<TComplex^,2>( ( int ) n->Real, ( int ) n->Real );

                //result = gcnew String( text );

                //Manager::LogInfo( String::Format( "{0}, {1}", arg1->Real, arg1->Imaginary ) );

                //array<TComplex^,2>^ m = gcnew array<TComplex^,2>( n->Real, n->Real );

                for ( int r = 0; r < m->GetLength(0); r++ ) {

                    for ( int c = 0; c < m->GetLength(1); c++ ) {

                        m[r, c] = gcnew TComplex( r, c );
                    }
                }

            } catch ( Exception^ ex ) {

                Manager::LogError( ex->Message );
                return false;
            }

            result = m;

            return true;
        }

    };

}

