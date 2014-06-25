#include "stdafx.h"
#include <msclr\marshal_cppstd.h>
#include "mcadincl.h"
#include "netefi.h"


using namespace NetEFI;


static int id;
static PBYTE pCode;

PCREATE_USER_FUNCTION CreateUserFunction; 
PCREATE_USER_ERROR_MESSAGE_TABLE CreateUserErrorMessageTable; 
PMATHCAD_ALLOCATE MathcadAllocate; 
PMATHCAD_FREE MathcadFree; 
PMATHCAD_ARRAY_ALLOCATE MathcadArrayAllocate; 
PMATHCAD_ARRAY_FREE MathcadArrayFree; 
PIS_USER_INTERRUPTED isUserInterrupted;


// Эмуляция is из C#.
template < class T, class U > 
Boolean isinst(U u) {
   
    return dynamic_cast< T >(u) != nullptr;
}


LRESULT GlobalFunction( void * out, ... ) {

    LogInfo( "GlobalFunction()" );

    LPCOMPLEXSCALAR v = ( LPCOMPLEXSCALAR ) out;

    v->real = 0;
    //out->imag = in->imag;
    v->imag = id;

    return 0;
}


void LogInfo( string text ) {

    Manager::LogInfo( marshal_as<String^, string>( text ) );
}


void LogError( string text ) {

    Manager::LogError( marshal_as<String^, string>( text ) );
}


void Manager::LogInfo( String^ Text ) {

    File::AppendAllText( LogFile, ( gcnew String( "" ) )->Format( "{0} {1} [INFO ] {2}{3}", 
        DateTime::Now.ToShortDateString(), DateTime::Now.ToLongTimeString(), 
        Text, Environment::NewLine ), Encoding::UTF8 );
}


void Manager::LogError( String^ Text ) {

    File::AppendAllText( LogFile, ( gcnew String( "" ) )->Format( "{0} {1} [ERROR] {2}{3}", 
        DateTime::Now.ToShortDateString(), DateTime::Now.ToLongTimeString(), 
        Text, Environment::NewLine ), Encoding::UTF8 );
}


bool Manager::Initialize() {

    try {
        ::System::IO::File::Delete( Manager::LogFile );
    } catch (...) {}

    pCode = ( PBYTE ) ::VirtualAllocEx( ::GetCurrentProcess(), 0, 1 << 16, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );    

    if ( pCode == NULL ) {
        
        LogInfo( "VirtualAllocEx() failed." );
        return false;
    }

    String^ path = Path::GetDirectoryName( Manager::AssemblyPath );
        
    path = Path::Combine( path, gcnew String( L"..\\mcaduser.dll" ) );

    if ( !File::Exists( path ) ) {
    
        LogError( "mcaduser.dll not found." );
        return false;
    }

    HMODULE hLib = ::LoadLibraryW( marshal_as<wstring, String^>( path ).c_str() ); 

    if ( hLib == NULL ) {
        
        LogError( "LoadLibrary() returns NULL." );
        return false;
    }

    CreateUserFunction          = ( PCREATE_USER_FUNCTION )             ::GetProcAddress( hLib, "CreateUserFunction" );
    CreateUserErrorMessageTable = ( PCREATE_USER_ERROR_MESSAGE_TABLE )  ::GetProcAddress( hLib, "CreateUserErrorMessageTable" ); 
    MathcadAllocate             = ( PMATHCAD_ALLOCATE )                 ::GetProcAddress( hLib, "MathcadAllocate" ); 
    MathcadFree                 = ( PMATHCAD_FREE )                     ::GetProcAddress( hLib, "MathcadFree" ); 
    MathcadArrayAllocate        = ( PMATHCAD_ARRAY_ALLOCATE )           ::GetProcAddress( hLib, "MathcadArrayAllocate" ); 
    MathcadArrayFree            = ( PMATHCAD_ARRAY_FREE )               ::GetProcAddress( hLib, "MathcadArrayFree" ); 
    isUserInterrupted           = ( PIS_USER_INTERRUPTED )              ::GetProcAddress( hLib, "isUserInterrupted" ); 

    if ( CreateUserFunction == NULL 
        || CreateUserErrorMessageTable == NULL 
        || MathcadAllocate == NULL 
        || MathcadFree == NULL
        || MathcadArrayAllocate == NULL 
        || MathcadArrayFree == NULL 
        || isUserInterrupted == NULL ) { 
        
        LogError( "GetProcAddress() returns NULL." );

        return false;
    }
    
    return true; 
}

