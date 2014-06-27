using System;
using System.Collections.Generic;
using System.Reflection;

using NetEFI;


public class cstest: IFunction {

    private FunctionInfo _info;

    public FunctionInfo Info {

        get { return _info; }
    }

    public cstest() {

        _info = new FunctionInfo(

            "cstest", "cmd", "return info",            
            typeof( String ),
            new[] { typeof( String ) }
            );
    }

    public FunctionInfo GetFunctionInfo( string lang ) {

        return Info;
    }

    public bool NumericEvaluation( object[] args, out object result ) {

        try {

            var cmd = ( String ) args[0];

            result = "empty";

            if (cmd.Equals("info")) {

                result = Assembly.GetExecutingAssembly().ToString();

            } else if (cmd.Equals("list")) {

                var list = new List<string>();

                var types = Assembly.GetExecutingAssembly().GetTypes();

                foreach ( var type in types ) {

                    if ( !type.IsPublic || type.IsAbstract || !typeof(IFunction).IsAssignableFrom( type ) ) continue;

                    var f = ( IFunction ) Activator.CreateInstance( type );

                    list.Add( f.Info.Name );
                }

                result = String.Join(", ", list.ToArray());
            }

        } catch ( Exception ex ) {

            result = null;
            return false;
        }

        return true;
    }

}
