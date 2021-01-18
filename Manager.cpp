#include "stdafx.h"
#include "TComplex.h"
#include "test.h"
#include "Manager.h"

using namespace NetEFI;

extern LRESULT CallbackFunction( void * out, ... );

#define MAX_FUNCTIONS_COUNT 10000UL
#define DYNAMIC_BLOCK_SIZE 25

int assemblyId = -1;
int functionId = -1;
PBYTE pCode = NULL;

void Manager::Log( String ^ format, ... array<Object ^> ^ list )
{
    String ^ text = String::Format( format, list );

    text = String::Format( "{0:dd.MM.yyyy HH:mm:ss} {1}{2}", DateTime::Now, text, Environment::NewLine );

    try
    {
        File::AppendAllText( LogFile, text, Encoding::UTF8 );
    }
    catch ( ... )
    {
    }
}


// TODO: VirtualFreeEx (?)
bool Manager::Initialize()
{
    try
    { 
        if ( File::Exists( LogFile ) ) File::Delete( LogFile );
    }
    catch ( ... ) {}

    bool is64Bit = Marshal::SizeOf( IntPtr::typeid ) == 8;

    auto name = ExecAssembly->GetName();
    auto version = name->Version;

    DateTime ^ bdate = ( gcnew DateTime( 2000, 1, 1 ) )->AddDays( version->Build );

    bdate = bdate->AddSeconds( 2 * version->Revision );

    LogInfo( ".Net: {0}", Environment::Version );
    LogInfo( "{0}-bit", ( is64Bit ? "64" : "32" ) );
    LogInfo( "netefi, version {0}, {1:dd-MMM-yyyy HH:mm:ss}", version, bdate );

    // Расчёт необходимого размера динамической памяти в зависимости от
    // максимального числа поддерживаемых функций.
    size_t size = MAX_FUNCTIONS_COUNT * DYNAMIC_BLOCK_SIZE;

    ::pCode = ( PBYTE ) ::VirtualAllocEx( ::GetCurrentProcess(), 0, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );

    // TODO: Обработка кодов ошибок.
    if ( ::pCode == NULL )
    {
        LogError( "VirtualAllocEx() failed." );

        return false;
    }

    String ^ path = Path::Combine( Manager::AssemblyDirectory, "..\\mcaduser.dll" );

    if ( !File::Exists( path ) )
    {
        LogError( "File not found: {0}", path );

        return false;
    }

    marshal_context context;

    HMODULE hLib = ::LoadLibraryW( context.marshal_as<LPCWSTR>( path ) );

    if ( hLib == NULL )
    {
        LogError( "[LoadLibrary] returns NULL." );

        return false;
    }

    ::CreateUserFunction = ( PCREATE_USER_FUNCTION ) ::GetProcAddress( hLib, "CreateUserFunction" );
    ::CreateUserErrorMessageTable = ( PCREATE_USER_ERROR_MESSAGE_TABLE ) ::GetProcAddress( hLib, "CreateUserErrorMessageTable" );
    ::MathcadAllocate = ( PMATHCAD_ALLOCATE ) ::GetProcAddress( hLib, "MathcadAllocate" );
    ::MathcadFree = ( PMATHCAD_FREE ) ::GetProcAddress( hLib, "MathcadFree" );
    ::MathcadArrayAllocate = ( PMATHCAD_ARRAY_ALLOCATE ) ::GetProcAddress( hLib, "MathcadArrayAllocate" );
    ::MathcadArrayFree = ( PMATHCAD_ARRAY_FREE ) ::GetProcAddress( hLib, "MathcadArrayFree" );
    ::isUserInterrupted = ( PIS_USER_INTERRUPTED ) ::GetProcAddress( hLib, "isUserInterrupted" );

    if ( ::CreateUserFunction == NULL
        || ::CreateUserErrorMessageTable == NULL
        || ::MathcadAllocate == NULL
        || ::MathcadFree == NULL
        || ::MathcadArrayAllocate == NULL
        || ::MathcadArrayFree == NULL
        || ::isUserInterrupted == NULL )
    {
        LogError( "[GetProcAddress] returns NULL." );

        return false;
    }

    return true;
}

