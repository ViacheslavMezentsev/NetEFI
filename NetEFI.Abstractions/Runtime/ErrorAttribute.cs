using System;

namespace NetEFI.Runtime
{
    /// <summary>
    /// Describes a single custom error message for a Mathcad function, identified by a unique key.
    /// The order of attributes determines the final error index.
    /// </summary>
    [AttributeUsage( AttributeTargets.Class, Inherited = false, AllowMultiple = true )]
    public sealed class ErrorAttribute: Attribute
    {
        /// <summary>
        /// A short, unique identifier for this error (e.g., "NegativeInput"). Used to throw the error.
        /// </summary>
        public string Key { get; }

        /// <summary>
        /// The error message text that will be displayed in Mathcad.
        /// </summary>
        public string Message { get; }

        public ErrorAttribute( string key, string message )
        {
            // We can add validation here to ensure the key is a valid C# identifier, for example.
            if ( string.IsNullOrWhiteSpace( key ) )
                throw new ArgumentException( "Error key cannot be null or empty.", nameof( key ) );

            Key = key;
            Message = message;
        }
    }
}
