#pragma once

#include "stdafx.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Collections::Generic;
using namespace System::Linq;

using namespace NetEFI::Computables;
using namespace NetEFI::Design;
using namespace NetEFI::Functions;

namespace cpptest
{
    [Computable( "cpptest", "cmd", "A utility function to inspect the C++/CLI test assembly." )]
    public ref class CppTest : public MathcadFunction<String^, String^>
    {
    public:
        virtual String^ Execute( String^ cmd, Context^ context ) override
        {
            auto assembly = Assembly::GetExecutingAssembly();

            try
            {
                if ( cmd->Equals( "info", StringComparison::OrdinalIgnoreCase ) )
                {
                    auto name = assembly->GetName();
                    return String::Format( "{0}: {1}", name->Name, name->Version );
                }

                if ( cmd->Equals( "list", StringComparison::OrdinalIgnoreCase ) )
                {
                    auto list = gcnew List<String^>();
                    auto types = assembly->GetTypes();

                    for each ( Type ^ type in types )
                    {
                        if ( type->IsPublic && !type->IsAbstract && MathcadFunctionBase::typeid->IsAssignableFrom( type ) )
                        {
                            array<Object^>^ attributes = type->GetCustomAttributes( ComputableAttribute::typeid, false );
                            if ( attributes->Length > 0 )
                            {
                                auto attr = safe_cast< ComputableAttribute^ >( attributes[0] );
                                list->Add( attr->Name );
                            }
                        }
                    }
                    return String::Join( ", ", list );
                }
            }
            catch ( Exception^ ex )
            {
                return String::Format( "ERROR: {0}", ex->Message );
            }

            return "help: info, list";
        }
    };
}