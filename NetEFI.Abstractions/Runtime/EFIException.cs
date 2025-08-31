using System;

namespace NetEFI.Runtime
{
    public class EFIException: Exception
    {
        public int ErrNum { get; internal set; }
        public int ArgNum { get; }
        public string ErrorKey { get; }

        // Keep the old constructor for internal use by the host if needed
        internal EFIException( int errNum, int argNum )
        {
            ErrNum = errNum;
            ArgNum = argNum;
        }

        /// <summary>
        /// Creates an EFIException using a unique error key.
        /// The host will resolve this key to the correct error index at runtime.
        /// </summary>
        public EFIException( string errorKey, int argNum )
        {
            if ( string.IsNullOrWhiteSpace( errorKey ) )
                throw new ArgumentException( "Error key cannot be null or empty.", nameof( errorKey ) );

            ErrorKey = errorKey;
            ArgNum = argNum;
            ErrNum = -1; // Indicates that the index is not yet resolved
        }
    }
}