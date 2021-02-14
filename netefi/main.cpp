﻿// .NET programming with C++ / CLI
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
#include <msclr\marshal_cppstd.h>
#include "netefi.h"
#include "mcadincl.h"
#include "Context.h"
#include "Manager.h"

using namespace msclr::interop;
using namespace NetEFI;

extern CMathcadEfi MathcadEfi;


LRESULT ConvertInputs( IFunction ^ func, PVOID items[], array < Object ^ > ^ % args )
{
    int count = func->Info->ArgTypes->GetLength(0);

    // Convert each parameter to managed type.
    for ( int n = 0; n < count; n++ )
    {
        Type ^ type = func->Info->ArgTypes[n];

        void * item = items[ n + 1 ];

        // MCSTRING to String.
        // It seems that MCSTRING contains only one byte from unicode wchar.
        if ( type->Equals( String::typeid ) )
        {           
            // ANSI char[] to std::string.
            std::string text( ( ( LPMCSTRING ) item )->str );

            // std::string to .net unicode.
            args[n] = marshal_as<String ^>( text );
            
            /*
            size_t len = strlen( pmcString->str );

            auto bytes = gcnew array<Byte>( len );

            Marshal::Copy( IntPtr( pmcString->str ), bytes, 0, bytes->Length );

            args[n] = UTF8Encoding::UTF8->GetString( bytes );           
            */
        }

        // COMPLEXSCALAR to Complex;
        else if ( type->Equals( Complex::typeid ) )
        {
            auto pmcScalar = ( LPCOMPLEXSCALAR ) item;

            args[n] = Complex( pmcScalar->real, pmcScalar->imag );
        }

        // COMPLEXARRAY to Complex[,].
        else if ( type->Equals( array<Complex, 2>::typeid ) )
        {
            if ( item == nullptr ) return E_FAIL;

            auto pmcArray = ( LPCOMPLEXARRAY ) item;

            int rows = pmcArray->rows;
            int cols = pmcArray->cols;

            if ( rows < 1 || cols < 1 )
            {
                Manager::LogError( "'{0}': wrong dimension for {1} argument: {2}x{3}", func->Info->Name, n, rows, cols );

                return E_FAIL;
            }

            bool bReal = pmcArray->hReal != nullptr;
            bool bImag = pmcArray->hImag != nullptr;

            auto matrix = gcnew array<Complex, 2>( rows, cols );

            for ( int row = 0; row < rows; row++ )
            {
                for ( int col = 0; col < cols; col++ )
                {
                    double re = bReal ? pmcArray->hReal[ col ][ row ] : 0;
                    double im = bImag ? pmcArray->hImag[ col ][ row ] : 0;

                    matrix[ row, col ] = Complex( re, im );
                }
            }

            args[n] = matrix;
        }
        else
        {
            Manager::LogError( "'{0}': unknown argument type {1}: {2}", func->Info->Name, n, type->ToString() );

            return E_FAIL;
        }
    }

    return S_OK;
}


LRESULT Evaluate( IFunction ^ func, array < Object ^ > ^ args, array < String ^ > ^ errors, Object ^ % lvalue )
{
    int count = func->Info->ArgTypes->GetLength(0);

    // Call the user function.
    try
    {
        auto context = gcnew Context();

        if ( !func->NumericEvaluation( args, lvalue, context ) ) return E_FAIL;
    }
    catch ( EFIException ^ ex )
    {
        if ( errors == nullptr )
        {
            Manager::LogError( "'{0}' {1}", func->Info->Name, "Errors table is empty" );

            return E_FAIL;
        }

        if ( ( ex->ErrNum > 0 ) && ( ex->ErrNum <= errors->GetLength(0) )
            && ( ex->ArgNum > 0 ) && ( ex->ArgNum <= count ) )
        {
            // Calculate the error table location.
            int offset = 0;

            for ( int n = 0; n < MathcadEfi.AssemblyId; n++ )
            {                
                auto assembly = Manager::Assemblies[n];

                offset += assembly->Errors == nullptr ? 0 : assembly->Errors->GetLength(0);
            }

            // See Mathcad Developer Reference for the details.
            return MAKELRESULT( offset + ex->ErrNum, ex->ArgNum );
        }
        else
        {
            Manager::LogError( "'{0}': {1}", func->Info->Name, ex->Message );

            return E_FAIL;
        }
    }
    catch ( System::Exception ^ ex )
    {
        Manager::LogError( "'{0}': {1}", func->Info->Name, ex->Message );

        return E_FAIL;
    }

    return S_OK;
}


