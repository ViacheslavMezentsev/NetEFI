using System;
using System.Linq;
using System.Reflection;
using System.Collections.Generic;

// Import the new NetEFI namespaces
using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Functions;

namespace cstools
{
    // Describe the function using the Computable attribute.
    [Computable( "cstools", "cmd", "A utility function to inspect .NET assemblies." )]
    // Inherit from the strongly-typed base class.
    public class CsTools: MathcadFunction<string, string>
    {
        // Implement the simple, strongly-typed Execute method.
        public override string Execute( string cmd, Context context )
        {
            var assembly = this.GetType().Assembly;

            try
            {
                if ( cmd.Equals( "info", StringComparison.OrdinalIgnoreCase ) )
                {
                    var name = assembly.GetName();
                    return $"{name.Name}: {name.Version}";
                }

                if ( cmd.Equals( "list", StringComparison.OrdinalIgnoreCase ) )
                {
                    // Find all function types in this assembly that use the new architecture.
                    var functionTypes = assembly.GetTypes().Where( t =>
                        t.IsPublic && !t.IsAbstract && typeof( MathcadFunctionBase ).IsAssignableFrom( t ) );

                    var names = new List<string>();

                    foreach ( var type in functionTypes )
                    {
                        // Get the function name from its attribute.
                        var attr = type.GetCustomAttribute<ComputableAttribute>( false );

                        if ( attr != null )
                        {
                            names.Add( attr.Name );
                        }
                    }

                    return string.Join( ", ", names );
                }
            }
            catch ( Exception ex )
            {
                context.LogError( $"cstools failed: {ex.Message}" );
                return $"ERROR: {ex.Message}";
            }

            return "help: info, list";
        }
    }
}