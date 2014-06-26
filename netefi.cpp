// Component Extensions for Runtime Platforms
// http://msdn.microsoft.com/en-us/library/xey702bw.aspx
// Основы миграции C++/CLI
// http://msdn.microsoft.com/ru-ru/library/ms235289.aspx

#include "stdafx.h"
#include <msclr\marshal_cppstd.h>
#include "mcadincl.h"
#include "test.h"
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


int GetParamsCount( int index ) {

    return Manager::Infos[ index ]->Arguments->GetLength(0);
}


// Обобщённая функция.
bool UserFunction( int index, PVOID items[] ) {

    MCSTRING * pmcString;
    COMPLEXSCALAR * pmcScalar;
    COMPLEXARRAY * pmcArray;
    Type^ type;

    FunctionInfo^ info = Manager::Infos[index];

    int Count = info->Arguments->GetLength(0);

    array < Object^ > ^ args = gcnew array < Object^ >( Count );

    for ( int n = 0; n < Count; n++ ) {

        type = info->Arguments[n];

        // MCSTRING
        if ( type->Equals( String::typeid ) ) {

            pmcString = ( MCSTRING * ) items[ n + 1 ];

            args[n] = marshal_as<String^>( pmcString->str );

        // COMPLEXSCALAR
        } else if ( type->Equals( TComplex::typeid ) ) {

            pmcScalar = ( COMPLEXSCALAR * ) items[ n + 1 ];

            args[n] = gcnew TComplex( pmcScalar->real, pmcScalar->imag );

        // COMPLEXARRAY
        } else if ( type->Equals( array<TComplex^,2>::typeid ) ) {

            pmcArray = ( COMPLEXARRAY * ) items[ n + 1 ];
            
            int rows = pmcArray->rows;
            int cols = pmcArray->cols;

            array<TComplex^,2>^ Matrix = gcnew array<TComplex^,2>( rows, cols );
            
            TComplex^ tmp;

            for ( int row = 0; row < rows; row++ ) {

                for ( int col = 0; col < cols; col++ ) {

                    if ( ( pmcArray->hReal != NULL ) && ( pmcArray->hImag != NULL ) ) 
                        tmp = gcnew TComplex( pmcArray->hReal[col][row], pmcArray->hImag[col][row] );
                    
                    if ( ( pmcArray->hReal != NULL ) && ( pmcArray->hImag == NULL ) ) 
                        tmp = gcnew TComplex( pmcArray->hReal[col][row], 0.0 );
                        
                    if ( ( pmcArray->hReal == NULL ) && ( pmcArray->hImag != NULL ) ) 
                        tmp = gcnew TComplex( 0.0, pmcArray->hImag[col][row] );

                    Matrix[ row, col ] = tmp;
                }

            }
            
            args[n] = Matrix;
        }
        
    }

    // Вызываем функцию.
    Object^ result;

    Manager::Items[ index ]->NumericEvaluation( args, result );

    // Преобразуем результат.
    type = result->GetType();

    if ( type->Equals( String::typeid ) ) {

        // Выделяем память под структуру.
        pmcString = ( MCSTRING * ) items[0];

        marshal_context context;

        char * text = ( char * ) context.marshal_as<const char *>( ( String^ ) result );

        // Выделяем память для строки и завершающего нуля.
        pmcString->str = ( char * ) ::malloc( ::strlen( text ) + 1 );

        ::memset( pmcString->str, 0, ::strlen( text ) + 1 );
                    
        // Копируем строку из временной области памяти.
        ::memcpy( pmcString->str, text, ::strlen( text ) );


    } else if ( type->Equals( TComplex::typeid ) ) {

        pmcScalar = ( COMPLEXSCALAR * ) items[0];
                
        TComplex^ Number = ( TComplex^ ) result;          

        pmcScalar->real = Number->Real;
        pmcScalar->imag = Number->Imaginary;


    } else if ( type->Equals( array<TComplex^,2>::typeid ) ) {

        array<TComplex^,2>^ Matrix = ( array<TComplex^,2>^ ) result;

        // Согласно документации в функцию MathcadArrayAllocate() должна передаваться
        // ссылка на заполненную структуру COMPLEXARRAY.
        pmcArray = ( COMPLEXARRAY * ) items[0];
                
        int rows = Matrix->GetLength(0);
        int cols = Matrix->GetLength(1);                
                
        bool bReal = false;
        bool bImag = false;
                
        // Проверка наличия действительных частей.           
        for ( int row = 0; row < rows; row++ ) {
            for ( int col = 0; col < cols; col++ ) {

                if ( ( ( TComplex^ ) Matrix[ row, col ] )->Real != 0.0 ) {
                    bReal = true;
                    break;

                }
            }

            if ( bReal == true ) break;
        }                
                
        // Проверка наличия мнимых частей.
        for ( int row = 0; row < rows; row++ ) {
            for ( int col = 0; col < cols; col++ ) {

                if ( ( ( TComplex^ ) Matrix[ row, col ] )->Imaginary != 0.0 ) {
                    bImag = true;
                    break;
                }

            }

            if ( bImag == true ) break;
        }                
                
        if ( ( ( bReal == true ) && ( bImag == true ) ) || ( ( bReal == false ) && ( bImag == true ) ) )  {
                    
            ::MathcadArrayAllocate( pmcArray, rows, cols, TRUE, TRUE );
                    
            for ( int row = 0; row < rows; row++ ) {
                for ( int col = 0; col < cols; col++ ) {

                    ( ( COMPLEXARRAY * ) pmcArray )->hReal[col][row] = ( ( TComplex^ ) Matrix[ row, col ] )->Real;
                    ( ( COMPLEXARRAY * ) pmcArray )->hImag[col][row] = ( ( TComplex^ ) Matrix[ row, col ] )->Imaginary;
                }
            }
        }

        else if ( ( ( bReal == true ) && ( bImag == false ) ) || ( ( bReal == false ) && ( bImag == false ) ) ) {
                    
            ::MathcadArrayAllocate( pmcArray, rows, cols, TRUE, FALSE );
                    
            for ( int row = 0; row < rows; row++ ) {
                for ( int col = 0; col < cols; col++ ) {

                    ( ( COMPLEXARRAY * ) pmcArray )->hReal[col][row] = ( ( TComplex^ ) Matrix[ row, col ] )->Real;
                }
            }
        }

    }

    return true;
}

