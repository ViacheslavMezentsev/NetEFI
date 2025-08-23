#include "stdafx.h"
#include "Manager.h"

using namespace NetEFI;

extern LRESULT CallbackFunction( void * out, ... );


CMathcadEfi MathcadEfi;

CMathcadEfi::CMathcadEfi()
    : Attached(false), AssemblyId(-1), FunctionId(-1), DynamicCode(nullptr),
    CreateUserFunction( nullptr ),
    CreateUserErrorMessageTable( nullptr ),
    MathcadAllocate(nullptr),
    MathcadFree(nullptr),
    MathcadArrayAllocate(nullptr),
    MathcadArrayFree(nullptr),
    isUserInterrupted(nullptr)
{
    auto moduleName = Path::Combine( Manager::AssemblyDirectory, "..\\mcaduser.dll" );

    marshal_context context;

    HMODULE hLib = ::GetModuleHandleW( context.marshal_as<LPCWSTR>( moduleName ) );

    if ( hLib != NULL )
    {            
        CreateUserFunction = ( PCREATE_USER_FUNCTION ) ::GetProcAddress( hLib, "CreateUserFunction" );
        CreateUserErrorMessageTable = ( PCREATE_USER_ERROR_MESSAGE_TABLE ) ::GetProcAddress( hLib, "CreateUserErrorMessageTable" );
        MathcadAllocate = ( PMATHCAD_ALLOCATE ) ::GetProcAddress( hLib, "MathcadAllocate" );
        MathcadFree = ( PMATHCAD_FREE ) ::GetProcAddress( hLib, "MathcadFree" );
        MathcadArrayAllocate = ( PMATHCAD_ARRAY_ALLOCATE ) ::GetProcAddress( hLib, "MathcadArrayAllocate" );
        MathcadArrayFree = ( PMATHCAD_ARRAY_FREE ) ::GetProcAddress( hLib, "MathcadArrayFree" );
        isUserInterrupted = ( PIS_USER_INTERRUPTED ) ::GetProcAddress( hLib, "isUserInterrupted" );
    }
    else
    {
        Manager::LogError( "Can't get module handle for mcaduser.dll." );
    }

    Attached = ( hLib && CreateUserFunction && CreateUserErrorMessageTable && MathcadAllocate && MathcadFree
        && MathcadArrayAllocate && MathcadArrayFree && isUserInterrupted );
}


CMathcadEfi::~CMathcadEfi()
{
    try
    {
        if ( DynamicCode != nullptr ) ::VirtualFreeEx( ::GetCurrentProcess(), DynamicCode, 0, MEM_RELEASE );

        if ( File::Exists( Manager::LogFile ) ) File::Delete( Manager::LogFile );
    }
    catch ( ... ) {}
}


void Manager::Log( String ^ text )
{
    text = String::Format( "{0:dd.MM.yyyy HH:mm:ss} {1}{2}", DateTime::Now, text, Environment::NewLine );

    try
    {
        File::AppendAllText( LogFile, text, Encoding::UTF8 );
    }
    catch ( ... ) { }
}


void Manager::Log( String ^ format, ... array<Object ^> ^ list )
{
    Log( String::Format( format, list ) );
}


/// <summary>
/// This handler is called only when the CLR tries to bind to the assembly and fails
/// </summary>
/// <param name="sender">Event originator</param>
/// <param name="args">Event data</param>
/// <returns>The loaded assembly</returns>
Assembly ^ Manager::OnAssemblyResolve( Object ^ sender, ResolveEventArgs ^ args )
{
    Assembly ^ retval = nullptr;

    // Load the assembly from the specified path.
    try
    {
        auto fields = args->Name->Split( ',' );

        auto name = fields[0];

        if ( name->EndsWith( ".resources" ) && fields->Length > 2 )
        {
            auto culture = fields[2];

            if ( !culture->EndsWith( "neutral" ) ) return retval;
        }

        if ( name->Equals( ExecAssembly->GetName()->Name ) ) return ExecAssembly;

        name = name + ".dll";        

        String ^ path = Path::Combine( AssemblyDirectory, name );

        if ( File::Exists( path ) )
        {
            retval = Assembly::LoadFile( path );

            LogInfo( "Assembly loaded: {0}", path );
        }
        else
        {
            LogInfo( "Assembly not found: {0}", path );
        }
    }
    catch ( System::Exception ^ ex )
    {
        LogError( "Assembly not loaded: {0}", ex->Message );
    }

    return retval;
}


