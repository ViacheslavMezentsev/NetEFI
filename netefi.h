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

	public ref class FunctionInfo {

	public:

		String^ Name;
		String^ Parameters; 
		String^ Description;
        String^ AssemblyPath;
		Type^ ReturnType;
		array < Type^ > ^ ArgTypes;

        FunctionInfo( String^ name, String^ params, String^ descr, String^ path, Type^ returnType, array < Type^ > ^ argTypes ){

            Name = name;
            Parameters = params;
            Description = descr;
            AssemblyPath = path;
            ReturnType = returnType;
            ArgTypes = argTypes;
        }

	};


	public interface class IFunction {

	public:

		property FunctionInfo^ Info { FunctionInfo^ get(); }

		FunctionInfo^ GetFunctionInfo( String^ lang );
		bool NumericEvaluation( array < Object^ > ^, [Out] Object ^ % );
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


	public ref class Manager {

	public:

		Manager() {} // Конструктор
		~Manager() {} // Деструктор
		!Manager() {} // Финализатор

		// Элементы.
		static List < IFunction^ > ^ Items;
        static List < FunctionInfo^ > ^ Infos;		

		static property String^ AssemblyPath {

			String^ get() {

				return ( gcnew System::Uri( Assembly::GetExecutingAssembly()->CodeBase ) )->LocalPath;
			}; 

		};

		static property String^ LogFile {

			String^ get() {

				return Path::Combine( Path::GetDirectoryName( AssemblyPath ), gcnew String( L"log.txt" ) );
			}; 

		};

	public:
        
        static bool Initialize();

		// Ведение журнала сообщений.
		static void LogInfo( String^ );

		static void LogError( String^ );

        static bool IsManagedAssembly( String^ );

		// Загрузка пользовательских библиотек.
		static bool LoadAssemblies( HINSTANCE );		
	};

}
