#include "stdafx.h"
#include <msclr\marshal_cppstd.h>
#include "Manager.h"

using namespace msclr::interop;
using namespace NetEFI;

extern LRESULT CallbackFunction( void * out, ... );

#define MAX_FUNCTIONS_COUNT     10000UL
#define DYNAMIC_BLOCK_SIZE      40U

int AssemblyId = -1;
int FunctionId = -1;
PBYTE DynamicCode = nullptr;
CMathcadEfi MathcadEfi;

CMathcadEfi::CMathcadEfi()
{
    String ^ path = Path::Combine( Manager::AssemblyDirectory, "..\\mcaduser.dll" );

    if ( File::Exists( path ) )
    {        
        marshal_context context;

        HMODULE hLib = ::LoadLibraryW( context.marshal_as<LPCWSTR>( path ) );

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
            Manager::LogError( "[LoadLibrary] returns NULL." );
        }
    }
    else
    {
        Manager::LogError( "File not found: {0}", path );
    }

    Attached = ( CreateUserFunction != nullptr && CreateUserErrorMessageTable != nullptr
        && MathcadAllocate != nullptr && MathcadFree != nullptr
        && MathcadArrayAllocate != nullptr && MathcadArrayFree != nullptr
        && isUserInterrupted != nullptr );
}


CMathcadEfi::~CMathcadEfi()
{
    try
    {
        if ( ::DynamicCode != nullptr ) ::VirtualFreeEx( ::GetCurrentProcess(), ::DynamicCode, 0, MEM_RELEASE );

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
    catch ( ... )
    {
    }
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

        if ( name->Equals( Manager::ExecAssembly->GetName()->Name ) ) return Manager::ExecAssembly;

        name = name + ".dll";

        Manager::LogInfo( "[OnAssemblyResolve] {0}", name );             

        String ^ path = Path::Combine( Manager::AssemblyDirectory, name );

        if ( File::Exists( path ) )
        {
            retval = Assembly::LoadFile( path );

            Manager::LogInfo( "Assembly loaded: {0}", path );
        }
        else
        {
            Manager::LogInfo( "Assembly not found: {0}", path );
        }
    }
    catch ( System::Exception ^ ex )
    {
        Manager::LogError( "Assembly not loaded: {0}", ex->Message );
    }

    return retval;
}


bool Manager::Initialize()
{
    auto aname = ExecAssembly->GetName();

    auto version = aname->Version;

    auto bdate = ( gcnew DateTime( 2000, 1, 1 ) )->AddDays( version->Build ).AddSeconds( 2 * version->Revision );

    LogInfo( ".Net: {0}", Environment::Version );

#ifdef _DEBUG
    LogInfo( "{0}: {1}-bit debug version {2}, {3:dd-MMM-yyyy HH:mm:ss}", aname->Name, ( Environment::Is64BitProcess ? "64" : "32" ), version, bdate );
#else
    LogInfo( "{0}: {1}-bit release version {2}, {3:dd-MMM-yyyy HH:mm:ss}", aname->Name, ( Environment::Is64BitProcess ? "64" : "32" ), version, bdate );
#endif

    return MathcadEfi.Attached;
}


// How to determine whether a DLL is a managed assembly or native (prevent loading a native dll)?
// http://stackoverflow.com/questions/367761/how-to-determine-whether-a-dll-is-a-managed-assembly-or-native-prevent-loading
bool Manager::IsManagedAssembly( String ^ fileName )
{
    Stream ^ fileStream = gcnew FileStream( fileName, IO::FileMode::Open, IO::FileAccess::Read );

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
    int count = errors->GetLength(0);

    char ** errorMessages = new char * [ count ];

    for ( int n = 0; n < count; n++ )
    {
        String ^ text = errors[n];

        std::string s = marshal_as<std::string>( text );

        char * msgItem = MathcadEfi.MathcadAllocate( ( unsigned ) s.length() + 1 );

        errorMessages[n] = msgItem;

        ::memset( msgItem, 0, s.length() + 1 );

        // Copy string from the temporery buffer.
        ::memcpy( msgItem, s.c_str(), s.length() );
    }

    // Copy table content to the inner memory.
    if ( !MathcadEfi.CreateUserErrorMessageTable( ::GetModuleHandle( NULL ), count, errorMessages ) )
    {
        LogError( "[CreateUserErrorMessageTable] failed" );
    }

    // Free allocated memory.
    for ( int n = 0; n < count; n++ ) MathcadEfi.MathcadFree( errorMessages[n] );

    delete[] errorMessages;
}