bool Manager::Initialize()
{
    auto process = Process::GetCurrentProcess();

    LogInfo( "OS: {0}, 64-bit: {1}", Environment::OSVersion->ToString(), Environment::Is64BitOperatingSystem );
    LogInfo( "Current Culture: {0}", CultureInfo::CurrentCulture->Name );

    // Проверка прав администратора
    auto identity = System::Security::Principal::WindowsIdentity::GetCurrent();
    auto principal = gcnew System::Security::Principal::WindowsPrincipal( identity );
    bool isAdmin = principal->IsInRole( System::Security::Principal::WindowsBuiltInRole::Administrator );

    LogInfo( "Running as Administrator: {0}", isAdmin );

    auto fileInfo = FileVersionInfo::GetVersionInfo(process->MainModule->FileName);

    LogInfo("{0} version {1}, {2}", fileInfo->ProductName, fileInfo->ProductVersion, fileInfo->LegalCopyright);

    auto aname = ExecAssembly->GetName();

    auto version = aname->Version;

    auto bdate = ( gcnew DateTime( 2000, 1, 1 ) )->AddDays( version->Build ).AddSeconds( 2 * version->Revision );

    LogInfo( ".Net: {0}", Environment::Version );

#ifdef _DEBUG
    LogInfo( "{0}: {1}-bit debug version {2} ({3:dd-MMM-yyyy HH:mm:ss})", aname->Name, ( Environment::Is64BitProcess ? "64" : "32" ), version, bdate );
#else
    LogInfo( "{0}: {1}-bit release version {2} ({3:dd-MMM-yyyy HH:mm:ss})", aname->Name, ( Environment::Is64BitProcess ? "64" : "32" ), version, bdate );
#endif

    return MathcadEfi.Attached;
}


// How to determine whether a DLL is a managed assembly or native (prevent loading a native dll)?
// http://stackoverflow.com/questions/367761/how-to-determine-whether-a-dll-is-a-managed-assembly-or-native-prevent-loading
bool Manager::IsManagedAssembly( String ^ fileName )
{
    Stream ^ fileStream = gcnew FileStream( fileName, FileMode::Open, FileAccess::Read );

    BinaryReader ^ binaryReader = gcnew BinaryReader( fileStream );

    if ( fileStream->Length < 64 ) return false;

    // PE Header starts @ 0x3C (60). Its a 4 byte header.
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

    return ( cliHeaderRva != 0 );
}


void Manager::CreateUserErrorMessageTable( array < String ^ > ^ errors )
{
    marshal_context context;

    int count = errors->GetLength(0);

    PCHAR * errorMessages = new PCHAR[ count ];

    for ( int n = 0; n < count; n++ )
    {
        String ^ text = errors[n];

        errorMessages[n] = ( PCHAR ) context.marshal_as<const char *>( text );
    }

    // Copy table content to the inner memory.
    if ( !MathcadEfi.CreateUserErrorMessageTable( ::GetModuleHandle( NULL ), count, errorMessages ) )
    {
        LogError( "Create user error messages table failed" );
    }

    delete[] errorMessages;
}


