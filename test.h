#pragma once

#include "netefi.h"

using namespace System;
using namespace System::IO;
using namespace System::Text;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Globalization;
using namespace System::Collections::Generic;

using namespace std;


namespace NetEFI {

    public ref class test: public IFunction {

    private:
        FunctionInfo ^ info;

    protected:

        ~test() {}

    public:

        virtual property FunctionInfo^ Info {

            FunctionInfo^ get() { return info; }
        }

        !test() {}

        test() {

			info = gcnew FunctionInfo();

            info->Name = L"test";
            info->Parameters = L"x";
            info->Description = L"test(x)";
            info->ReturnType = TComplex::typeid;
            info->Arguments = gcnew array<Type^> { TComplex::typeid };
        }

        virtual FunctionInfo^ GetFunctionInfo(String^ lang) {

            return info;
        }

        virtual bool NumericEvaluation(array< Object^ > ^, Object ^ %) {

            return true;
        }

    };

}