// How to determine whether a DLL is a managed assembly or native (prevent loading a native dll)?
// http://stackoverflow.com/questions/367761/how-to-determine-whether-a-dll-is-a-managed-assembly-or-native-prevent-loading
bool Manager::IsManagedAssembly( String ^ fileName )
{
    Stream ^ fileStream = gcnew FileStream( fileName, IO::FileMode::Open, IO::FileAccess::Read );

    BinaryReader ^ binaryReader = gcnew BinaryReader( fileStream );

    if ( fileStream->Length < 64 ) return false;

    //PE Header starts @ 0x3C (60). Its a 4 byte header.
    fileStream->Position = 0x3C;

    UINT peHeaderPointer = binaryReader->ReadUInt32();

    if ( peHeaderPointer == 0 ) peHeaderPointer = 0x80;

    // Ensure there is at least enough room for the following structures:
    //     24 byte PE Signature & Header
    //     28 byte Standard Fields         (24 bytes for PE32+)
    //     68 byte NT Fields               (88 bytes for PE32+)
    // >= 128 byte Data Dictionary Table
    if ( peHeaderPointer > fileStream->Length - 256 ) return false;

    // Check the PE signature.  Should equal 'PE\0\0'.
    fileStream->Position = peHeaderPointer;

    UINT peHeaderSignature = binaryReader->ReadUInt32();

    if ( peHeaderSignature != 0x00004550 ) return false;

    // skip over the PEHeader fields
    fileStream->Position += 20;

    const USHORT PE32 = 0x10b;
    const USHORT PE32Plus = 0x20b;

    // Read PE magic number from Standard Fields to determine format.
    USHORT peFormat = binaryReader->ReadUInt16();

    if ( peFormat != PE32 && peFormat != PE32Plus ) return false;

    // Read the 15th Data Dictionary RVA field which contains the CLI header RVA.
    // When this is non-zero then the file contains CLI data otherwise not.
    USHORT dataDictionaryStart = ( USHORT ) ( peHeaderPointer + ( peFormat == PE32 ? 232 : 248 ) );

    fileStream->Position = dataDictionaryStart;

    UINT cliHeaderRva = binaryReader->ReadUInt32();

    if ( cliHeaderRva == 0 ) return false;

    return true;
}


void Manager::CreateUserErrorMessageTable( array < String ^ > ^ errors )
{
    char ** ErrorMessageTable;

    int count = errors->GetLength(0);

    ErrorMessageTable = new char * [ count ];

    for ( int n = 0; n < count; n++ )
    {
        String ^ text = errors[n];

        std::string s = marshal_as<std::string>( text );

        char * msgItem = ::MathcadAllocate( s.length() + 1 );

        ErrorMessageTable[n] = msgItem;

        ::memset( msgItem, 0, s.length() + 1 );

        // Копируем строку из временной области памяти.
        ::memcpy( msgItem, s.c_str(), s.length() );
    }

    // Функция копирует содержимое таблицы во внутреннюю память.
    if ( !::CreateUserErrorMessageTable( ::GetModuleHandle( NULL ), count, ErrorMessageTable ) )
    {
        LogError( "[CreateUserErrorMessageTable] failed" );
    }

    // Освобождаем выделенную память.
    for ( int n = 0; n < count; n++ ) ::MathcadFree( ErrorMessageTable[n] );

    delete[] ErrorMessageTable;
}


