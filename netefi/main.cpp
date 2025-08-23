// .NET programming with C++ / CLI
// https://docs.microsoft.com/ru-ru/cpp/dotnet/dotnet-programming-with-cpp-cli-visual-cpp
// C++/CLI Migration Primer.
// http://msdn.microsoft.com/ru-ru/library/ms235289.aspx
// C++/CLI Tasks.
// http://msdn.microsoft.com/ru-ru/library/hh875047.aspx
// Common Language Runtime Loader and DllMain
// http://msdn.microsoft.com/en-us/library/aa290048.aspx
// Component Extensions for Runtime Platforms
// http://msdn.microsoft.com/en-us/library/xey702bw.aspx
// Loading Mixed-Mode C++/CLI .dll (and dependencies) dynamically from unmanaged c++
// http://stackoverflow.com/questions/7016663/loading-mixed-mode-c-cli-dll-and-dependencies-dynamically-from-unmanaged-c
// How to check an object's type in C++/CLI?
// http://stackoverflow.com/questions/2410721/how-to-check-an-objects-type-in-c-cli

#include "stdafx.h"
#include "netefi.h"
#include "mcadincl.h"
#include "Manager.h"
#include "ContextImpl.h"

using namespace NetEFI;

extern CMathcadEfi MathcadEfi;


LRESULT ConvertInputs( IComputable ^ func, PVOID items[], array < Object ^ > ^ % args )
{
    int count = func->Info->ArgTypes->GetLength(0);

    // Convert each parameter to managed type.
    for ( int n = 0; n < count; n++ )
    {
        Type ^ type = func->Info->ArgTypes[n];

        void * item = items[ n + 1 ];

        if ( item == nullptr ) throw gcnew System::Exception( String::Format( "'{0}': argument {1} is NULL", func->Info->Name, n ) );

		// Convert to String, Complex or Complex array type.
        if ( type->Equals( String::typeid ) )
        {
            auto mcstr = static_cast<LPMCSTRING>( item );

            args[n] = safe_cast<String^>( *mcstr );
        }

        else if ( type->Equals( Complex::typeid ) )
        {
            args[n] = ( Complex ) ( * ( LPCOMPLEXSCALAR ) item );
        }

        else if ( type->Equals( array<Complex, 2>::typeid ) )
        {
            auto pmcArray = ( LPCOMPLEXARRAY ) item;

            unsigned int rows = pmcArray->rows;
            unsigned int cols = pmcArray->cols;

            if ( rows == 0 || cols == 0 )
                throw gcnew System::Exception( String::Format( "'{0}': wrong dimension for {1} argument: {2}x{3}", func->Info->Name, n, rows, cols ) );

            args[n] = ( array<Complex, 2>^ ) ( * pmcArray );
        }
        else throw gcnew System::Exception( String::Format( "'{0}': unknown argument type {1}: {2}", func->Info->Name, n, type->ToString() ) );
    }

    return S_OK;
}


LRESULT Evaluate( IComputable^ func, array<Object^>^ args, Object^% lvalue )
{
    // The main try-catch now only handles unexpected, critical exceptions.
    try
    {
        ContextImpl^ context = gcnew ContextImpl();

        // NumericEvaluation will now never throw an EFIException.
        // It will either return a normal value or an ErrorResult object.
        if ( !func->NumericEvaluation( args, lvalue, context ) )
        {
            // This path is now less likely, but we keep it for robustness.
            throw gcnew System::Exception( String::Format( "'{0}': numeric evaluation returned false", func->Info->Name ) );
        }

        // NEW LOGIC: Check if the returned object is our error marker.
        auto error = dynamic_cast< ErrorResult^ >( lvalue );

        if ( error != nullptr )
        {
            // It is an error! Now we perform the offset calculation.
            int offset = 0;

            for ( int k = 0; k <= MathcadEfi.AssemblyId; k++ )
            {
                auto assemblyInfo = Manager::Assemblies[k];

                if ( assemblyInfo == nullptr || assemblyInfo->Functions == nullptr ) continue;

                for ( int n = 0; n < assemblyInfo->Functions->Count; n++ )
                {
                    if ( k == MathcadEfi.AssemblyId && n >= MathcadEfi.FunctionId ) break;

                    auto funcobj = assemblyInfo->Functions[n];
                    auto errorsFieldInfo = funcobj->GetType()->GetField( "Errors", BindingFlags::GetField | BindingFlags::Static | BindingFlags::Public );
                    if ( errorsFieldInfo == nullptr ) continue;

                    try
                    {
                        auto errors = ( array<String^>^ )errorsFieldInfo->GetValue( nullptr );
                        if ( errors != nullptr ) offset += errors->GetLength( 0 );
                    }
                    catch ( ... ) {}
                }
            }

		    // Get the error message from the function.
            auto assemblyInfo = Manager::Assemblies[ MathcadEfi.AssemblyId ];

            if ( MathcadEfi.FunctionId < 0 || MathcadEfi.FunctionId >= assemblyInfo->Functions->Count )
                throw gcnew System::Exception( String::Format( "'{0}': function id {1} is out of range", func->Info->Name, MathcadEfi.FunctionId ) );

            auto funcobj = assemblyInfo->Functions[ MathcadEfi.FunctionId ];

            if ( funcobj == nullptr ) throw gcnew System::Exception( String::Format( "'{0}': function object is null", func->Info->Name ) );

            auto errorsFieldInfo = funcobj->GetType()->GetField( "Errors", BindingFlags::GetField | BindingFlags::Static | BindingFlags::Public );

            if ( errorsFieldInfo == nullptr )
                throw gcnew System::Exception( String::Format( "'{0}': can't find error table", func->Info->Name ) );

            auto errors = ( array < String ^ > ^ ) errorsFieldInfo->GetValue( nullptr );

            if ( errors == nullptr )
                throw gcnew System::Exception( String::Format( "'{0}': error table is null", func->Info->Name ) );

            if ( error->ErrorCode <= 0 || error->ErrorCode > errors->GetLength(0) )
                throw gcnew System::Exception( String::Format( "'{0}': can't find error message ({1})", func->Info->Name, error->ErrorCode ) );

            int count = func->Info->ArgTypes->GetLength(0);

            if ( error->ArgumentIndex < 0 || error->ArgumentIndex > count ) error->ArgumentIndex = 0;

            // See Mathcad Developer Reference for the details.
            return MAKELRESULT( offset + error->ErrorCode, error->ArgumentIndex );
        }
    }
    catch ( Exception^ ex ) // This now catches only CRITICAL, unexpected exceptions.
    {
        Manager::LogError( "An unhandled exception occurred in Evaluate for function '{0}'.", func->Info->Name );
        Manager::LogError( ex->ToString() );
        return E_FAIL;
    }

    return S_OK; // Normal, successful execution.
}


