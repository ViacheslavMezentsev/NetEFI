#include "stdafx.h"
#include "Manager.h"

using namespace NetEFI::Runtime;
using namespace NetEFI::Functions;

namespace NetEFI
{
    // The main class for the built-in 'netefi' function.
    [Computable( "netefi", "cmd", "NetEFI host command interface. Returns help string by default." )]
    public ref class netefi : public CustomFunction<String^, String^>
    {
    private:
        /// <summary>
        /// A helper method to generate a CustFunc-compatible XML file for a given assembly.
        /// </summary>
        String^ GenerateXmlForAssembly( String^ assemblyName, Context^ context )
        {
            int functionCount = 0;

            try
            {
                String^ assemblyPath = Path::Combine( Manager::AssemblyDirectory, assemblyName + ".dll" );

                if ( !File::Exists( assemblyPath ) )
                {
                    return String::Format( "ERROR: Assembly '{0}' not found in the Custom Functions directory.", assemblyName );
                }

                // We will use Reflection to find all valid function types.
                auto assembly = Assembly::LoadFrom( assemblyPath );

                auto functionTypes = gcnew List<Type^>();

                // Iterate through all types in the loaded assembly.
                for each ( Type ^ type in assembly->GetTypes() )
                {
                    // The check should be against the non-generic base class 'CustomFunctionBase'.
                    // 'IsAssignableFrom' correctly checks for inheritance.
                    if ( type->IsPublic && !type->IsAbstract && CustomFunctionBase::typeid->IsAssignableFrom( type ) )
                    {
                        // The check for the attribute remains the same and is correct.
                        if ( type->GetCustomAttributes( ComputableAttribute::typeid, false )->Length > 0 )
                        {
                            functionTypes->Add( type );
                        }
                    }
                }

                if ( functionTypes->Count == 0 )
                {
                    return String::Format( "No [Computable] functions found in '{0}'.", assemblyName );
                }

                auto xdoc = gcnew XDocument(
                    gcnew XDeclaration( "1.0", "UTF-8", "yes" ),
                    gcnew XElement( "FUNCTIONS" )
                );

                String^ category;

                array<Object^>^ assemblyAttributes = assembly->GetCustomAttributes( AssemblyTitleAttribute::typeid, false );

                if ( assemblyAttributes->Length > 0 )
                {
                    auto titleAttribute = safe_cast< AssemblyTitleAttribute^ >( assemblyAttributes[0] );

                    if ( !String::IsNullOrWhiteSpace( titleAttribute->Title ) )
                    {
                        category = titleAttribute->Title;
                    }
                }

                if ( String::IsNullOrWhiteSpace( category ) )
                {
                    category = assemblyName;
                }

                for each ( Type ^ type in functionTypes )
                {
                    auto attr = safe_cast< ComputableAttribute^ >( type->GetCustomAttributes( ComputableAttribute::typeid, false )[0] );
                    xdoc->Root->Add(
                        gcnew XElement( "function",
                            gcnew XElement( "name", attr->Name ),
                            gcnew XElement( "local_name", attr->Name ),
                            gcnew XElement( "params", attr->Parameters ),
                            gcnew XElement( "category", category ),
                            gcnew XElement( "description", attr->Description )
                        )
                    );

                    functionCount++;
                }

                String^ appDataDocsPath = Manager::MathcadAppData;
                String^ docsPath = Path::Combine( appDataDocsPath, "docs" );
                Directory::CreateDirectory( docsPath );

                String^ xmlPath = Path::Combine( docsPath, assemblyName + ".xml" );

                xdoc->Save( xmlPath );

                return String::Format( "Successfully generated XML file for {0} functions. Please copy '{1}' to your Mathcad installation's 'Custom Functions\\docs' directory to use it with CustFunc.",
                    functionCount, xmlPath );
            }
            catch ( Exception^ ex )
            {
                context->LogError( "XML Generation failed: {0}", ex->ToString() );
                return String::Format( "ERROR: {0}", ex->Message );
            }
        }

    public:
        /// <summary>
        /// The main execution logic for the 'netefi' function.
        /// </summary>
        virtual String^ Execute( String^ cmd, Context^ context ) override
        {
            if ( cmd == "info" )
            {
                auto aname = Assembly::GetExecutingAssembly()->GetName();

                auto version = aname->Version;

                auto bdate = ( gcnew DateTime( 2000, 1, 1 ) )->AddDays( version->Build ).AddSeconds( 2 * version->Revision );

                return String::Format( "{0}: {1}-bit, {2}, {3:dd-MMM-yyyy HH:mm:ss}", aname->Name, 
                    ( Environment::Is64BitProcess ? "64" : "32" ), version, bdate );
            }

            else if ( cmd == "os" ) return Environment::OSVersion->ToString();

            else if ( cmd == "net" ) return Environment::Version->ToString();

            else if ( cmd == "author" ) return "Viacheslav N. Mezentsev";

            else if ( cmd == "email" ) return "viacheslavmezentsev@ya.ru";

            else if ( cmd == "list" )
            {
                auto list = gcnew List<String^>();

                for each ( auto info in Manager::Assemblies )
                {
                    if ( info->Path->Contains( Manager::AssemblyFileName ) ) continue;

                    list->Add( Path::GetFileNameWithoutExtension( info->Path ) );
                }

                return String::Join( ", ", list->ToArray() );
            }


            // --- Debugging commands ---
/*
            if ( cmd->Equals( "debug:on", StringComparison::OrdinalIgnoreCase ) )
            {
                Manager::LogExecutionEnabled = true;
                Manager::LogInfo( "Execution logging has been ENABLED by user command." );
                return "Execution logging ENABLED.";
            }

            if ( cmd->Equals( "debug:off", StringComparison::OrdinalIgnoreCase ) )
            {
                Manager::LogExecutionEnabled = false;
                Manager::LogInfo( "Execution logging has been DISABLED by user command." );
                return "Execution logging DISABLED.";
            }
*/
            // --- XML Generation command ---

            if ( cmd->StartsWith( "generate_xml,", StringComparison::OrdinalIgnoreCase ) )
            {
                // Split only on the first comma
                auto parts = cmd->Split( gcnew array<Char>{','}, 2 );

                if ( parts->Length < 2 || String::IsNullOrWhiteSpace( parts[1] ) )
                {
                    return "ERROR: Assembly name not provided. Usage: generate_xml,YourLibraryName";
                }

                String^ assemblyName = parts[1]->Trim();

                return GenerateXmlForAssembly( assemblyName, context );
            }

            // Default response if no command matched
            return "help: info, os, net, author, email, list, 'generate_xml,AssemblyName'";
        }
    };
}