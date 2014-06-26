#pragma once

using namespace System;
using namespace System::IO;
using namespace System::Text;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Globalization;
using namespace System::Collections::Generic;

using namespace std;
using namespace NetEFI;


namespace Functions {

    public ref class cpptest: public IFunction {

    private:
        FunctionInfo ^ info;

    protected:

        ~cpptest() {}

    public:

        virtual property FunctionInfo^ Info {

            FunctionInfo^ get() { return info; }
        }

        !cpptest() {}

        cpptest() {

			info = gcnew FunctionInfo(
                
                "cpptest", "cmd", "return info",
                ( gcnew Uri( Assembly::GetExecutingAssembly()->CodeBase ) )->LocalPath, 
                String::typeid,
                gcnew array<Type^> { String::typeid }
            );
        }

        virtual FunctionInfo^ GetFunctionInfo(String^ lang) {

            return info;
        }

        virtual bool NumericEvaluation( array< Object^ > ^ args, [Out] Object ^ % result ) {            

            try {

                String^ cmd = ( String^ ) args[0];

                result = gcnew String( "empty" );

                if ( cmd->Equals( gcnew String("info") ) ) {

                    result = Assembly::GetExecutingAssembly()->ToString();

                } else if ( cmd->Equals( gcnew String("list") ) ) {

                    List<String^>^ list = gcnew List<String^>();

                    array<Type^>^ types = Assembly::GetExecutingAssembly()->GetTypes();

                    for each ( Type^ type in types ) {

                        if ( !type->IsPublic || type->IsAbstract || !IFunction::typeid->IsAssignableFrom( type ) ) continue;

                        IFunction^ f = ( IFunction^ ) Activator::CreateInstance( type );
                        
                        list->Add( f->Info->Name );
                    }

                    result = String::Join( gcnew String( "," ), list->ToArray() );
                }

            } catch ( Exception^ ex ) {

                result = nullptr;
                return false;
            }

            return true;
        }

    };

}