#pragma unmanaged

LRESULT GlobalFunction( void * out, ... ) {

    int Count = ::GetParamsCount( id ) + 1;

    va_list marker;

    va_start( marker, out );

    PVOID * p = new PVOID[ Count ];

    p[0] = out;

    for ( int n = 1; n < Count; n++ ) {

        p[n] = va_arg( marker, PVOID );
    }

    va_end( marker );

    ::UserFunction( id, p );

    return 0;
}

#pragma managed

void LogInfo( string text ) {

    Manager::LogInfo( marshal_as<String^>( text ) );
}


void LogError( string text ) {

    Manager::LogError( marshal_as<String^>( text ) );
}


void Manager::LogInfo( String^ Text ) {

    File::AppendAllText( LogFile, String::Format( "{0} {1} [INFO ] {2}{3}", 
        DateTime::Now.ToShortDateString(), DateTime::Now.ToLongTimeString(), 
        Text, Environment::NewLine ), Encoding::UTF8 );
}


void Manager::LogError( String^ Text ) {

    File::AppendAllText( LogFile, String::Format( "{0} {1} [ERROR] {2}{3}", 
        DateTime::Now.ToShortDateString(), DateTime::Now.ToLongTimeString(), 
        Text, Environment::NewLine ), Encoding::UTF8 );
}


