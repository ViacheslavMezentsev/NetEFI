// Managed Code and DllMain
// http://msdn.microsoft.com/en-us/library/aa290048(v=vs.71).aspx
// Component Extensions for Runtime Platforms
// http://msdn.microsoft.com/en-us/library/xey702bw.aspx
// Основы миграции C++/CLI
// http://msdn.microsoft.com/ru-ru/library/ms235289.aspx
// C++/CLI Tasks
// http://msdn.microsoft.com/ru-ru/library/hh875047.aspx

#include "stdafx.h"
#include <msclr\marshal_cppstd.h>
#include "mcadincl.h"
#include "test.h"
#include "netefi.h"

using namespace NetEFI;

#define MAX_FUNCTIONS_COUNT 10000UL
#define DYNAMIC_BLOCK_SIZE 25

static int assemblyId = -1;
static int functionId = -1;
static PBYTE pCode = NULL;

PCREATE_USER_FUNCTION CreateUserFunction;
PCREATE_USER_ERROR_MESSAGE_TABLE CreateUserErrorMessageTable;
PMATHCAD_ALLOCATE MathcadAllocate;
PMATHCAD_FREE MathcadFree;
PMATHCAD_ARRAY_ALLOCATE MathcadArrayAllocate;
PMATHCAD_ARRAY_FREE MathcadArrayFree;
PIS_USER_INTERRUPTED isUserInterrupted;


