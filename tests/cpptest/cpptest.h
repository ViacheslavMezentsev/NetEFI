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
        result = "help: info, list";

        auto assembly = Assembly::GetExecutingAssembly();

        try
        {
            auto cmd = ( String^ ) args[0];

            if ( cmd == "info" )
            {
                auto name = assembly->GetName();

                result = String::Format( "{0}: {1}", name->Name, name->Version );
            }

            else if ( cmd == "list" )
            {
                auto list = gcnew List<String^>();

                auto types = assembly->GetTypes();

                for each ( Type^ type in types )
                {
                    if ( !type->IsPublic || type->IsAbstract || !IFunction::typeid->IsAssignableFrom( type ) ) continue;

                    auto f = ( IFunction^ ) Activator::CreateInstance( type );
                        
                    list->Add( f->Info->Name );
                }

                result = String::Join( ", ", list->ToArray() );
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