PVOID Manager::CreateUserFunction( FunctionInfo ^ info, PVOID p )
{
    try
    {
        FUNCTIONINFO fi;

        marshal_context context;

        String ^ s = info->Name;
        fi.lpstrName = ( char * ) context.marshal_as<const char *>(s);

        s = info->Parameters;
        fi.lpstrParameters = ( char * ) context.marshal_as<const char *>(s);

        s = info->Description;
        fi.lpstrDescription = ( char * ) context.marshal_as<const char *>(s);

        fi.lpfnMyCFunction = ( LPCFUNCTION ) p;

        Type ^ type = info->ReturnType;

        // How to check an object's type in C++/CLI?
        // http://stackoverflow.com/questions/2410721/how-to-check-an-objects-type-in-c-cli
        if ( type->Equals( String::typeid ) )
        {
            fi.returnType = STRING;
        }
        else if ( type->Equals( TComplex::typeid ) )
        {
            fi.returnType = COMPLEX_SCALAR;
        }
        else if ( type->Equals( array<TComplex ^, 2>::typeid ) )
        {
            fi.returnType = COMPLEX_ARRAY;
        }
        else
        {
            LogError( "[{0}] Unknown return type: {1}", info->Name, type->ToString() );

            return NULL;
        }

        fi.nArgs = info->ArgTypes->GetLength(0);

        if ( fi.nArgs == 0 )
        {
            LogInfo( "[{0}] No arguments (must be between 1 and {1}).", info->Name, MAX_ARGS );

            return NULL;
        }

        if ( fi.nArgs > MAX_ARGS )
        {
            LogInfo( "[{0}] Too many arguments: {1}. Cut to MAX_ARGS = {2}", info->Name, fi.nArgs, MAX_ARGS );

            fi.nArgs = MAX_ARGS;
        }

        for ( unsigned int m = 0; m < fi.nArgs; m++ )
        {
            type = info->ArgTypes[ m ];

            if ( type->Equals( String::typeid ) )
            {
                fi.argType[ m ] = STRING;
            }
            else if ( type->Equals( TComplex::typeid ) )
            {
                fi.argType[ m ] = COMPLEX_SCALAR;
            }
            else if ( type->Equals( array<TComplex ^, 2>::typeid ) )
            {
                fi.argType[ m ] = COMPLEX_ARRAY;
            }
            else
            {
                LogError( "[{0}] Unknown argument type {1}: {2}", info->Name, m, type->ToString() );

                return NULL;
            }
        }

        return ::CreateUserFunction( ::GetModuleHandle( NULL ), & fi );
    }
    catch ( System::Exception ^ ex )
    {
        LogError( "[{0}] {1}", info->Name, ex->Message );

        return NULL;
    }
}


// TODO: 64-bit.
void Manager::InjectCode( PBYTE & p, int k, int n )
{
    // Пересылка константы (номера сборки) в глобальную переменную.
    *p++ = 0xB8; // mov eax, imm32
    p[0] = k;
    p += sizeof( int );

    *p++ = 0xA3; // mov [assemblyId], eax
    ( int *& ) p[0] = & ::assemblyId;
    p += sizeof( int * );

    // Пересылка константы (номера функции) в глобальную переменную.
    *p++ = 0xB8; // mov eax, imm32
    p[0] = n;
    p += sizeof( int );

    *p++ = 0xA3; // mov [functionId], eax
    ( int *& ) p[0] = & ::functionId;
    p += sizeof( int * );

    // jmp to CallbackFunction. 
    *p++ = 0xE9;
    ( UINT & ) p[0] = ( PBYTE ) ::CallbackFunction - 4 - p;
    p += sizeof( PBYTE );
}


