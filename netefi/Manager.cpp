#include "stdafx.h"
#include "Manager.h"

using namespace NetEFI;

extern LRESULT CallbackFunction( void * out, ... );

#define MAX_FUNCTIONS_COUNT     10000UL
#define DYNAMIC_BLOCK_SIZE      40U

CMathcadEfi MathcadEfi;

CMathcadEfi::CMathcadEfi()
{
    AssemblyId = -1;
    FunctionId = -1;
    DynamicCode = nullptr;

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
    catch ( ... ) {}
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

    auto fileInfo = FileVersionInfo::GetVersionInfo( process->MainModule->FileName );

    LogInfo( "{0} version {1}, {2}", fileInfo->ProductName, fileInfo->ProductVersion, fileInfo->LegalCopyright );

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


PVOID Manager::CreateUserFunction( FunctionInfo ^ info, PVOID p )
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
    auto errorMessages = gcnew List<String ^>();

    try
    {
        // Register all functions in Mathcad.        
        for ( int k = 0; k < Assemblies->Count; k++ )
        {
            auto assemblyInfo = Assemblies[k];

            int count = 0;

            for ( int n = 0; n < assemblyInfo->Functions->Count; n++ )
            {
                if ( totalCount >= MAX_FUNCTIONS_COUNT ) break;

                auto lang = CultureInfo::CurrentCulture->ThreeLetterISOLanguageName;

                auto funcobj = assemblyInfo->Functions[n];

                auto info = ( ( IFunction ^ ) funcobj )->GetFunctionInfo( lang );

                if ( CreateUserFunction( info, pCode ) == nullptr ) continue;

                InjectCode( pCode, k, n );

                auto params = gcnew List< String ^ >();

                for each ( auto type in info->ArgTypes ) params->Add( type->ToString() );

                auto text = ( info->Parameters->Length > 0 ) ? info->Parameters : String::Join( ",", params->ToArray() );

                text = String::Format( "[ {0} ] {1}", text, info->Description );

                LogInfo( "{0} - {1}", info->Name, text );

                count++;
                totalCount++;                

                auto errorsFieldInfo = funcobj->GetType()->GetField( "Errors", BindingFlags::GetField | BindingFlags::Static | BindingFlags::Public );

                if ( errorsFieldInfo == nullptr ) continue;

                try
                {
                    auto errors = ( array < String ^ > ^ ) errorsFieldInfo->GetValue( nullptr );

                    if ( errors != nullptr ) errorMessages->AddRange( errors );
                }
                catch (...) {}
            }

            LogInfo( "{0}: {1} function(s) loaded.", Path::GetFileName( assemblyInfo->Path ), count );
        }

        // Note. The only one error table supported.
        CreateUserErrorMessageTable( errorMessages->ToArray() );
    }
    catch ( System::Exception ^ ex )
    {
        LogError( ex->Message );

        return false;
    }

    return true;
}


// Load user libraries.
bool Manager::LoadAssemblies()
{
    Assemblies = gcnew List < AssemblyInfo ^ >();

    if ( !Initialize() ) return false;

    try
    {
        // Get all assemblies.
        auto libs = Directory::GetFiles( AssemblyDirectory, "*.dll" );

        // Find all types with IFunction interface.
        for each ( auto path in libs )
        {
            try
            {
                if ( !IsManagedAssembly( path ) ) continue;

                // LoadFile vs. LoadFrom
                // http://blogs.msdn.com/b/suzcook/archive/2003/09/19/loadfile-vs-loadfrom.aspx
                auto assembly = Assembly::LoadFile( path );

                auto assemblyInfo = gcnew AssemblyInfo( path );

                // Assembly and GetType().
                // http://www.rsdn.ru/forum/dotnet/3154438?tree=tree
                // http://www.codeproject.com/KB/cs/pluginsincsharp.aspx
                for each ( Type ^ type in assembly->GetTypes() )
                {
                    if ( !type->IsPublic || !type->IsClass || !IFunction::typeid->IsAssignableFrom( type ) ) continue;

                    assemblyInfo->Functions->Add( Activator::CreateInstance( type ) );
                }

                if ( assemblyInfo->Functions->Count > 0 ) Assemblies->Add( assemblyInfo );
            }
            catch ( Exception ^ ex )
            {
                LogError( ex->Message );
                continue;
            }
        }

        // TODO: Use calculated number of functions instead of MAX_FUNCTIONS_COUNT.
        // Расчёт необходимого размера динамической памяти в зависимости от
        // максимального числа поддерживаемых функций.
        size_t size = MAX_FUNCTIONS_COUNT * DYNAMIC_BLOCK_SIZE;

        MathcadEfi.DynamicCode = ( PBYTE ) ::VirtualAllocEx( ::GetCurrentProcess(), 0, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );

        // TODO: Handle errors.
        if ( MathcadEfi.DynamicCode == nullptr )
        {
            LogError( "Dynamic memory allocation failed." );

            return false;
        }
    }
    catch ( System::Exception ^ ex )
    {
        LogError( ex->Message );

        return false;
    }

    return true;
}
