using System;

namespace NetEFI.Runtime
{
    /// <summary>
    /// A specialized exception for handling errors during function evaluation.
    /// </summary>
    public class EFIException: Exception
    {
        /// <summary>
        /// The error number, corresponding to the error table for the given function.
        /// </summary>
        public int ErrNum { get; set; }

        /// <summary>
        /// The 1-based index of the argument that caused the error.
        /// Is 0 if the error is not associated with a specific argument.
        /// </summary>
        public int ArgNum { get; set; }

        public EFIException( int errNum, int argNum )
            : base( $"Function evaluation error number {errNum} on argument {argNum}." )
        {
            ErrNum = errNum;
            ArgNum = argNum;
        }

        public EFIException( string message, int errNum, int argNum )
            : base( message )
        {
            ErrNum = errNum;
            ArgNum = argNum;
        }

        public EFIException( string message, Exception innerException, int errNum, int argNum )
            : base( message, innerException )
        {
            ErrNum = errNum;
            ArgNum = argNum;
        }
    }
}