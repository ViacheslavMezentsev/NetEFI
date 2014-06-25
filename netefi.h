#pragma once

using namespace System;
using namespace System::IO;
using namespace System::Text;
using namespace System::Numerics;
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
		Object^ ReturnType;
		array < Type^ > ^ Arguments;
	};

	public interface class IFunction {

	public:

		property FunctionInfo^ Info { FunctionInfo^ get(); }

		FunctionInfo^ GetFunctionInfo( String^ lang );
		bool NumericEvaluation( array < Object^ > ^, Object ^ % );
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

        static property Assembly^ NetEFIAssembly {

            Assembly^ get() {
                
                return Assembly::GetExecutingAssembly();
            }
        }

	public:
        
        static bool Initialize();

		// Ведение журнала сообщений.
		static void LogInfo( String^ );

		static void LogError( String^ );

        static bool IsManagedAssembly( String^ );

		// Загрузка пользовательских библиотек.
		static bool LoadAssemblies( HINSTANCE );

		// Обобщённая функция.
		static bool NumericEvaluation( int, array < Object^ > ^, Object ^ % );
	};
}