// Загрузка пользовательских библиотек.
bool Manager::LoadAssemblies()
{
    try
    {
        // Списки функций и их карточек.
        Manager::Assemblies = gcnew List < AssemblyInfo ^ >();

#ifdef _DEBUG

        LogInfo( "[netefi] Debug mode" );

        AssemblyInfo ^ assemblyInfo = gcnew AssemblyInfo();
        assemblyInfo->Path = AssemblyDirectory;
        assemblyInfo->Functions = gcnew List< IFunction ^ >();

        array<Type ^> ^ types = Assembly::GetExecutingAssembly()->GetTypes();

        for each ( Type ^ type in types )
        {
            if ( !type->IsPublic || type->IsAbstract || !IFunction::typeid->IsAssignableFrom( type ) ) continue;

            IFunction ^ f = ( IFunction ^ ) Activator::CreateInstance( type );

            assemblyInfo->Functions->Add( ( IFunction ^ ) Activator::CreateInstance( type ) );

            // Проверяем наличие таблицы с сообщениями об обшибках.
            if ( assemblyInfo->Errors != nullptr ) continue;

            FieldInfo ^ errorsFieldInfo = type->GetField( gcnew String( "Errors" ),
                Reflection::BindingFlags::GetField | Reflection::BindingFlags::Static | Reflection::BindingFlags::Public );

            if ( errorsFieldInfo == nullptr ) continue;

            assemblyInfo->Errors = ( array < String ^ > ^ ) errorsFieldInfo->GetValue( nullptr );
        }

        if ( assemblyInfo->Functions->Count > 0 ) Assemblies->Add( assemblyInfo );

#else
        // Получаем список всех библиотек в текущей папке.
        array< String ^ > ^ libs = Directory::GetFiles( AssemblyDirectory, gcnew String( "*.dll" ) );

        // Сканируем каждый файл на наличие классов, реализующих интерфейс IFunction.
        for each ( String ^ path in libs )
        {
            try
            {
                if ( !IsManagedAssembly( path ) || path->Equals( ( gcnew System::Uri( ExecAssembly->CodeBase ) )->LocalPath ) ) continue;

                AssemblyInfo ^ assemblyInfo = gcnew AssemblyInfo();

                // LoadFile vs. LoadFrom
                // http://blogs.msdn.com/b/suzcook/archive/2003/09/19/loadfile-vs-loadfrom.aspx
                Assembly ^ assembly = Assembly::LoadFile( path );

                assemblyInfo->Path = path;
                assemblyInfo->Functions = gcnew List< IFunction ^ >();

                // Assembly и GetType().
                // http://www.rsdn.ru/forum/dotnet/3154438?tree=tree
                // http://www.codeproject.com/KB/cs/pluginsincsharp.aspx
                for each ( Type ^ type in assembly->GetTypes() )
                {
                    if ( !type->IsPublic || type->IsAbstract || !IFunction::typeid->IsAssignableFrom( type ) ) continue;

                    assemblyInfo->Functions->Add( ( IFunction ^ ) Activator::CreateInstance( type ) );

                    // Проверяем наличие таблицы с сообщениями об обшибках.
                    if ( assemblyInfo->Errors != nullptr ) continue;

                    FieldInfo ^ errorsFieldInfo = type->GetField( gcnew String( "Errors" ),
                        Reflection::BindingFlags::GetField | Reflection::BindingFlags::Static | Reflection::BindingFlags::Public );

                    if ( errorsFieldInfo == nullptr ) continue;

                    assemblyInfo->Errors = ( array < String ^ > ^ ) errorsFieldInfo->GetValue( nullptr );
                }

                if ( assemblyInfo->Functions->Count > 0 ) Assemblies->Add( assemblyInfo );
            }
            catch ( Exception ^ ex )
            {
                LogError( ex->Message );
                continue;
            }
        }
#endif

        // Теперь регистрируем все функции в Mathcad.        
        int totalCount = 0;
        PBYTE p = pCode;
        List<String ^> ^ errorMessageTable = gcnew List<String ^>();

        for ( int k = 0; k < Assemblies->Count; k++ )
        {
            AssemblyInfo ^ assemblyInfo = Assemblies[k];

            // Соединияем все таблицы.
            if ( assemblyInfo->Errors != nullptr )
            {
                errorMessageTable->AddRange( assemblyInfo->Errors );
            }

            for ( int n = 0; n < assemblyInfo->Functions->Count; n++ )
            {
                if ( totalCount >= MAX_FUNCTIONS_COUNT ) break;

                String ^ lang = CultureInfo::CurrentCulture->ThreeLetterISOLanguageName;

                FunctionInfo ^ info = assemblyInfo->Functions[n]->GetFunctionInfo( lang );

                if ( CreateUserFunction( info, p ) == NULL ) continue;

                InjectCode( p, k, n );

                List< String ^ > ^ params = gcnew List< String ^ >();

                for each ( Type ^ type in info->ArgTypes ) params->Add( type->ToString() );

                String ^ args = String::Join( gcnew String( "," ), params->ToArray() );

                String ^ text = ( info->Parameters->Length > 0 )
                    ? String::Format( "[ {0} ] {1}", info->Parameters, info->Description )
                    : String::Format( "[ {0} ] {1}", args, info->Description );

                LogInfo( "[{0}] [{1}] {2} - {3}", totalCount, Path::GetFileName( assemblyInfo->Path ), info->Name, text );

                totalCount++;
            }
        }

        // Таблица может быть только одна.
        CreateUserErrorMessageTable( errorMessageTable->ToArray() );

    }
    catch ( System::Exception ^ ex )
    {
        LogError( ex->Message );

        return false;
    }

    return true;
}