// How to determine whether a DLL is a managed assembly or native (prevent loading a native dll)?
// http://stackoverflow.com/questions/367761/how-to-determine-whether-a-dll-is-a-managed-assembly-or-native-prevent-loading
bool Manager::IsManagedAssembly( String^ fileName ) {

    Stream^ fileStream = gcnew FileStream( fileName, IO::FileMode::Open, IO::FileAccess::Read );

    BinaryReader^ binaryReader = gcnew BinaryReader( fileStream );

    if ( fileStream->Length < 64 ) {

        return false;
    }

    //PE Header starts @ 0x3C (60). Its a 4 byte header.
    fileStream->Position = 0x3C;

    UINT peHeaderPointer = binaryReader->ReadUInt32();

    if ( peHeaderPointer == 0 ) {

        peHeaderPointer = 0x80;
    }

    // Ensure there is at least enough room for the following structures:
    //     24 byte PE Signature & Header
    //     28 byte Standard Fields         (24 bytes for PE32+)
    //     68 byte NT Fields               (88 bytes for PE32+)
    // >= 128 byte Data Dictionary Table
    if ( peHeaderPointer > fileStream->Length - 256 ) {

        return false;
    }

    // Check the PE signature.  Should equal 'PE\0\0'.
    fileStream->Position = peHeaderPointer;

    UINT peHeaderSignature = binaryReader->ReadUInt32();

    if ( peHeaderSignature != 0x00004550 ) {

        return false;
    }

    // skip over the PEHeader fields
    fileStream->Position += 20;

    const USHORT PE32 = 0x10b;
    const USHORT PE32Plus = 0x20b;

    // Read PE magic number from Standard Fields to determine format.
    USHORT peFormat = binaryReader->ReadUInt16();

    if ( peFormat != PE32 && peFormat != PE32Plus ) {

        return false;
    }

    // Read the 15th Data Dictionary RVA field which contains the CLI header RVA.
    // When this is non-zero then the file contains CLI data otherwise not.
    USHORT dataDictionaryStart = ( USHORT ) ( peHeaderPointer + ( peFormat == PE32 ? 232 : 248 ) );

    fileStream->Position = dataDictionaryStart;

    UINT cliHeaderRva = binaryReader->ReadUInt32();

    if ( cliHeaderRva == 0 ) {

        return false;
    }

    return true;
}


