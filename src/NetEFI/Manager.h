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
        static HashSet<Type^>^ SupportedTypes;

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
}
