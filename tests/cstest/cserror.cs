using System.Numerics;

using NetEFI.Functions;
using NetEFI.Runtime;

namespace cstest
{
    // Describe each error with a unique key and a message using [Error] attributes.
    // The declaration order determines the final 1-based index.
    [Error( "Error1", "cserror: This is custom error message 1." )] // This will be error #1
    [Error( "Error2", "cserror: This is custom error message 2." )] // This will be error #2
    [Error( "Error3", "cserror: This is custom error message 3." )] // This will be error #3
    [Error( "Error4", "cserror: This is custom error message 4." )] // This will be error #4

    // The Computable attribute remains the same.
    [Computable( "cserror", "n", "Throws a custom error specified by its index n." )]
    public class CsError: CustomFunction<Complex, string>
    {
        // Implement the Execute method.
        public override string Execute( Complex n, Context context )
        {
            int errorIndex = ( int ) n.Real;

            // We now throw the EFIException using a KEY, not an index.
            // This is more robust, but for this example, we'll map the input index to a key.
            // In a real application, you would throw based on a condition, e.g., throw new EFIException("Error2", 1);

            string errorKey;

            switch ( errorIndex )
            {
                case 1:
                    errorKey = "Error1";
                    break;
                case 2:
                    errorKey = "Error2";
                    break;
                case 3:
                    errorKey = "Error3";
                    break;
                case 4:
                    errorKey = "Error4";
                    break;
                default:
                    // Throw a standard exception if the user provides an invalid index.
                    // The host will catch this and log it as a critical error.
                    throw new System.ArgumentOutOfRangeException(
                        nameof( n ),
                        $"The provided error index '{errorIndex}' is not valid. It must be between 1 and 4."
                    );
            }

            // Throws the custom EFIException using the resolved key.
            // The second parameter is the 1-based index of the argument causing the error.
            throw new EFIException( errorKey, 1 );
        }
    }
}