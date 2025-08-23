using System.IO;
using System;
using NetEFI.Computables;
using NetEFI.Design;
using NetEFI.Functions;

namespace cstest
{
    [Computable( "csrfile", "filePath", "Returns the content of a specified text file." )]
    public class CsReadFile: MathcadFunction<string, string>
    {
        public override string Execute( string filePath, Context context )
        {
            try
            {
                if ( File.Exists( filePath ) )
                {
                    return File.ReadAllText( filePath );
                }
                // Return a clear error message if the file doesn't exist.
                return $"ERROR: File not found at '{filePath}'";
            }
            catch ( Exception ex )
            {
                context.LogError( $"Failed to read file '{filePath}': {ex.Message}" );
                return $"ERROR: {ex.Message}";
            }
        }
    }
}