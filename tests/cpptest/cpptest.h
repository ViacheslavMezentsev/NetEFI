#pragma once

using namespace System;
using namespace System::IO;
using namespace System::Text;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Globalization;
using namespace System::Collections::Generic;

using namespace NetEFI;

public ref class cpptest: public IFunction
{
public:

    virtual property FunctionInfo^ Info
    {
        FunctionInfo^ get()
        { 
            return gcnew FunctionInfo( "cpptest", "cmd", "return info",
                String::typeid, gcnew array<Type ^> { String::typeid }
            );
        }
    }

    virtual FunctionInfo^ GetFunctionInfo( String^ lang )
    {
        return Info;
    }

    virtual bool NumericEvaluation( array< Object^ > ^ args, [Out] Object ^ % result, Context ^ % context )
    {
        try
        {
            String^ cmd = ( String^ ) args[0];

            result = gcnew String( "help: info, list" );

            if ( cmd->Equals( gcnew String("info") ) )
            {
                auto name = Assembly::GetExecutingAssembly()->GetName();

                result = String::Format( "{0} {1}", name->Name, name->Version );
            }
            else if ( cmd->Equals( gcnew String("list") ) )
            {
                List<String^>^ list = gcnew List<String^>();

                array<Type^>^ types = Assembly::GetExecutingAssembly()->GetTypes();

                for each ( Type^ type in types )
                {
                    if ( !type->IsPublic || type->IsAbstract || !IFunction::typeid->IsAssignableFrom( type ) ) continue;

                    IFunction^ f = ( IFunction^ ) Activator::CreateInstance( type );
                        
                    list->Add( f->Info->Name );
                }

                result = String::Join( gcnew String( ", " ), list->ToArray() );
            }
        }
        catch ( ... )
        {
            result = nullptr;
            return false;
        }

        return true;
    }
};
