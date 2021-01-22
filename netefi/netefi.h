#pragma once

using namespace System;
using namespace System::Numerics;
using namespace System::IO;
using namespace System::Text;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Globalization;
using namespace System::Collections::Generic;

namespace NetEFI
{
    public ref class EFIException : Exception
    {
    public:

        int ArgNum;
        int ErrNum;

        EFIException( int errNum, int argNum )
        {
            ErrNum = errNum;
            ArgNum = argNum;
        }
    };

    public ref class FunctionInfo
    {
    public:

        String ^ Name;
        String ^ Parameters;
        String ^ Description;
        Type ^ ReturnType;
        array < Type ^ > ^ ArgTypes;

        FunctionInfo( String ^ name, String ^ params, String ^ descr, Type ^ returnType, array < Type ^ > ^ argTypes )
        {
            Name = name;
            Parameters = params;
            Description = descr;
            ReturnType = returnType;
            ArgTypes = argTypes;
        }
    };

    ref class Context;

    public interface class IFunction
    {
    public:

        property FunctionInfo ^ Info { FunctionInfo ^ get(); }

        FunctionInfo ^ GetFunctionInfo( String ^ lang );
        bool NumericEvaluation( array < Object ^ > ^, [ Out ] Object ^%, Context ^% );
    };

    public ref class AssemblyInfo
    {
    public:
        String ^ Path;
        List < IFunction ^ > ^ Functions;
        array < String ^ > ^ Errors;
    };

    public ref class netefi : public IFunction
    {
    public:

        virtual property FunctionInfo ^ Info
        {
            FunctionInfo ^ get()
            {
                return gcnew FunctionInfo( "netefi", "cmd", "return string",
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

            result = gcnew String( "help: info, author, email" );

            if ( cmd->Equals( gcnew String( "info" ) ) )
            {
                bool is64Bit = Marshal::SizeOf( IntPtr::typeid ) == 8;

                auto name = Assembly::GetExecutingAssembly()->GetName();

                auto version = name->Version;

                DateTime ^ bdate = ( gcnew DateTime( 2000, 1, 1 ) )->AddDays( version->Build );

                bdate = bdate->AddSeconds( 2 * version->Revision );

                result = String::Format( "{0}: {1}-bit, {2}, {3:dd-MMM-yyyy HH:mm:ss}", name->Name, ( is64Bit ? "64" : "32" ), version, bdate );
            }

            else if ( cmd->Equals( gcnew String( "author" ) ) )
            {
                result = gcnew String( "Viacheslav N. Mezentsev" );
            }

            else if ( cmd->Equals( gcnew String( "email" ) ) )
            {
                result = gcnew String( "viacheslavmezentsev@ya.ru" );
            }

            return true;
        }
    };
}