LRESULT ConvertOutput( IComputable ^ func, Object ^ lvalue, void * result )
{
    // Convert the result.
    Type ^ type = lvalue->GetType();

    // String to MCSTRING.
    if ( type->Equals( String::typeid ) )
    {
        auto pmcString = ( LPMCSTRING ) result;

        // .net unicode to ansi std::string.
        auto text = marshal_as< std::string >( ( String ^ ) lvalue );

        pmcString->str = ( char * ) MathcadEfi.MathcadAllocate( ( unsigned int ) text.length() + 1u );

        if ( !pmcString->str )
            throw gcnew System::Exception( String::Format( "'{0}': memory allocation for string failed", func->Info->Name ) );

        // std::string to char[].
        strcpy_s( pmcString->str, text.length() + 1u, ( const char * ) text.c_str() );
    }

    // Complex to COMPLEXSCALAR.
    else if ( type->Equals( Complex::typeid ) )
    {
        auto pmcScalar = ( LPCOMPLEXSCALAR ) result;

        Complex cmplx = ( Complex ) lvalue;

        pmcScalar->real = cmplx.Real;
        pmcScalar->imag = cmplx.Imaginary;
    }

    // Complex[,] to COMPLEXARRAY.
    else if ( type->Equals( array<Complex, 2>::typeid ) )
    {
        auto matrix = ( array<Complex, 2> ^ ) lvalue;

        if ( matrix == nullptr ) throw gcnew System::Exception( String::Format( "'{0}': return value is NULL", func->Info->Name ) );

        unsigned int rows = matrix->GetLength(0);
        unsigned int cols = matrix->GetLength(1);

        if ( rows == 0 || cols == 0 ) throw gcnew System::Exception( String::Format( "'{0}': wrong dimension for return value: {1}x{2}", func->Info->Name, rows, cols ) );

        bool bImag = false;

        // Check if imaginary part is empty.
        for each ( Complex c in matrix )
        {
            if ( c.Imaginary != 0.0 )
            {
                bImag = true;
                break;
            }
        }

        // The first parameter for the MathcadArrayAllocate() function
        // must be pointer to the COMPLEXARRAY structure.
        auto pmcArray = ( LPCOMPLEXARRAY ) result;

        MathcadEfi.MathcadArrayAllocate( pmcArray, rows, cols, TRUE, bImag ? TRUE : FALSE );

        for ( unsigned int row = 0; row < rows; row++ )
        {
            for ( unsigned int col = 0; col < cols; col++ )
            {
                pmcArray->hReal[ col ][ row ] = matrix[ row, col ].Real;

                if ( bImag )
                {
                    pmcArray->hImag[ col ][ row ] = matrix[ row, col ].Imaginary;
                }
            }
        }
    }
	else throw gcnew System::Exception( String::Format( "'{0}': unknown return type: {1}", func->Info->Name, type->ToString() ) );

    return S_OK;
}


