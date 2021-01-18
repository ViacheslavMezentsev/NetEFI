// Основы миграции C++/CLI.
// http://msdn.microsoft.com/ru-ru/library/ms235289.aspx
// C++/CLI Tasks.
// http://msdn.microsoft.com/ru-ru/library/hh875047.aspx
// Managed Code and DllMain
// http://msdn.microsoft.com/en-us/library/aa290048(v=vs.71).aspx
// Component Extensions for Runtime Platforms
// http://msdn.microsoft.com/en-us/library/xey702bw.aspx

#include "stdafx.h"
#include "netefi.h"
#include "TComplex.h"
#include "Context.h"
#include "mcadincl.h"
#include "Manager.h"

using namespace NetEFI;

extern int assemblyId;
extern int functionId;

PCREATE_USER_FUNCTION CreateUserFunction;
PCREATE_USER_ERROR_MESSAGE_TABLE CreateUserErrorMessageTable;
PMATHCAD_ALLOCATE MathcadAllocate;
PMATHCAD_FREE MathcadFree;
PMATHCAD_ARRAY_ALLOCATE MathcadArrayAllocate;
PMATHCAD_ARRAY_FREE MathcadArrayFree;
PIS_USER_INTERRUPTED isUserInterrupted;

/// <summary>
/// This handler is called only when the CLR tries to bind to the assembly and fails
/// </summary>
/// <param name="sender">Event originator</param>
/// <param name="args">Event data</param>
/// <returns>The loaded assembly</returns>
Assembly ^ OnAssemblyResolve( Object ^ sender, ResolveEventArgs ^ args )
{
    Manager::LogInfo( "[OnAssemblyResolve] {0}", args->Name );

    Assembly ^ retval = nullptr;
    String ^ finalPath = nullptr;

    // Load the assembly from the specified path
    try
    {
        finalPath = args->Name->Substring( 0, args->Name->IndexOf( "," ) ) + gcnew String( ".dll" );

        finalPath = Path::Combine( Manager::AssemblyDirectory, finalPath );

        if ( File::Exists( finalPath ) )
        {
            retval = Assembly::LoadFile( finalPath );

            Manager::LogInfo( "Assembly loaded: {0}", finalPath );
        }
        else
        {
            Manager::LogInfo( "File not found: {0}", finalPath );
        }
    }
    catch ( System::Exception ^ ex )
    {
        Manager::LogError( "Assembly not loaded: {0}", ex->Message );
    }

    return retval;
}


void PrepareManagedCode()
{
    // Set up our resolver for assembly loading.
    AppDomain ^ currentDomain = AppDomain::CurrentDomain;

    currentDomain->AssemblyResolve += gcnew ResolveEventHandler( OnAssemblyResolve );
}


bool LoadAssemblies()
{
    // Loading Mixed-Mode C++/CLI .dll (and dependencies) dynamically from unmanaged c++
    // http://stackoverflow.com/questions/7016663/loading-mixed-mode-c-cli-dll-and-dependencies-dynamically-from-unmanaged-c
    PrepareManagedCode();

    return Manager::Initialize() ? Manager::LoadAssemblies() : false;
}


