#pragma once

#include "netefi.h"

using namespace System;
using namespace System::IO;
using namespace System::Text;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Globalization;
using namespace System::Collections::Generic;

using namespace msclr::interop;

using namespace NetEFI;

public ref class Manager
{
private:
    static void Log( String ^, ... array<Object ^> ^ );

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

    // Ведение журнала сообщений.
    static void LogInfo( String ^, ... array<Object ^> ^ );
    static void LogError( String ^, ... array<Object ^> ^ );

    static void LogInfo( std::string );
    static void LogError( std::string );

    // Проверка типа библиотеки.
    static bool IsManagedAssembly( String ^ );

    // Настройка менеджера.
    static bool Initialize();

    // Загрузка пользовательских сборок.
    static bool LoadAssemblies();

    static PVOID CreateUserFunction( FunctionInfo ^, PVOID );

    static void CreateUserErrorMessageTable( array < String ^ > ^ );

    static void InjectCode( PBYTE &, int, int );
};
