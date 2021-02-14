#pragma once

#include "netefi.h"

class CMathcadEfi
{
public:
    bool Attached;
    int AssemblyId;
    int FunctionId;
    PBYTE DynamicCode;

    PCREATE_USER_FUNCTION CreateUserFunction;
    PCREATE_USER_ERROR_MESSAGE_TABLE CreateUserErrorMessageTable;
    PMATHCAD_ALLOCATE MathcadAllocate;
    PMATHCAD_FREE MathcadFree;
    PMATHCAD_ARRAY_ALLOCATE MathcadArrayAllocate;
    PMATHCAD_ARRAY_FREE MathcadArrayFree;
    PIS_USER_INTERRUPTED isUserInterrupted;

public:
    ~CMathcadEfi();
    CMathcadEfi();
};

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

        static List < AssemblyInfo ^ > ^ Assemblies;

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
        static Assembly ^ OnAssemblyResolve( Object ^, ResolveEventArgs ^ );

        static void LogInfo( String ^ text ) { Log( "[INFO ] " + text ); }
        static void LogInfo( String ^ format, ... array<Object ^> ^ list ) { Log( "[INFO ] " + format, list ); }

        static void LogError( String ^ text ) { Log( "[ERROR] " + text ); }
        static void LogError( String ^ format, ... array<Object ^> ^ list ) { Log( "[ERROR] " + format, list ); }

        static bool Initialize();
        static bool LoadAssemblies();
        static bool RegisterFunctions();        
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
