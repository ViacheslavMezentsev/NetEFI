using System;
using System.Collections.Generic;
using System.Reflection;

using NetEFI;

public class cstest: IFunction
{
    public FunctionInfo Info => new FunctionInfo( "cstest", "cmd", "return info",
        typeof( string ), new[] { typeof( string ) } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        try
        {
            var cmd = ( string ) args[0];

            result = "help: info, list";

            if ( cmd.Equals( "info" ) )
            {
                var name = Assembly.GetExecutingAssembly().GetName();

                result = $"{name.Name} {name.Version}";
            }
            else if ( cmd.Equals( "list" ) )
            {
                var list = new List<string>();

                var types = Assembly.GetExecutingAssembly().GetTypes();

                foreach ( var type in types )
                {
                    if ( !type.IsPublic || type.IsAbstract || !typeof(IFunction).IsAssignableFrom( type ) ) continue;

                    var f = ( IFunction ) Activator.CreateInstance( type );

                    list.Add( f.Info.Name );
                }

                result = string.Join(", ", list.ToArray());
            }
        }
        catch
        {
            result = null;
            return false;
        }

        return true;
    }
}
