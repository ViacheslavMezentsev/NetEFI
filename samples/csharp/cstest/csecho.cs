using NetEFI.Functions;
using NetEFI.Runtime;

namespace cstest
{
    [Computable( "csecho", "s", "Returns the input string." )]
    public class CsEcho: CustomFunction<string, string>
    {
        public override string Execute( string s, Context context )
        {
            // This demonstrates how to call another function registered with NetEFI.
            if ( context.IsDefined( "vbecho" ) )
            {
                // Use the strongly-typed Invoke method.
                context.LogInfo( "csecho is forwarding the call to vbecho." );

                return context.Invoke<string>( "vbecho", s );
            }

            // Fallback if vbecho is not available
            context.LogInfo( "vbecho not found, csecho is returning the value directly." );

            return s;
        }
    }
}