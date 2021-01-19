#pragma once

#include "netefi.h"

namespace NetEFI
{
    public ref class Context
    {
    public:

        property bool IsUserInterrupted { bool get(); }
        property IFunction ^ default[ String ^ ]{ IFunction ^ get( String ^ ); }

    public:

        bool IsDefined( String ^ );

        void LogInfo( String ^ );
        void LogInfo( String ^, ... array<Object ^> ^ );
        void LogError( String ^ );
        void LogError( String ^, ... array<Object ^> ^ );
    };
}