// Обобщённая функция.
LRESULT UserFunction( PVOID items[] )
{
    MCSTRING * pmcString;
    COMPLEXSCALAR * pmcScalar;
    COMPLEXARRAY * pmcArray;

    Type ^ type;
    AssemblyInfo ^ assemblyInfo = nullptr;
    IFunction ^ func = nullptr;

    try
    {
        assemblyInfo = Manager::Assemblies[ assemblyId ];
        func = assemblyInfo->Functions[ functionId ];
    }
    catch ( ... )
    {
        Manager::LogError( "assemblyId: {0}, functionId: {1}", assemblyId, functionId );
        return E_FAIL;
    }

    // Узнаём общее число параметров функции.
    int Count = func->Info->ArgTypes->GetLength(0);

    // Создаём управляемый массив параметров функции.
    array < Object ^ > ^ args = gcnew array < Object ^ >( Count );

    // Преобразуем каждый тип параметра к управляемому аналогу.
    for ( int n = 0; n < Count; n++ )
    {
        type = func->Info->ArgTypes[n];

        // MCSTRING
        // TODO: Преобразовывать ansi в unicode.
        if ( type->Equals( String::typeid ) )
        {
            pmcString = ( MCSTRING * ) items[ n + 1 ];

            args[n] = marshal_as<String ^>( pmcString->str );
        }

        // COMPLEXSCALAR
        else if ( type->Equals( TComplex::typeid ) )
        {
            pmcScalar = ( COMPLEXSCALAR * ) items[ n + 1 ];

            args[n] = gcnew TComplex( pmcScalar->real, pmcScalar->imag );
        }

        // COMPLEXARRAY
        else if ( type->Equals( array<TComplex ^, 2>::typeid ) )
        {
            pmcArray = ( COMPLEXARRAY * ) items[ n + 1 ];

            int rows = pmcArray->rows;
            int cols = pmcArray->cols;

            array<TComplex ^, 2> ^ Matrix = gcnew array<TComplex ^, 2>( rows, cols );

            TComplex ^ tmp;

            for ( int row = 0; row < rows; row++ )
            {
                for ( int col = 0; col < cols; col++ )
                {
                    if ( ( pmcArray->hReal != NULL ) && ( pmcArray->hImag != NULL ) )
                        tmp = gcnew TComplex( pmcArray->hReal[ col ][ row ], pmcArray->hImag[ col ][ row ] );

                    if ( ( pmcArray->hReal != NULL ) && ( pmcArray->hImag == NULL ) )
                        tmp = gcnew TComplex( pmcArray->hReal[ col ][ row ], 0.0 );

                    if ( ( pmcArray->hReal == NULL ) && ( pmcArray->hImag != NULL ) )
                        tmp = gcnew TComplex( 0.0, pmcArray->hImag[ col ][ row ] );

                    Matrix[ row, col ] = tmp;
                }
            }

            args[n] = Matrix;
        }
        else
        {
            Manager::LogError( "[{0}] Unknown argument type {1}: {2}", func->Info->Name, n, type->ToString() );

            return E_FAIL;
        }
    }

    // Вызываем функцию.
    // TODO: Сделать вызов в отдельном потоке.
    Object ^ result;
    Context ^ context = gcnew Context();

    try
    {
        if ( !func->NumericEvaluation( args, result, context ) ) return E_FAIL;
    }
    catch ( EFIException ^ ex )
    {
        if ( assemblyInfo->Errors == nullptr )
        {
            Manager::LogError( "[{0}] {1}", func->Info->Name, "Errors table is empty" );

            return E_FAIL;
        }

        if ( ( ex->ErrNum > 0 )
            && ( ex->ErrNum <= assemblyInfo->Errors->GetLength(0) )
            && ( ex->ArgNum > 0 )
            && ( ex->ArgNum <= func->Info->ArgTypes->GetLength(0) ) )
        {
            // Рассчитываем положение таблицы ошибок для текущей сборки
            // в общей зарегистрированной таблице.
            int offset = 0;

            for ( int n = 0; n < assemblyId; n++ )
            {
                offset += assemblyInfo->Errors->GetLength(0);
            }

            // См. Руководство разработчика библиотек пользователя.
            return MAKELRESULT( offset + ex->ErrNum, ex->ArgNum );
        }
        else
        {
            Manager::LogError( "[{0}] {1}", func->Info->Name, ex->Message );

            return E_FAIL;
        }

    }
    catch ( System::Exception ^ ex )
    {
        Manager::LogError( "[{0}] {1}", func->Info->Name, ex->Message );

        return E_FAIL;
    }

    // Преобразуем результат.
    type = result->GetType();

    // MCSTRING
    // TODO: Преобразовывать unicode в ansi.
    if ( type->Equals( String::typeid ) )
    {
        // Выделяем память под структуру.
        pmcString = ( MCSTRING * ) items[0];

        marshal_context context;

        char * text = ( char * ) context.marshal_as<const char *>( ( String ^ ) result );

        // Выделяем память для строки и завершающего нуля.
        pmcString->str = ( char * ) ::MathcadAllocate( ::strlen( text ) + 1 );

        ::memset( pmcString->str, 0, ::strlen( text ) + 1 );

        // Копируем строку из временной области памяти.
        ::memcpy( pmcString->str, text, ::strlen( text ) );        
    }

    // COMPLEXSCALAR
    else if ( type->Equals( TComplex::typeid ) )
    {
        pmcScalar = ( COMPLEXSCALAR * ) items[0];

        TComplex ^ Number = ( TComplex ^ ) result;

        pmcScalar->real = Number->Real;
        pmcScalar->imag = Number->Imaginary;
    }

    // COMPLEXARRAY
    else if ( type->Equals( array<TComplex ^, 2>::typeid ) )
    {
        array<TComplex ^, 2> ^ Matrix = ( array<TComplex ^, 2> ^ ) result;

        // Согласно документации в функцию MathcadArrayAllocate() должна передаваться
        // ссылка на заполненную структуру COMPLEXARRAY.
        pmcArray = ( COMPLEXARRAY * ) items[0];

        int rows = Matrix->GetLength( 0 );
        int cols = Matrix->GetLength( 1 );

        bool bReal = false;
        bool bImag = false;

        // Проверка наличия действительных частей.           
        for ( int row = 0; row < rows; row++ )
        {
            for ( int col = 0; col < cols; col++ )
            {
                if ( ( ( TComplex ^ ) Matrix[ row, col ] )->Real != 0.0 )
                {
                    bReal = true;
                    break;
                }
            }
            
            if ( bReal == true ) break;
        }

        // Проверка наличия мнимых частей.
        for ( int row = 0; row < rows; row++ )
        {
            for ( int col = 0; col < cols; col++ )
            {
                if ( ( ( TComplex ^ ) Matrix[ row, col ] )->Imaginary != 0.0 )
                {
                    bImag = true;
                    break;
                }
            }

            if ( bImag == true ) break;
        }

        if ( ( ( bReal == true ) && ( bImag == true ) ) || ( ( bReal == false ) && ( bImag == true ) ) )
        {
            ::MathcadArrayAllocate( pmcArray, rows, cols, TRUE, TRUE );

            for ( int row = 0; row < rows; row++ )
            {
                for ( int col = 0; col < cols; col++ )
                {
                    ( ( COMPLEXARRAY * ) pmcArray )->hReal[ col ][ row ] = ( ( TComplex ^ ) Matrix[ row, col ] )->Real;
                    ( ( COMPLEXARRAY * ) pmcArray )->hImag[ col ][ row ] = ( ( TComplex ^ ) Matrix[ row, col ] )->Imaginary;
                }
            }
        }

        else if ( ( ( bReal == true ) && ( bImag == false ) ) || ( ( bReal == false ) && ( bImag == false ) ) )
        {
            ::MathcadArrayAllocate( pmcArray, rows, cols, TRUE, FALSE );

            for ( int row = 0; row < rows; row++ )
            {
                for ( int col = 0; col < cols; col++ )
                {
                    ( ( COMPLEXARRAY * ) pmcArray )->hReal[ col ][ row ] = ( ( TComplex ^ ) Matrix[ row, col ] )->Real;
                }
            }
        }
    }
    else
    {
        Manager::LogError( "[{0}] Unknown return type: {1}", func->Info->Name, type->ToString() );

        return E_FAIL;
    }

    return S_OK;
}


#pragma unmanaged

LRESULT CallbackFunction( void * out, ... )
{
    return ::UserFunction( & out );
}

#pragma managed


