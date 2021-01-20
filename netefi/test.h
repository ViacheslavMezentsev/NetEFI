#pragma once

#include "netefi.h"
#include "Manager.h"

namespace NetEFI
{
    public ref class test: public IFunction
    {
    public:

        virtual property FunctionInfo^ Info
        {
            FunctionInfo^ get()
            { 
                return gcnew FunctionInfo( "test", "", "test(x, y)",
                    array<Complex ^, 2>::typeid, gcnew array<Type ^> { Complex::typeid, String::typeid } );
            }
        }

        virtual FunctionInfo^ GetFunctionInfo( String^ lang ) { return Info; }

        virtual bool NumericEvaluation( array< Object^ > ^ args, [Out] Object ^ % result, Context ^ % )
        {
            //Complex^ arg1 = ( Complex^ ) args[0];

            //result = gcnew Complex( arg1->Real + 10, arg1->Imaginary + 10 );

            array<Complex^,2>^ m;

            try
            {
                Complex^ n = ( Complex^ ) args[0];
                String^ text = ( String^ ) args[1];

                Manager::LogInfo( String::Format( "{0}, {1}", n->Real, text ) );

                m = gcnew array<Complex^,2>( ( int ) n->Real, ( int ) n->Real );

                //result = gcnew String( text );

                //Manager::LogInfo( String::Format( "{0}, {1}", arg1->Real, arg1->Imaginary ) );

                //array<Complex^,2>^ m = gcnew array<Complex^,2>( n->Real, n->Real );

                for ( int r = 0; r < m->GetLength(0); r++ )
                {
                    for ( int c = 0; c < m->GetLength(1); c++ )
                    {
                        m[r, c] = gcnew Complex( r, c );
                    }
                }
            } catch ( Exception^ ex )
            {
                Manager::LogError( ex->Message );

                return false;
            }

            result = m;

            return true;
        }
    };
}