PVOID Manager::CreateUserFunction( FunctionInfo^ info, PVOID p )
{
    FUNCTIONINFO fi {};

    marshal_context context;

    auto types = gcnew Dictionary<Type ^, unsigned long>();

    types->Add( String::typeid, STRING );
    types->Add( Complex::typeid, COMPLEX_SCALAR );
    types->Add( array<Complex, 2>::typeid, COMPLEX_ARRAY );

    try
    {
        String ^ s = info->Name;
        fi.lpstrName = ( char * ) context.marshal_as<const char *>(s);

        s = info->Parameters;
        fi.lpstrParameters = ( char * ) context.marshal_as<const char *>(s);

        s = info->Description;
        fi.lpstrDescription = ( char * ) context.marshal_as<const char *>(s);

        fi.lpfnMyCFunction = ( LPCFUNCTION ) p;

        Type ^ type = info->ReturnType;

        if ( !types->ContainsKey( type ) )
        {
            LogError( "[{0}] Unknown return type: {1}", info->Name, type->ToString() );

            return nullptr;
        }

        fi.returnType = types[ type ];

        fi.nArgs = info->ArgTypes->GetLength(0);

        if ( fi.nArgs == 0 )
        {
            LogInfo( "[{0}] No arguments (must be between 1 and {1}).", info->Name, MAX_ARGS );

            return nullptr;
        }

        if ( fi.nArgs > MAX_ARGS )
        {
            LogInfo( "[{0}] Too many arguments: {1}. Cut to MAX_ARGS = {2}", info->Name, fi.nArgs, MAX_ARGS );

            fi.nArgs = MAX_ARGS;
        }

        for ( unsigned int m = 0; m < fi.nArgs; m++ )
        {
            type = info->ArgTypes[m];

            if ( !types->ContainsKey( type ) )
            {
                LogError( "[{0}] Unknown argument type {1}: {2}", info->Name, m, type->ToString() );

                return nullptr;
            }

            fi.argType[m] = types[ type ];
        }

        return MathcadEfi.CreateUserFunction( ::GetModuleHandle( NULL ), & fi );
    }
    catch ( System::Exception ^ ex )
    {
        LogError( "[{0}] {1}", info->Name, ex->Message );

        return nullptr;
    }
}


// mov eax, imm32
#define imm2eax(x)  *p++ = 0xB8; ( int & ) p[0] = x; p += sizeof( int )

// mov mem, eax
#define eax2mem(y)  *p++ = 0xA3; ( void *& ) p[0] = & y; p += sizeof( void * )

// mov mem, imm32
#define imm2mem(x,y)    imm2eax(x); eax2mem(y);

// mov rax, CallbackFunction
// jmp rax
#define jump(addr)  *p++ = 0x48; *p++ = 0xB8; ( PBYTE & ) p[0] = ( PBYTE ) addr; p += sizeof( void * ); *p++ = 0xFF; *p++ = 0xE0

// The magic part is here.
void Manager::InjectCode( PBYTE & p, int assemblyId, int functionId )
{
    imm2mem( assemblyId, MathcadEfi.AssemblyId );

    imm2mem( functionId, MathcadEfi.FunctionId );

    jump( ::CallbackFunction );
}


