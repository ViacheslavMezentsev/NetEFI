#pragma once

#include "netefi.h"

namespace NetEFI
{
    public ref class Manager
    {
    private:
        static void Log( String ^ );
        static void Log( String ^, ... array<Object ^> ^ );
        static bool IsManagedAssembly( String ^ );        
        static PVOID CreateUserFunction( FunctionInfo ^, PVOID );
        static void CreateUserErrorMessageTable( array < String ^ > ^ );
        static void InjectCode( PBYTE &, int, int );

    public:

        ~Manager() {}

        static List < AssemblyInfo ^ > ^ Assemblies = LoadAssemblies();

        static property Assembly ^ ExecAssembly
        {
            Assembly ^ get()
            {
                return Assembly::GetExecutingAssembly();
            };
        };

        static property String ^ AssemblyDirectory
        {
            String ^ get()
            {
                return Path::GetDirectoryName( ( gcnew System::Uri( ExecAssembly->CodeBase ) )->LocalPath );
            };
        };

        static property String ^ AssemblyFileName
        {
            String ^ get()
            {
                return Path::GetFileName( ( gcnew System::Uri( ExecAssembly->CodeBase ) )->LocalPath );
            };
        };

        static property String ^ MathcadAppData
        {
            String ^ get()
            {
                return Path::Combine( Environment::GetFolderPath( Environment::SpecialFolder::ApplicationData ), "Mathsoft\\Mathcad" );
            };
        };

        static property String ^ LogFile
        {
            String ^ get()
            {
                return Path::Combine( MathcadAppData, Path::GetFileNameWithoutExtension( AssemblyFileName ) + ".log" );
            };
        };


    public:

        static void LogInfo( String ^ text ) { Log( "[INFO ] " + text ); }
        static void LogInfo( String ^ format, ... array<Object ^> ^ list ) { Log( "[INFO ] " + format, list ); }

        static void LogError( String ^ text ) { Log( "[ERROR] " + text ); }
        static void LogError( String ^ format, ... array<Object ^> ^ list ) { Log( "[ERROR] " + format, list ); }

        static bool Initialize();
        static bool RegisterFunctions();
        static List < AssemblyInfo ^ > ^ LoadAssemblies();
    };

    public ref class netefi : public IFunction
    {
    public:

        virtual property FunctionInfo ^ Info
        {
            FunctionInfo ^ get()
            {
                return gcnew FunctionInfo( Manager::ExecAssembly->GetName()->Name, "cmd", "return string",
                    String::typeid, gcnew array<Type ^> { String::typeid } );
            }
        }

        virtual FunctionInfo ^ GetFunctionInfo( String ^ lang )
        {
            return Info;
        }

        virtual bool NumericEvaluation( array< Object ^ > ^ args, [ Out ] Object ^% result, Context ^% context )
        {
            String ^ cmd = ( String ^ ) args[0];

            result = gcnew String( "help: info, os, net, author, email, list" );

            if ( cmd->Equals( gcnew String( "info" ) ) )
            {
                bool is64Bit = Marshal::SizeOf( IntPtr::typeid ) == 8;

                auto name = Assembly::GetExecutingAssembly()->GetName();

                auto version = name->Version;

                DateTime ^ bdate = ( gcnew DateTime( 2000, 1, 1 ) )->AddDays( version->Build );

                bdate = bdate->AddSeconds( 2 * version->Revision );

                result = String::Format( "{0}: {1}-bit, {2}, {3:dd-MMM-yyyy HH:mm:ss}", name->Name, ( is64Bit ? "64" : "32" ), version, bdate );
            }

            else if ( cmd->Equals( gcnew String( "os" ) ) )
            {
                result = Environment::OSVersion->ToString();
            }

            else if ( cmd->Equals( gcnew String( "net" ) ) )
            {
                result = Environment::Version->ToString();
            }

            else if ( cmd->Equals( gcnew String( "author" ) ) )
            {
                result = gcnew String( "Viacheslav N. Mezentsev" );
            }

            else if ( cmd->Equals( gcnew String( "email" ) ) )
            {
                result = gcnew String( "viacheslavmezentsev@ya.ru" );
            }

            else if ( cmd->Equals( gcnew String( "list" ) ) )
            {
                List<String^>^ list = gcnew List<String ^>();

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
