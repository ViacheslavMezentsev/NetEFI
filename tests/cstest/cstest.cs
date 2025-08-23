using System;
using System.Linq;
using System.Reflection;
using System.Collections.Generic;
using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Functions;

namespace cstest
{
    [Computable( "cstest", "cmd", "A utility function to inspect the C# test assembly." )]
    public class CsTest: MathcadFunction<string, string>
    {
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
                    var functionTypes = assembly.GetTypes().Where( t =>
                        t.IsPublic && !t.IsAbstract && typeof( MathcadFunctionBase ).IsAssignableFrom( t ) );

                    var names = functionTypes
                        .Select( t => t.GetCustomAttribute<ComputableAttribute>( false )?.Name )
                        .Where( n => n != null );

                    return string.Join( ", ", names );
                }
            }
            catch ( Exception ex )
            {
                return $"ERROR: {ex.Message}";
            }

            return "help: info, list";
        }
    }
}