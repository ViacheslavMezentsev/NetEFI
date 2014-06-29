#pragma once

using namespace System;
using namespace System::IO;
using namespace System::Text;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Globalization;
using namespace System::Collections::Generic;

using namespace std;
using namespace msclr::interop;


void LogInfo( string text );
void LogError( string text );
LRESULT GlobalFunction( void * out, ... );


namespace NetEFI {

    public ref class EFIException: Exception {
      
    public:

        int ArgNum;
        int ErrNum;

        EFIException( int errNum, int argNum ) {

            ErrNum = errNum;
            ArgNum = argNum;
        }
    };

	public ref class FunctionInfo {

	public:

		String^ Name;
		String^ Parameters; 
		String^ Description;
		Type^ ReturnType;
		array < Type^ > ^ ArgTypes;

        FunctionInfo( String^ name, String^ params, String^ descr, Type^ returnType, array < Type^ > ^ argTypes ){

            Name = name;
            Parameters = params;
            Description = descr;
            ReturnType = returnType;
            ArgTypes = argTypes;
        }

	};

    
    public ref class Context {
    
    public:

        property bool IsUserInterrupted { bool get(); }
    };


	public interface class IFunction {

	public:

        property FunctionInfo^ Info { FunctionInfo^ get(); }        

		FunctionInfo^ GetFunctionInfo( String^ lang );
		bool NumericEvaluation( array < Object^ > ^, [Out] Object ^ %, Context ^ % );
	};


    private ref class AssemblyInfo {

    public:
        String^ Path;
        List < IFunction^ > ^ Functions;
        array < String ^ > ^ Errors;
    };


	public ref class TComplex {

	private:
		double _imaginary;
		double _real;

	public:
		property double Imaginary { double get() { return _imaginary; } }
		property double Real { double get() { return _real; } }

	public:
		TComplex( double real, double imaginary ) {

			_real = real;
			_imaginary = imaginary;
		}

	};


	private ref class Manager {

	public:

		Manager() {} // Конструктор
		~Manager() {} // Деструктор
		!Manager() {} // Финализатор

        static List < AssemblyInfo^ > ^ Assemblies;

        static property String^ AssemblyPath { String^ get(); };
		static property String^ LogFile { String^ get(); };

	public:
        
		// Вывод информационных сообщений.
		static void LogInfo( String^ );

        static void LogInfo( string );

        // Вывод сообщений об ошибках.
		static void LogError( String^ );

        static void LogError( string );

        // Проверка типа библиотеки.
        static bool IsManagedAssembly( String^ );

        // Настройка менеджера.
        static bool Initialize();

		// Загрузка пользовательских сборок.
		static bool LoadAssemblies();	

        static PVOID CreateUserFunction( FunctionInfo^, PVOID );

        static void CreateUserErrorMessageTable( array < String ^ > ^ );

        static void InjectCode( PBYTE &, int, int );
	};

}