PVOID Manager::CreateUserFunction( FunctionInfo ^ info, PVOID p )
{
    FUNCTIONINFO fi;

    marshal_context context;

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

        if ( type->Equals( String::typeid ) )
        {
            fi.returnType = STRING;
        }
        else if ( type->Equals( Complex::typeid ) )
        {
            fi.returnType = COMPLEX_SCALAR;
        }
        else if ( type->Equals( array<Complex, 2>::typeid ) )
        {
            fi.returnType = COMPLEX_ARRAY;
        }
        else
        {
            LogError( "[{0}] Unknown return type: {1}", info->Name, type->ToString() );

            return nullptr;
        }

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

            if ( type->Equals( String::typeid ) )
            {
                fi.argType[m] = STRING;
            }
            else if ( type->Equals( Complex::typeid ) )
            {
                fi.argType[m] = COMPLEX_SCALAR;
            }
            else if ( type->Equals( array<Complex, 2>::typeid ) )
            {
                fi.argType[m] = COMPLEX_ARRAY;
            }
            else
            {
                LogError( "[{0}] Unknown argument type {1}: {2}", info->Name, m, type->ToString() );

                return nullptr;
            }
        }

        return MathcadEfi.CreateUserFunction( ::GetModuleHandle( NULL ), & fi );
    }
    catch ( System::Exception ^ ex )
    {
        LogError( "[{0}] {1}", info->Name, ex->Message );

        return nullptr;
    }
}


// The magic part is here.
void Manager::InjectCode( PBYTE & p, int k, int n )
{
    // mov eax, imm32
    *p++ = 0xB8; 
    p[0] = k;
    p += sizeof( int );

    // mov [AssemblyId], eax
    *p++ = 0xA3; 
    ( int *& ) p[0] = & ::AssemblyId;
    p += sizeof( int * );

    // mov eax, imm32
    *p++ = 0xB8; 
    p[0] = n;
    p += sizeof( int );

    // mov [FunctionId], eax
    *p++ = 0xA3; 
    ( int *& ) p[0] = & ::FunctionId;
    p += sizeof( int * );

    // mov rax, CallbackFunction.
    *p++ = 0x48;
    *p++ = 0xB8;
    ( PBYTE & ) p[0] = ( PBYTE ) ::CallbackFunction;
    p += sizeof( PBYTE );

    // jmp rax.
    *p++ = 0xFF;
    *p++ = 0xE0;
}


// Register user functions.
bool Manager::RegisterFunctions()
{
    if ( Assemblies == nullptr ) return false;

    try
    {
        // Register all functions in Mathcad.        
        int totalCount = 0;
        PBYTE pCode = DynamicCode;
        auto errorMessages = gcnew List<String ^>();

        for ( int k = 0; k < Assemblies->Count; k++ )
        {
            AssemblyInfo ^ assemblyInfo = Assemblies[k];

            // Add error table.
            if ( assemblyInfo->Errors != nullptr )
            {
                errorMessages->AddRange( assemblyInfo->Errors );
            }

            int count = 0;

            for ( int n = 0; n < assemblyInfo->Functions->Count; n++ )
            {
                if ( totalCount >= MAX_FUNCTIONS_COUNT ) break;

                String ^ lang = CultureInfo::CurrentCulture->ThreeLetterISOLanguageName;

                FunctionInfo ^ info = assemblyInfo->Functions[n]->GetFunctionInfo( lang );

                if ( CreateUserFunction( info, pCode ) == nullptr ) continue;

                InjectCode( pCode, k, n );

                auto params = gcnew List< String ^ >();

                for each ( auto type in info->ArgTypes ) params->Add( type->ToString() );

                String ^ text = ( info->Parameters->Length > 0 ) ? info->Parameters : String::Join( ",", params->ToArray() );

                text = String::Format( "[ {0} ] {1}", text, info->Description );

                LogInfo( "{0} - {1}", info->Name, text );

                count++;
                totalCount++;
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
        for each ( String ^ path in libs )
        {
            try
            {
                if ( !IsManagedAssembly( path ) ) continue;

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

                    // Check if error table exists.
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

        // TODO: Use calculated number of functions instead of MAX_FUNCTIONS_COUNT.
        // Расчёт необходимого размера динамической памяти в зависимости от
        // максимального числа поддерживаемых функций.
        size_t size = MAX_FUNCTIONS_COUNT * DYNAMIC_BLOCK_SIZE;

        ::DynamicCode = ( PBYTE ) ::VirtualAllocEx( ::GetCurrentProcess(), 0, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );

        // TODO: Handle errors.
        if ( ::DynamicCode == nullptr )
        {
            LogError( "Dynamic memory allocate failed." );

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
