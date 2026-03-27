#pragma once

#include "netefi.h"

namespace NetEFI
{
    public ref class ContextImpl : public Context
    {
    public:

        virtual property bool IsUserInterrupted { bool get() override; }

        virtual property CustomFunctionBase^ default[String^]
        {
            CustomFunctionBase^ get( String ^ functionName ) override
            {
                if ( String::IsNullOrEmpty( functionName ) ) return nullptr;

                for each( AssemblyInfo ^ assemblyInfo in Manager::Assemblies )
                {
                    for each( CustomFunctionBase ^ func in assemblyInfo->Functions )
                    {
                        if ( func->Info->Name->Equals( functionName, StringComparison::OrdinalIgnoreCase ) )
                        {
                            return func;
                        }
                    }
                }

                return nullptr;
            }
        }

    public:

        virtual bool IsDefined( String ^ ) override;

        generic<typename TResult>
        virtual TResult Invoke( String^ functionName, ... array<Object^>^ args ) override;

        virtual void LogInfo( String ^ ) override;
        virtual void LogInfo( String ^, ... array<Object ^> ^ ) override;
        virtual void LogError( String ^ ) override;
        virtual void LogError( String ^, ... array<Object ^> ^ ) override;
    };
}