// Register user functions.
bool Manager::RegisterFunctions()
{
	if ( Assemblies == nullptr ) return false;

	int totalCount = 0;
	PBYTE pCode = MathcadEfi.DynamicCode;
	auto errorMessages = gcnew List<String^>();

	try
	{
		for ( int k = 0; k < Assemblies->Count; k++ )
		{
			auto assemblyInfo = Assemblies[k];
			int count = 0;

			for ( int n = 0; n < assemblyInfo->Functions->Count; n++ )
			{
				auto funcobj = ( IComputable^ ) assemblyInfo->Functions[n];

				// Просто берем готовый FunctionInfo!
				auto info = funcobj->GetFunctionInfo( CultureInfo::CurrentCulture->ThreeLetterISOLanguageName );

				if ( CreateUserFunction( info, pCode ) == nullptr ) continue;

				InjectCode( pCode, k, n );

				auto text = String::Format( "[ {0} ] {1}", info->Parameters, info->Description );
				LogInfo( "{0} - {1}", info->Name, text );

				count++;
				totalCount++;

				// Логика с таблицей ошибок остается прежней
				auto errorsFieldInfo = funcobj->GetType()->GetField( "Errors", BindingFlags::GetField | BindingFlags::Static | BindingFlags::Public );

                if ( errorsFieldInfo != nullptr )
				{
					try
					{
						auto errors = ( array<String^>^ ) errorsFieldInfo->GetValue( nullptr );

                        if ( errors != nullptr ) errorMessages->AddRange( errors );
					}
					catch ( Exception^ ex )
					{
						Manager::LogError( "Failed to get 'Errors' field from type {0}: {1}",
							errorsFieldInfo->DeclaringType->FullName, ex->Message );
					}
				}
			}

			LogInfo( "{0}: {1} function(s) loaded.", Path::GetFileName( assemblyInfo->Path ), count );
		}

		CreateUserErrorMessageTable( errorMessages->ToArray() );
	}
	catch ( System::Exception^ ex )
	{
		LogError( ex->ToString() );
		return false;
	}

	return true;
}


