﻿// Основы миграции C++/CLI.
// http://msdn.microsoft.com/ru-ru/library/ms235289.aspx
// C++/CLI Tasks.
// http://msdn.microsoft.com/ru-ru/library/hh875047.aspx
// Common Language Runtime Loader and DllMain
// http://msdn.microsoft.com/en-us/library/aa290048(v=vs.71).aspx
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

extern int AssemblyId;
extern int FunctionId;
extern CMathcadEfi MathcadEfi;


void RegisterFunctions()
{
    // Prepare managed code. Setup our resolver for assembly loading.
    AppDomain::CurrentDomain->AssemblyResolve += gcnew ResolveEventHandler( Manager::OnAssemblyResolve );

    if ( Manager::LoadAssemblies() ) Manager::RegisterFunctions();
}


// General function.
LRESULT UserFunction( PVOID items[] )
{   
    AssemblyInfo ^ assemblyInfo = nullptr;
    IFunction ^ func = nullptr;

    try
    {
        assemblyInfo = Manager::Assemblies[ AssemblyId ];

        if ( assemblyInfo == nullptr ) throw gcnew Exception();

        func = assemblyInfo->Functions[ FunctionId ];

        if ( func == nullptr ) throw gcnew Exception();
    }
    catch ( ... )
    {
        Manager::LogError( "AssemblyId: {0}, FunctionId: {1}", AssemblyId, FunctionId );

        return E_FAIL;
    }

    // Get the count of parameters.
    int count = func->Info->ArgTypes->GetLength(0);

    // Create an array of managed function parameters.
    array < Object ^ > ^ args = gcnew array < Object ^ >( count );

    // Convert each parameter to managed type.
    for ( int n = 0; n < count; n++ )
    {
        Type ^ type = func->Info->ArgTypes[n];

        void * item = items[ n + 1 ];

        // MCSTRING
        // It seems that MCSTRING contains only one byte from unicode wchar.
        if ( type->Equals( String::typeid ) )
        {
            MCSTRING * pmcString = ( MCSTRING * ) item;
            
            // ANSI char[] to std::string.
            std::string text( pmcString->str );

            // std::string to .net unicode.
            args[n] = marshal_as<String ^>( text );
            
            /*
            size_t len = strlen( pmcString->str );

            array<Byte> ^ bytes = gcnew array<Byte>( len );

            Marshal::Copy( IntPtr( pmcString->str ), bytes, 0, bytes->Length );

            args[n] = UTF8Encoding::UTF8->GetString( bytes );           
            */
        }

        // COMPLEXSCALAR
        else if ( type->Equals( Complex::typeid ) )
        {
            COMPLEXSCALAR * pmcScalar = ( COMPLEXSCALAR * ) item;

            args[n] = Complex( pmcScalar->real, pmcScalar->imag );
        }

        // COMPLEXARRAY
        else if ( type->Equals( array<Complex, 2>::typeid ) )
        {
            COMPLEXARRAY * pmcArray = ( COMPLEXARRAY * ) item;

            int rows = pmcArray->rows;
            int cols = pmcArray->cols;

            bool bReal = pmcArray->hReal != nullptr;
            bool bImag = pmcArray->hImag != nullptr;

            array<Complex, 2> ^ matrix = gcnew array<Complex, 2>( rows, cols );

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
            Manager::LogError( "'{0}' Unknown argument type {1}: {2}", func->Info->Name, n, type->ToString() );

            return E_FAIL;
        }
    }
    
    Object ^ result;    

    // Call the user function.
    try
    {
        auto context = gcnew Context();

        if ( !func->NumericEvaluation( args, result, context ) ) return E_FAIL;
    }
    catch ( EFIException ^ ex )
    {
        if ( assemblyInfo->Errors == nullptr )
        {
            Manager::LogError( "'{0}' {1}", func->Info->Name, "Errors table is empty" );

            return E_FAIL;
        }

        if ( ( ex->ErrNum > 0 ) && ( ex->ErrNum <= assemblyInfo->Errors->GetLength(0) )
            && ( ex->ArgNum > 0 ) && ( ex->ArgNum <= count ) )
        {
            // Calculate the error table location.
            int offset = 0;

            for ( int n = 0; n < AssemblyId; n++ )
            {                
                auto assembly = Manager::Assemblies[n];

                offset += assembly->Errors == nullptr ? 0 : assembly->Errors->GetLength(0);
            }

            // See Mathcad Developer Reference for the details.
            return MAKELRESULT( offset + ex->ErrNum, ex->ArgNum );
        }
        else
        {
            Manager::LogError( "'{0}' {1}", func->Info->Name, ex->Message );

            return E_FAIL;
        }
    }
    catch ( System::Exception ^ ex )
    {
        Manager::LogError( "'{0}' {1}", func->Info->Name, ex->Message );

        return E_FAIL;
    }

    // Convert the result.
    Type ^ type = result->GetType();
    void * item = items[0];

    // MCSTRING
    if ( type->Equals( String::typeid ) )
    {
        MCSTRING * pmcString = ( MCSTRING * ) item;

        // .net unicode to ansi std::string.
        std::string text = marshal_as< std::string >( ( String ^ ) result );

        pmcString->str = ( char * ) MathcadEfi.MathcadAllocate( text.size() + 1 );

        // std::string to char[].
        ::memcpy( pmcString->str, text.c_str(), text.size() );
        
        // Null terminate.
        pmcString->str[ text.size() ] = '\0';

        /*
        array<Byte> ^ bytes = Encoding::UTF8->GetBytes( ( String ^ ) result );

        pmcString->str = ( char * ) ::MathcadAllocate( bytes->Length + 1 );

        Marshal::Copy( bytes, 0, IntPtr( pmcString->str ), bytes->Length );
        */
    }

    // COMPLEXSCALAR
    else if ( type->Equals( Complex::typeid ) )
    {
        COMPLEXSCALAR * pmcScalar = ( COMPLEXSCALAR * ) item;

        Complex cmplx = ( Complex ) result;

        pmcScalar->real = cmplx.Real;
        pmcScalar->imag = cmplx.Imaginary;
    }

    // COMPLEXARRAY
    else if ( type->Equals( array<Complex, 2>::typeid ) )
    {
        array<Complex, 2> ^ matrix = ( array<Complex, 2> ^ ) result;

        int rows = matrix->GetLength(0);
        int cols = matrix->GetLength(1);

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
        COMPLEXARRAY * pmcArray = ( COMPLEXARRAY * ) item;

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
        Manager::LogError( "'{0}' Unknown return type: {1}", func->Info->Name, type->ToString() );

        return E_FAIL;
    }

    return S_OK;
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
        catch (...) {}
    }

    else if ( dwReason == DLL_PROCESS_DETACH )
    {
    }

    return TRUE;
}

#pragma managed