// Обобщённая функция.
LRESULT UserFunction( PVOID items[] )
{
    MCSTRING * pmcString;
    COMPLEXSCALAR * pmcScalar;
    COMPLEXARRAY * pmcArray;

    Type ^ type;
    AssemblyInfo ^ assemblyInfo = nullptr;
    IFunction ^ func = nullptr;

    try
    {
        assemblyInfo = Manager::Assemblies[ assemblyId ];

        func = assemblyInfo->Functions[ functionId ];
    }
    catch ( ... )
    {
        Manager::LogError( "assemblyId: {0}, functionId: {1}", assemblyId, functionId );

        return E_FAIL;
    }

    // Узнаём общее число параметров функции.
    int Count = func->Info->ArgTypes->GetLength(0);

    // Создаём управляемый массив параметров функции.
    array < Object ^ > ^ args = gcnew array < Object ^ >( Count );

    // Преобразуем каждый тип параметра к управляемому аналогу.
    for ( int n = 0; n < Count; n++ )
    {
        type = func->Info->ArgTypes[n];

        // MCSTRING
        // TODO: Преобразовывать ansi в unicode.
        if ( type->Equals( String::typeid ) )
        {
            pmcString = ( MCSTRING * ) items[ n + 1 ];

            args[n] = marshal_as<String ^>( pmcString->str );
        }

        // COMPLEXSCALAR
        else if ( type->Equals( TComplex::typeid ) )
        {
            pmcScalar = ( COMPLEXSCALAR * ) items[ n + 1 ];

            args[n] = gcnew TComplex( pmcScalar->real, pmcScalar->imag );
        }

        // COMPLEXARRAY
        else if ( type->Equals( array<TComplex ^, 2>::typeid ) )
        {
            pmcArray = ( COMPLEXARRAY * ) items[ n + 1 ];

            int rows = pmcArray->rows;
            int cols = pmcArray->cols;

            array<TComplex ^, 2> ^ Matrix = gcnew array<TComplex ^, 2>( rows, cols );

            TComplex ^ tmp;

            for ( int row = 0; row < rows; row++ )
            {
                for ( int col = 0; col < cols; col++ )
                {
                    if ( ( pmcArray->hReal != NULL ) && ( pmcArray->hImag != NULL ) )
                        tmp = gcnew TComplex( pmcArray->hReal[ col ][ row ], pmcArray->hImag[ col ][ row ] );

                    if ( ( pmcArray->hReal != NULL ) && ( pmcArray->hImag == NULL ) )
                        tmp = gcnew TComplex( pmcArray->hReal[ col ][ row ], 0.0 );

                    if ( ( pmcArray->hReal == NULL ) && ( pmcArray->hImag != NULL ) )
                        tmp = gcnew TComplex( 0.0, pmcArray->hImag[ col ][ row ] );

                    Matrix[ row, col ] = tmp;
                }
            }

            args[n] = Matrix;
        }
        else
        {
            Manager::LogError( "[{0}] Unknown argument type {1}: {2}", func->Info->Name, n, type->ToString() );

            return E_FAIL;
        }
    }

    // Вызываем функцию.
    // TODO: Сделать вызов в отдельном потоке.
    Object ^ result;
    Context ^ context = gcnew Context();

    try
    {
        if ( !func->NumericEvaluation( args, result, context ) ) return E_FAIL;
    }
    catch ( EFIException ^ ex )
    {
        if ( assemblyInfo->Errors == nullptr )
        {
            Manager::LogError( "[{0}] {1}", func->Info->Name, "Errors table is empty" );

            return E_FAIL;
        }

        if ( ( ex->ErrNum > 0 )
            && ( ex->ErrNum <= assemblyInfo->Errors->GetLength(0) )
            && ( ex->ArgNum > 0 )
            && ( ex->ArgNum <= func->Info->ArgTypes->GetLength(0) ) )
        {
            // Рассчитываем положение таблицы ошибок для текущей сборки
            // в общей зарегистрированной таблице.
            int offset = 0;

            for ( int n = 0; n < assemblyId; n++ )
            {
                offset += assemblyInfo->Errors->GetLength(0);
            }

            // См. Руководство разработчика библиотек пользователя.
            return MAKELRESULT( offset + ex->ErrNum, ex->ArgNum );
        }
        else
        {
            Manager::LogError( "[{0}] {1}", func->Info->Name, ex->Message );

            return E_FAIL;
        }

    }
    catch ( System::Exception ^ ex )
    {
        Manager::LogError( "[{0}] {1}", func->Info->Name, ex->Message );

        return E_FAIL;
    }

    // Преобразуем результат.
    type = result->GetType();

    // MCSTRING
    // TODO: Преобразовывать unicode в ansi.
    if ( type->Equals( String::typeid ) )
    {
        // Выделяем память под структуру.
        pmcString = ( MCSTRING * ) items[0];

        marshal_context context;

        char * text = ( char * ) context.marshal_as<const char *>( ( String ^ ) result );

        // Выделяем память для строки и завершающего нуля.
        pmcString->str = ( char * ) ::MathcadAllocate( ::strlen( text ) + 1 );

        ::memset( pmcString->str, 0, ::strlen( text ) + 1 );

        // Копируем строку из временной области памяти.
        ::memcpy( pmcString->str, text, ::strlen( text ) );        
    }

    // COMPLEXSCALAR
    else if ( type->Equals( TComplex::typeid ) )
    {
        pmcScalar = ( COMPLEXSCALAR * ) items[0];

        TComplex ^ Number = ( TComplex ^ ) result;

        pmcScalar->real = Number->Real;
        pmcScalar->imag = Number->Imaginary;
    }

    // COMPLEXARRAY
    else if ( type->Equals( array<TComplex ^, 2>::typeid ) )
    {
        array<TComplex ^, 2> ^ Matrix = ( array<TComplex ^, 2> ^ ) result;

        // Согласно документации в функцию MathcadArrayAllocate() должна передаваться
        // ссылка на заполненную структуру COMPLEXARRAY.
        pmcArray = ( COMPLEXARRAY * ) items[0];

        int rows = Matrix->GetLength(0);
        int cols = Matrix->GetLength(1);

        bool bReal = false;
        bool bImag = false;

        // Проверка наличия действительных частей.           
        for ( int row = 0; row < rows; row++ )
        {
            for ( int col = 0; col < cols; col++ )
            {
                if ( ( ( TComplex ^ ) Matrix[ row, col ] )->Real != 0.0 )
                {
                    bReal = true;
                    break;
                }
            }
            
            if ( bReal == true ) break;
        }

        // Проверка наличия мнимых частей.
        for ( int row = 0; row < rows; row++ )
        {
            for ( int col = 0; col < cols; col++ )
            {
                if ( ( ( TComplex ^ ) Matrix[ row, col ] )->Imaginary != 0.0 )
                {
                    bImag = true;
                    break;
                }
            }

            if ( bImag == true ) break;
        }

        if ( ( ( bReal == true ) && ( bImag == true ) ) || ( ( bReal == false ) && ( bImag == true ) ) )
        {
            ::MathcadArrayAllocate( pmcArray, rows, cols, TRUE, TRUE );

            for ( int row = 0; row < rows; row++ )
            {
                for ( int col = 0; col < cols; col++ )
                {
                    ( ( COMPLEXARRAY * ) pmcArray )->hReal[ col ][ row ] = ( ( TComplex ^ ) Matrix[ row, col ] )->Real;
                    ( ( COMPLEXARRAY * ) pmcArray )->hImag[ col ][ row ] = ( ( TComplex ^ ) Matrix[ row, col ] )->Imaginary;
                }
            }
        }

        else if ( ( ( bReal == true ) && ( bImag == false ) ) || ( ( bReal == false ) && ( bImag == false ) ) )
        {
            ::MathcadArrayAllocate( pmcArray, rows, cols, TRUE, FALSE );

            for ( int row = 0; row < rows; row++ )
            {
                for ( int col = 0; col < cols; col++ )
                {
                    ( ( COMPLEXARRAY * ) pmcArray )->hReal[ col ][ row ] = ( ( TComplex ^ ) Matrix[ row, col ] )->Real;
                }
            }
        }
    }
    else
    {
        Manager::LogError( "[{0}] Unknown return type: {1}", func->Info->Name, type->ToString() );

        return E_FAIL;
    }

    return S_OK;
}


#pragma unmanaged

LRESULT CallbackFunction( void * out, ... )
{
    return ::UserFunction( & out );
}


// Common Language Runtime Loader and DllMain
// http://msdn.microsoft.com/en-us/library/aa290048(v=vs.71).aspx
BOOL WINAPI DllEntryPoint( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved ) 
{
    switch ( dwReason )
    {
        // DLL проецируется на адресное пространство процесса
        case DLL_PROCESS_ATTACH:
        { 
            try
            {
                LoadAssemblies();
            }
            catch (...) {}

            break;
        }

        // Создаётся поток.
        case DLL_THREAD_ATTACH: { break; }

        // Поток корректно завершается.
        case DLL_THREAD_DETACH: { break; }

        // DLL отключается от адресного пространства процесса.
        case DLL_PROCESS_DETACH: { break; }
    }

    // Используется только для DLL_PROCESS_ATTACH.
    return TRUE;
}

#pragma managed