bool Context::IsUserInterrupted::get()
{
    return ::isUserInterrupted == NULL ? false : ::isUserInterrupted();
}


bool Context::IsDefined( String ^ name )
{
    return default[ name ] != nullptr;
}


IFunction ^ Context::default::get( String ^ name )
{
    IFunction ^ res = nullptr;

    for each ( AssemblyInfo ^ assembly in Manager::Assemblies )
    {
        for each ( IFunction ^ func in assembly->Functions )
        {
            if ( func->Info->Name->Equals( name ) )
            {
                res = func;
                break;
            }
            else
            {
                continue;
            }
        }

        if ( res != nullptr ) break;
    }

    return res;
}


void Context::LogInfo( String ^ text )
{
    Manager::LogInfo( text );
}


/// <summary>
/// This handler is called only when the CLR tries to bind to the assembly and fails
/// </summary>
/// <param name="sender">Event originator</param>
/// <param name="args">Event data</param>
/// <returns>The loaded assembly</returns>
Assembly ^ OnAssemblyResolve( Object ^ sender, ResolveEventArgs ^ args )
{
    Manager::LogInfo( "[OnAssemblyResolve] {0}", args->Name );

    Assembly ^ retval = nullptr;
    String ^ finalPath = nullptr;

    // Load the assembly from the specified path
    try
    {
        finalPath = args->Name->Substring( 0, args->Name->IndexOf( "," ) ) + gcnew String( ".dll" );

        finalPath = Path::Combine( Manager::AssemblyDirectory, finalPath );

        if ( File::Exists( finalPath ) )
        {
            retval = Assembly::LoadFile( finalPath );

            Manager::LogInfo( "Assembly loaded: {0}", finalPath );
        }
        else
        {
            Manager::LogInfo( "File not found: {0}", finalPath );
        }
    }
    catch ( System::Exception ^ ex )
    {
        Manager::LogError( "Assembly not loaded: {0}", ex->Message );
    }

    return retval;
}


void PrepareManagedCode()
{
    // Set up our resolver for assembly loading.
    AppDomain ^ currentDomain = AppDomain::CurrentDomain;

    currentDomain->AssemblyResolve += gcnew ResolveEventHandler( OnAssemblyResolve );
}


// TODO: VirtualFreeEx (?)
bool Manager::Initialize()
{
    // Loading Mixed-Mode C++/CLI .dll (and dependencies) dynamically from unmanaged c++
    // http://stackoverflow.com/questions/7016663/loading-mixed-mode-c-cli-dll-and-dependencies-dynamically-from-unmanaged-c
    PrepareManagedCode();

    try
    { 
        File::Delete( LogFile ); 
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

    // Рассчёт необходимого размера динамической памяти в зависимости от
    // максимального числа поддерживаемых функций.
    size_t size = MAX_FUNCTIONS_COUNT * DYNAMIC_BLOCK_SIZE;

    ::pCode = ( PBYTE ) ::VirtualAllocEx( ::GetCurrentProcess(), 0, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );

    // TODO: Обработка кодов ошибок.
    if ( ::pCode == NULL )
    {
        LogInfo( "VirtualAllocEx() failed." );

        return false;
    }

    String ^ path = Manager::AssemblyDirectory;

    path = Path::Combine( path, gcnew String( L"..\\mcaduser.dll" ) );

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

        ErrorMessageTable[n] = ::MathcadAllocate( s.length() + 1 );

        ::memset( ErrorMessageTable[n], 0, s.length() + 1 );

        // Копируем строку из временной области памяти.
        ::memcpy( ErrorMessageTable[n], s.c_str(), s.length() );
    }

    // Функция копирует содержимое таблицы во внутреннюю память.
    if ( !::CreateUserErrorMessageTable( ::GetModuleHandle( NULL ), count, ErrorMessageTable ) )
    {
        LogError( "[CreateUserErrorMessageTable] failed" );
    }

    // Освобождаем выделенную память.
    for ( int n = 0; n < count; n++ )
    {
        ::MathcadFree( ErrorMessageTable[n] );
    }

    delete[] ErrorMessageTable;
}


PVOID Manager::CreateUserFunction( FunctionInfo ^ info, PVOID p )
{
    try
    {
        FUNCTIONINFO fi;

        marshal_context context;

        String ^ s = info->Name;
        fi.lpstrName = ( char * ) context.marshal_as<const char *>( s );

        s = info->Parameters;
        fi.lpstrParameters = ( char * ) context.marshal_as<const char *>( s );

        s = info->Description;
        fi.lpstrDescription = ( char * ) context.marshal_as<const char *>( s );

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
            type = info->ArgTypes[m];

            if ( type->Equals( String::typeid ) )
            {
                fi.argType[m] = STRING;
            }
            else if ( type->Equals( TComplex::typeid ) )
            {
                fi.argType[m] = COMPLEX_SCALAR;
            }
            else if ( type->Equals( array<TComplex ^, 2>::typeid ) )
            {
                fi.argType[m] = COMPLEX_ARRAY;
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