// Helper function to safely clean up the return value pointer in case of an error.
void CleanupReturnValueOnError( IComputable^ func, PVOID returnValue )
{
    // Safety checks
    if ( func == nullptr || returnValue == nullptr )
    {
        return;
    }

    Type^ returnType = func->Info->ReturnType;

    if ( returnType->Equals( String::typeid ) )
    {
        // For strings, a null pointer is a safe state.
        ( ( LPMCSTRING ) returnValue )->str = nullptr;
    }
    else if ( returnType->Equals( Complex::typeid ) )
    {
        // For scalars, a zeroed struct is a safe state.
        auto scalar = ( LPCOMPLEXSCALAR ) returnValue;
        scalar->real = 0.0;
        scalar->imag = 0.0;
    }
    else if ( returnType->Equals( array<Complex, 2>::typeid ) )
    {
        // For arrays, MathcadArrayFree is the designated cleanup function.
        // It's safe to call even if nothing was allocated.
        MathcadEfi.MathcadArrayFree( ( LPCOMPLEXARRAY ) returnValue );
    }
}


// General function.
LRESULT UserFunction( PVOID items[] )
{
    IComputable^ func = nullptr;

    try
    {
		// Get the assembly and function information.
        auto assemblyInfo = Manager::Assemblies[ MathcadEfi.AssemblyId ];

        if ( MathcadEfi.FunctionId < 0 || MathcadEfi.FunctionId >= assemblyInfo->Functions->Count )
            throw gcnew System::Exception( "FunctionId is out of range" );

		// Get the function object.
        func = ( IComputable ^ ) assemblyInfo->Functions[ MathcadEfi.FunctionId ];

        if ( func == nullptr ) throw gcnew System::Exception( "Function object is null" );

        // Get the count of parameters.
        int count = func->Info->ArgTypes->GetLength(0);

        // Create an array of managed function parameters.
        auto args = gcnew array < Object ^ >( count );

		// Get items from the arguments array.
        auto result = ConvertInputs( func, items, args );

        if ( result != S_OK ) throw gcnew System::Exception( "Input conversion failed" );

        Object ^ lvalue;

        result = Evaluate( func, args, lvalue );

        if ( result == E_FAIL ) throw gcnew System::Exception( "Evaluation failed" );

        else if ( result != S_OK ) return result;

        result = ConvertOutput( func, lvalue, items[0] );

        if ( result == E_FAIL ) throw gcnew System::Exception( "Output conversion failed" );

        else if ( result != S_OK ) return result;
    }
    catch ( System::Exception ^ ex )
    {
        // Log the error.
        Manager::LogError( "AssemblyId: {0}, FunctionId: {1}, Exception: {2}", MathcadEfi.AssemblyId, MathcadEfi.FunctionId, ex->Message );
        CleanupReturnValueOnError( func, items[0] );
        return E_FAIL;
    }
	catch (...)
    {
        Manager::LogError( "AssemblyId: {0}, FunctionId: {1}", MathcadEfi.AssemblyId, MathcadEfi.FunctionId );
        CleanupReturnValueOnError( func, items[0] );
        return E_FAIL;
    }

    return S_OK;
}


/// <summary>
/// Registers the functions in the current AppDomain.
/// </summary>
void RegisterFunctions()
{
    // Prepare managed code. Setup our resolver for assembly loading.
    AppDomain::CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler( Manager::OnAssemblyResolve );

    if ( Manager::LoadAssemblies() ) Manager::RegisterFunctions();
}

#pragma unmanaged

// Простая, надежная, полностью неуправляемая функция для записи в лог-файл.
// Используется только для критических ошибок в DllEntryPoint.
void UnmanagedLog( const char* message )
{
    // Получаем путь к папке %APPDATA%
    char appDataPath[MAX_PATH];

    if ( SUCCEEDED( SHGetFolderPathA( NULL, CSIDL_APPDATA, NULL, 0, appDataPath ) ) )
    {
        // Формируем полный путь к лог-файлу
        std::string logPath = std::string( appDataPath ) + "\\Mathsoft\\Mathcad\\netefi_critical.log";

        // Открываем файл для добавления записи (append)
        std::ofstream logFile( logPath, std::ios_base::app );

        if ( logFile.is_open() )
        {
            // Получаем текущее время для записи в лог
            char timeStr[128];
            time_t now = time(0);
            tm tstruct;

            localtime_s( &tstruct, &now );
            strftime( timeStr, sizeof( timeStr ), "%Y-%m-%d %H:%M:%S", &tstruct );

            // Записываем сообщение
            logFile << timeStr << ": " << message << std::endl;
        }
    }
}

LRESULT CallbackFunction( void * out, ... )
{
    return ::UserFunction( & out );
}

// This is the entry point for the DLL.
BOOL WINAPI DllEntryPoint( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved )
{
    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        try
        {
            RegisterFunctions();
        }
        // Ловим ТОЛЬКО управляемые исключения, чтобы извлечь из них сообщение.
        catch ( const std::exception& ex )
        {
            // Это для стандартных исключений C++ (маловероятно, но возможно)
            UnmanagedLog( "Critical unmanaged C++ exception during initialization." );
            UnmanagedLog( ex.what() );
        }
        catch (...) // Ловим все остальное, включая управляемые исключения .NET
        {
            // Мы не можем безопасно получить здесь сообщение из .NET исключения,
            // но мы можем зафиксировать сам факт сбоя.
            UnmanagedLog( "Critical unknown (likely .NET) exception during initialization." );
        }
    }

    else if ( dwReason == DLL_PROCESS_DETACH )
    {
    }

    return TRUE;
}

#pragma managed
