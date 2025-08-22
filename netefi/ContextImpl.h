#pragma once

#include "netefi.h"

namespace NetEFI
{
    public ref class ContextImpl : public Context
    {
    public:

        virtual property bool IsUserInterrupted { bool get() override; }
        virtual property IComputable^ default[ String ^ ]{ IComputable^ get( String ^ ) override; }

    public:

        virtual bool IsDefined( String ^ ) override;

        virtual void LogInfo( String ^ ) override;
        virtual void LogInfo( String ^, ... array<Object ^> ^ ) override;
        virtual void LogError( String ^ ) override;
        virtual void LogError( String ^, ... array<Object ^> ^ ) override;
    };
}