// Загрузка пользовательских библиотек.
bool Manager::LoadAssemblies( HINSTANCE hInstance ) {

    try {

        // Списки функций и их карточек.
        Manager::Items = gcnew List< IFunction^ >();
        Manager::Infos = gcnew List < FunctionInfo^ >();

        // Получаем список всех библиотек в текущей папке.
        array< String^ > ^ libs = Directory::GetFiles( Path::GetDirectoryName( AssemblyPath ), gcnew String( "*.dll" ) );

        // Сканируем каждый файл на наличие классов, реализующих интерфейс IFunction.
    for each ( String^ path in libs ) {

        try {
            
            if ( !IsManagedAssembly( path ) || path->Equals( AssemblyPath ) ) continue;

            LogInfo( path );

            array<Type^>^ types;

            try {
                                     
                // LoadFile vs. LoadFrom
                // http://blogs.msdn.com/b/suzcook/archive/2003/09/19/loadfile-vs-loadfrom.aspx
                Assembly^ assembly = Assembly::GetExecutingAssembly()->LoadFile( path );

                LogInfo( assembly->ToString() );

                types = assembly->GetTypes();

                LogInfo( types->ToString() );

            } catch ( System::Exception^ ex ) {

                LogError( ex->Message );
                continue;
                
            } catch ( ... ) {
                    
                ::LogError( "Error 1" );
                continue;    
            }

            for each ( Type^ type in types ) {

                LogInfo( type->ToString() );

                if ( !type->IsClass || !IFunction::typeid->IsAssignableFrom( type ) ) continue;

                Manager::Items->Add( ( IFunction^ ) Activator::CreateInstance( type ) );
            }

            LogInfo( String::Format( " [LoadLibraries] {0} loaded.", path ) );

        } catch ( System::Exception^ ex ) {

            LogError( ex->Message );
            continue;

        } catch ( ... ) {
                    
            ::LogError( "Error 2" );
            continue;    
        }

    }

        // Теперь регистрируем все функции в Mathcad.
        PBYTE p = pCode;

        for ( int n = 0; n < Manager::Items->Count; n++ ) {
            
            FunctionInfo^ info = Manager::Items[n]->GetFunctionInfo( "RUS" );
            
            Manager::Infos->Add( info );                  

            LogInfo( gcnew String( "Manager::Infos->Add( info )" ) );

            FUNCTIONINFO fi;

            marshal_context context;
                
            fi.lpstrName = ( char * ) context.marshal_as<string>( info->Name ).c_str();
            fi.lpstrParameters = ( char * ) context.marshal_as<string>( info->Parameters ).c_str();
            fi.lpstrDescription = ( char * ) context.marshal_as<string>( info->Description ).c_str();
            fi.lpfnMyCFunction = ( LPCFUNCTION ) p;

            Type^ type = info->ReturnType->GetType();

            // How to check an object's type in C++/CLI?
            // http://stackoverflow.com/questions/2410721/how-to-check-an-objects-type-in-c-cli
            if ( type->Equals( String::typeid ) ) {

                fi.returnType = STRING;

            } else if ( type->Equals( Complex::typeid ) ) {

                fi.returnType = COMPLEX_SCALAR;

            } else if ( type->Equals( array<Complex,2>::typeid ) ) {

                fi.returnType = COMPLEX_ARRAY;            
            
            } else continue;
            
            fi.nArgs = info->Arguments->GetLength(0);
            
            for ( unsigned int m = 0; m < fi.nArgs; m++ ) {

                type = info->Arguments[m]->GetType();

                if ( type->Equals( String::typeid ) ) {

                    fi.argType[m] = STRING;

                } else if ( type->Equals( Complex::typeid ) ) {

                    fi.argType[m] = COMPLEX_SCALAR;

                } else if ( type->Equals( array<Complex,2>::typeid ) ) {

                    fi.argType[m] = COMPLEX_ARRAY;            
            
                // TODO: continue для внешнего цикла.
                } else break;

            }            

            ::CreateUserFunction( hInstance, & fi );  

            // Пересылки константы (номера функции) в глобальную переменную id.
            * p++ = 0xB8; // mov eax, imm32
            p[0] = n;
            p += sizeof( int );

            * p++ = 0xA3; // mov [id], eax
            ( int * & ) p[0] = & id; 
            p += sizeof( int * );         

            // jmp to GlobalFunction(). 
            * p++ = 0xE9;
            ( UINT & ) p[0] = ( PBYTE ) GlobalFunction - 4 - p;
            p += sizeof( PBYTE );
        }

    } catch ( System::Exception^ ex ) {

        LogError( ex->Message );

        return false;

    } catch ( ... ) {
                    
        ::LogError( "Error 3" );
        return false;    
    }

    return true;
}


bool Manager::NumericEvaluation( int n, array < Object^ > ^ args, Object ^ % result ) {

    return true;
}