LRESULT ConvertOutput( IFunction ^ func, Object ^ lvalue, void * result )
{
    // Convert the result.
    Type ^ type = lvalue->GetType();

    // String to MCSTRING.
    if ( type->Equals( String::typeid ) )
    {
        auto pmcString = ( LPMCSTRING ) result;

        // .net unicode to ansi std::string.
        std::string text = marshal_as< std::string >( ( String ^ ) lvalue );

        pmcString->str = ( char * ) MathcadEfi.MathcadAllocate( ( unsigned int ) text.length() + 1u );

        // std::string to char[].
        ::memcpy( pmcString->str, text.c_str(), text.length() );
        
        // Null terminate.
        pmcString->str[ text.length() ] = '\0';

        /*
        auto bytes = Encoding::UTF8->GetBytes( ( String ^ ) lvalue );

        pmcString->str = ( char * ) MathcadEfi.MathcadAllocate( bytes->Length + 1 );

        Marshal::Copy( bytes, 0, IntPtr( pmcString->str ), bytes->Length );
        */
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

        if ( matrix == nullptr )
        {
            Manager::LogError( "'{0}': return value is NULL", func->Info->Name );

            return E_FAIL;
        }

        int rows = matrix->GetLength(0);
        int cols = matrix->GetLength(1);

        if ( rows < 1 || cols < 1 )
        {
            Manager::LogError( "'{0}': wrong return value: [{1}x{2}]", func->Info->Name, rows, cols );

            return E_FAIL;
        }

        bool bImag = false;

        // Check if imaginary part is empty.
        for ( int row = 0; row < rows; row++ )
        {
            for ( int col = 0; col < cols; col++ )
            {
                if ( matrix[ row, col ].Imaginary != 0.0 )
                {
                    bImag = true;
                    break;
                }
            }

            if ( bImag ) break;
        }

        // The first parameter for the MathcadArrayAllocate() function
        // must be pointer to the COMPLEXARRAY structure.
        auto pmcArray = ( LPCOMPLEXARRAY ) result;

        MathcadEfi.MathcadArrayAllocate( pmcArray, rows, cols, TRUE, bImag ? TRUE : FALSE );

        for ( int row = 0; row < rows; row++ )
        {
            for ( int col = 0; col < cols; col++ )
            {
                pmcArray->hReal[ col ][ row ] = matrix[ row, col ].Real;

                if ( bImag )
                {
                    pmcArray->hImag[ col ][ row ] = matrix[ row, col ].Imaginary;
                }
            }
        }
    }
    else
    {
        Manager::LogError( "'{0}': unknown return type: {1}", func->Info->Name, type->ToString() );

        return E_FAIL;
    }

    return S_OK;
}


// General function.
LRESULT UserFunction( PVOID items[] )
{   
    try
    {
        auto assemblyInfo = Manager::Assemblies[ MathcadEfi.AssemblyId ];

        auto func = assemblyInfo->Functions[ MathcadEfi.FunctionId ];

        // Get the count of parameters.
        int count = func->Info->ArgTypes->GetLength(0);

        // Create an array of managed function parameters.
        auto args = gcnew array < Object ^ >( count );

        auto result = ConvertInputs( func, items, args );

        if ( result != S_OK ) return result;

        Object ^ lvalue;

        result = Evaluate( func, args, assemblyInfo->Errors, lvalue );

        if ( result != S_OK ) return result;

        result = ConvertOutput( func, lvalue, items[0] );

        if ( result != S_OK ) return result;
    }
    catch ( ... )
    {
        Manager::LogError( "AssemblyId: {0}, FunctionId: {1}", MathcadEfi.AssemblyId, MathcadEfi.FunctionId );

        return E_FAIL;
    }

    return S_OK;
}


void RegisterFunctions()
{
    // Prepare managed code. Setup our resolver for assembly loading.
    AppDomain::CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler( Manager::OnAssemblyResolve );

    if ( Manager::LoadAssemblies() ) Manager::RegisterFunctions();
}

#pragma unmanaged

LRESULT CallbackFunction( void * out, ... )
{
    return ::UserFunction( & out );
}


BOOL WINAPI DllEntryPoint( HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved ) 
{   
    if ( dwReason == DLL_PROCESS_ATTACH )
    { 
        try
        {
            RegisterFunctions();
        }
        catch ( ... ) {}
    }

    else if ( dwReason == DLL_PROCESS_DETACH )
    {
    }

    return TRUE;
}

#pragma managed