// Load user libraries.
bool Manager::LoadAssemblies()
{
    // Статический HashSet для быстрой проверки поддерживаемых типов.
    // Инициализируем его один раз.
    auto types = gcnew array<Type^> { String::typeid, Complex::typeid, array<Complex, 2>::typeid };

    // Инициализируем наш HashSet из этого массива
    SupportedTypes = gcnew HashSet<Type^>( types );

    Assemblies = gcnew List<AssemblyInfo^>();

	if ( !Initialize() ) return false;

	LogInfo( "Starting assembly scan for MathcadFunction types..." );

	//Двухпроходный алгоритм.

	// Шаг 1: Сканируем, чтобы посчитать функции и проверить типы.
	int totalFunctionsCount = 0;
    
    // Сохраняем найденные типы.
    List<Type^>^ foundFunctionTypes = gcnew List<Type^>();

	try
	{
		auto libs = Directory::GetFiles( AssemblyDirectory, "*.dll" );

		for each ( auto path in libs )
		{
			try
			{
				if ( !IsManagedAssembly( path ) ) continue;

                auto assembly = Assembly::LoadFile( path );

				for each ( Type ^ type in assembly->GetTypes() )
				{
					// Ищем публичные классы, унаследованные от нашего базового класса.
					if ( type->IsPublic && !type->IsAbstract && MathcadFunctionBase::typeid->IsAssignableFrom( type ) )
					{
						// Проверяем наличие атрибута.
                        // 1. Получаем все атрибуты как массив Object^.
                        array<Object^>^ attributes = type->GetCustomAttributes( ComputableAttribute::typeid, false );

                        // 2. Проверяем, что атрибут найден.
                        if ( attributes->Length == 0 )
                        {
                            LogInfo( "Class {0} inherits from MathcadFunctionBase but is missing a [Computable] attribute. Skipping.", type->FullName );
                            continue;
                        }

						// Сохраняем тип для второго прохода.
						foundFunctionTypes->Add( type );
						totalFunctionsCount++;
					}
				}
			}
			catch ( Exception^ ex ) { LogError( "Could not scan assembly '{0}': {1}", Path::GetFileName( path ), ex->Message ); }
		}
	}
	catch ( Exception^ ex ) { LogError( "Failed during assembly scanning phase: {0}", ex->ToString() ); return false; }

	if ( totalFunctionsCount == 0 )
	{
		LogInfo( "No functions found." );
		return true;
	}

	// Шаг 2: Выделяем память для трамплинов.
	const size_t TRAMPOLINE_SIZE = 40;
	size_t requiredSize = totalFunctionsCount * TRAMPOLINE_SIZE;

	MathcadEfi.DynamicCode = ( PBYTE )::VirtualAllocEx( ::GetCurrentProcess(), 0, requiredSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );

	if ( MathcadEfi.DynamicCode == nullptr )
	{
		LogError( "Dynamic memory allocation failed for {0} bytes.", requiredSize );
		return false;
	}

	LogInfo( "Allocated {0} bytes for {1} function trampolines.", requiredSize, totalFunctionsCount );

	// Шаг 3: Создаем экземпляры и собираем FunctionInfo.
	try
	{
		Dictionary<Assembly^, AssemblyInfo^>^ assemblyMap = gcnew Dictionary<Assembly^, AssemblyInfo^>();

		for each ( Type ^ funcType in foundFunctionTypes )
		{
			// НАЧАЛО МАГИИ РЕФЛЕКСИИ.

            // 1. Получаем атрибут правильным способом.
            array<Object^>^ attributes = funcType->GetCustomAttributes( ComputableAttribute::typeid, false );

            // Мы уже знаем, что он есть, поэтому просто берем первый.
            auto attr = safe_cast< ComputableAttribute^ >( attributes[0] );

            // 2. Ищем базовый generic-тип (MathcadFunction<...>)
            Type^ baseType = funcType->BaseType;

            while ( baseType != nullptr && !baseType->IsGenericType )
            {
                baseType = baseType->BaseType;
            }

            if ( baseType == nullptr )
            {
                LogError( "Could not find a generic base for function type {0}. Skipping.", funcType->FullName );
                continue;
            }

			// 3. Извлекаем generic-аргументы (TArg1, TArg2, ..., TResult).
			array<Type^>^ genericArgs = baseType->GetGenericArguments();

			if ( genericArgs->Length == 0 )
			{
				LogError( "Generic base for {0} has no generic arguments. Skipping.", funcType->FullName );
				continue;
			}

			// 4. Определяем типы аргументов и возвращаемого значения.
			Type^ returnType = genericArgs[genericArgs->Length - 1];
			array<Type^>^ argTypes = gcnew array<Type^>( genericArgs->Length - 1 );
			Array::Copy( genericArgs, argTypes, argTypes->Length );

			// 5. Проверяем, что все типы поддерживаются Mathcad.
			bool typesAreValid = SupportedTypes->Contains( returnType );

			for each ( Type ^ argType in argTypes )
			{
				if ( !SupportedTypes->Contains( argType ) ) typesAreValid = false;
			}
			if ( !typesAreValid )
			{
				LogError( "Function {0} ('{1}') uses unsupported argument or return types. Skipping.", funcType->FullName, attr->Name );
				continue;
			}

			// 6. Собираем FunctionInfo.
			auto functionInfo = gcnew FunctionInfo( attr->Name, attr->Parameters, attr->Description, returnType, argTypes );

			// 7. Создаем экземпляр функции.
			auto instance = ( MathcadFunctionBase^ ) Activator::CreateInstance( funcType );

			// 8. Передаем в него собранный FunctionInfo
			// Мы используем явную реализацию интерфейса, чтобы "записать" в свойство.
			( ( IComputable^ ) instance )->Info = functionInfo;

			// КОНЕЦ МАГИИ РЕФЛЕКСИИ.

			// Добавляем экземпляр в правильный AssemblyInfo.
			AssemblyInfo^ assemblyInfo;

			if ( !assemblyMap->TryGetValue( funcType->Assembly, assemblyInfo ) )
			{
				assemblyInfo = gcnew AssemblyInfo( funcType->Assembly->Location );

				assemblyMap->Add( funcType->Assembly, assemblyInfo );
				Assemblies->Add( assemblyInfo );
			}

			assemblyInfo->Functions->Add( instance );
		}
	}
	catch ( Exception^ ex )
	{
		LogError( "Failed during function instantiation phase: {0}", ex->ToString() );
		::VirtualFreeEx( ::GetCurrentProcess(), MathcadEfi.DynamicCode, 0, MEM_RELEASE );
		MathcadEfi.DynamicCode = nullptr;
		return false;
	}

	return true;
}
