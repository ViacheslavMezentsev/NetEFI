using System;

namespace NetEFI.Computables
{
    /// <summary>
    /// An attribute to mark classes that implement user-defined functions.
    /// This can be used in the future for metadata-driven features like reflection-based registration.
    /// </summary>
    [AttributeUsage( AttributeTargets.Class, Inherited = false, AllowMultiple = false )]
    public sealed class ComputableAttribute: Attribute
    {
        /// <summary>
        /// A string describing the function's signature (e.g., "arg1, arg2").
        /// </summary>
        public string Parameters { get; }

        /// <summary>
        /// A brief description of what the function does.
        /// </summary>
        public string Description { get; }

        /// <summary>
        /// Initializes a new instance of the <see cref="ComputableAttribute"/> class.
        /// </summary>
        /// <param name="description">A brief description of the function.</param>
        /// <param name="parameters">A string describing the function parameters.</param>
        public ComputableAttribute( string description, string parameters = "" )
        {
            Description = description;
            Parameters = parameters;
        }
    }
}