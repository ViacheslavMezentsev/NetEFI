#pragma once

#include "netefi.h"

namespace NetEFI
{
    public ref class Manager
    {
    private:
        static void Log( String ^, ... array<Object ^> ^ );
        static bool IsManagedAssembly( String ^ );        
        static PVOID CreateUserFunction( FunctionInfo ^, PVOID );
        static void CreateUserErrorMessageTable( array < String ^ > ^ );
        static void InjectCode( PBYTE &, int, int );

    public:

        ~Manager() {}

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

        static void LogInfo( String ^ format, ... array<Object ^> ^ list ) { Log( "[INFO ] " + format, list ); }
        static void LogError( String ^ format, ... array<Object ^> ^ list ) { Log( "[ERROR] " + format, list ); }

        static bool LoadAssemblies();
        static bool Initialize();
    };
}
