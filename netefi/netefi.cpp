#include "stdafx.h"
#include "Manager.h"

namespace NetEFI
{
    public ref class netefi : public IComputable
    {
    public:

        virtual property FunctionInfo ^ Info
        {
            FunctionInfo^ get()
            {
                return gcnew FunctionInfo( Manager::ExecAssembly->GetName()->Name, "cmd", "return string",
                    String::typeid, gcnew array<Type ^> { String::typeid } );
            }
        }

        virtual FunctionInfo^ GetFunctionInfo( String ^ lang )
        {
            return Info;
        }

        virtual bool NumericEvaluation( array< Object ^ > ^ args, [ Out ] Object ^% result, Context ^ context )
        {
            result = "help: info, os, net, author, email, list";

            auto cmd = ( String ^ ) args[0];

            if ( cmd == "info" )
            {
                auto aname = Assembly::GetExecutingAssembly()->GetName();

                auto version = aname->Version;

                auto bdate = ( gcnew DateTime( 2000, 1, 1 ) )->AddDays( version->Build ).AddSeconds( 2 * version->Revision );

                result = String::Format( "{0}: {1}-bit, {2}, {3:dd-MMM-yyyy HH:mm:ss}", aname->Name, ( Environment::Is64BitProcess ? "64" : "32" ), version, bdate );
            }

            else if ( cmd == "os" ) result = Environment::OSVersion->ToString();

            else if ( cmd == "net" ) result = Environment::Version->ToString();

            else if ( cmd == "author" ) result = "Viacheslav N. Mezentsev";

            else if ( cmd == "email" ) result = "viacheslavmezentsev@ya.ru";

            else if ( cmd == "list" )
            {
                auto list = gcnew List<String ^>();

                for each ( auto info in Manager::Assemblies )
                {
                    if ( info->Path->Contains( Manager::AssemblyFileName ) ) continue;

                    list->Add( Path::GetFileNameWithoutExtension( info->Path ) );
                }

                result = String::Join( ", ", list->ToArray() );
            }

            return true;
        }
    };
}