bool Manager::Initialize() {

    try { File::Delete( Manager::LogFile ); } catch (...) {}

    // TODO: Рассчитать необходимый размер динамической памяти в зависимости от
    // максимального числа поддерживаемых функций.
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

#ifdef _DEBUG

        array<Type^>^ types = Assembly::GetExecutingAssembly()->GetTypes();

        for each ( Type^ type in types ) {

            //if ( type->IsClass ) LogInfo( type->ToString() );

            if ( !type->IsClass || !IFunction::typeid->IsAssignableFrom( type ) ) continue;

            IFunction^ f = ( IFunction^ ) Activator::CreateInstance( type );

            Manager::Items->Add( ( IFunction^ ) Activator::CreateInstance( type ) );
        }

#else
        // Получаем список всех библиотек в текущей папке.
        array< String^ > ^ libs = Directory::GetFiles( Path::GetDirectoryName( AssemblyPath ), gcnew String( "*.dll" ) );

        // Сканируем каждый файл на наличие классов, реализующих интерфейс IFunction.
        for each ( String^ path in libs ) {

            try {
            
                if ( !IsManagedAssembly( path ) || path->Equals( AssemblyPath ) ) continue;

                // LoadFile vs. LoadFrom
                // http://blogs.msdn.com/b/suzcook/archive/2003/09/19/loadfile-vs-loadfrom.aspx
                Assembly^ assembly = Assembly::LoadFrom( path );

                // Assembly и GetType().
                // http://www.rsdn.ru/forum/dotnet/3154438?tree=tree
                // http://www.codeproject.com/KB/cs/pluginsincsharp.aspx
                for each ( Type^ type in assembly->GetTypes() ) {

                    //if ( !type->IsPublic || type->IsAbstract || !type->IsClass || !IFunction::typeid->IsAssignableFrom( type ) ) continue;

                    if ( !type->IsPublic || type->IsAbstract ) continue;

                    Type^ typeInterface = type->GetInterface( "NetEFI.IFunction", true );

                    if ( typeInterface != nullptr ) {

                        Manager::Items->Add( ( IFunction^ ) Activator::CreateInstance( assembly->GetType( type->ToString() ) ) );
                    }

                }

                LogInfo( String::Format( " [LoadLibraries] {0} loaded.", path ) );

            } catch ( ReflectionTypeLoadException^ ex ) {

                for each ( Exception^ e in ex->LoaderExceptions ) {
                
                    LogError( e->Message );
                }
                
                continue;

            } catch ( Exception^ ex ) {

                LogError( ex->Message );
                continue;

            }

        }
#endif
        //LogInfo( String::Format( L"Items.Count: {0}", Manager::Items->Count ) );

        // Теперь регистрируем все функции в Mathcad.
        PBYTE p = pCode;

        for ( int n = 0; n < Manager::Items->Count; n++ ) {
            
            FunctionInfo^ info = Manager::Items[n]->GetFunctionInfo( "RUS" );
            
            Manager::Infos->Add( info );                  

            //LogInfo( gcnew String( "Manager::Infos->Add( info )" ) );

            FUNCTIONINFO fi;
                
			marshal_context context;

			// For VS2008.
			String^ s = info->Name;
            fi.lpstrName = ( char * ) context.marshal_as<const char *>(s);

			s = info->Parameters;
            fi.lpstrParameters = ( char * ) context.marshal_as<const char *>(s);

			s = info->Description;
            fi.lpstrDescription = ( char * ) context.marshal_as<const char *>(s);

            fi.lpfnMyCFunction = ( LPCFUNCTION ) p;

            Type^ type = info->ReturnType;

            // How to check an object's type in C++/CLI?
            // http://stackoverflow.com/questions/2410721/how-to-check-an-objects-type-in-c-cli
            if ( type->Equals( String::typeid ) ) {

                fi.returnType = STRING;

            } else if ( type->Equals( TComplex::typeid ) ) {

                fi.returnType = COMPLEX_SCALAR;

            } else if ( type->Equals( array<TComplex^,2>::typeid ) ) {

                fi.returnType = COMPLEX_ARRAY;            
            
            } else continue;
            
            fi.nArgs = info->Arguments->GetLength(0);
            
            for ( unsigned int m = 0; m < fi.nArgs; m++ ) {

                type = info->Arguments[m];

                if ( type->Equals( String::typeid ) ) {

                    fi.argType[m] = STRING;

                } else if ( type->Equals( TComplex::typeid ) ) {

                    fi.argType[m] = COMPLEX_SCALAR;

                } else if ( type->Equals( array<TComplex^,2>::typeid ) ) {

                    fi.argType[m] = COMPLEX_ARRAY;            
            
                // TODO: continue для внешнего цикла.
                } else break;

            }            

            ::CreateUserFunction( hInstance, & fi );  

            // Пересылка константы (номера функции) в глобальную переменную id.
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
    }

    return true;
}
