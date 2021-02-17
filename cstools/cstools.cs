using System;
using System.Linq;
using System.Reflection;

using NetEFI;

public class cstools: IFunction
{
    public FunctionInfo Info => new FunctionInfo( "cstools", "cmd", "return info",
        typeof( string ), new[] { typeof( string ) } );

    public FunctionInfo GetFunctionInfo( string lang ) => Info;

    public bool NumericEvaluation( object[] args, out object result, ref Context context )
    {
        result = "help: info, list";

        var assembly = Assembly.GetExecutingAssembly();

        try
        {
            var cmd = ( string ) args[0];

            if ( cmd == "info" )
            {
                var name = assembly.GetName();

                result = $"{name.Name}: {name.Version}";
            }

            else if ( cmd == "list" )
            {
                var types = assembly.GetTypes().Where( t => t.IsPublic && !t.IsAbstract && typeof( IFunction ).IsAssignableFrom(t) );

                var names = types.Select( t => ( ( IFunction ) Activator.CreateInstance(t) ).Info.Name ).ToArray();

                result = string.Join( ", ", names );
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
