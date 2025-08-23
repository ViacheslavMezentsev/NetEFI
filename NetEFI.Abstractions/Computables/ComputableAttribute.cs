using System;

namespace NetEFI.Computables
{
    /// <summary>
    /// Describes a user-defined function for Mathcad.
    /// Place this attribute on any class that inherits from MathcadFunction.
    /// </summary>
    [AttributeUsage( AttributeTargets.Class, Inherited = false, AllowMultiple = false )]
    public sealed class ComputableAttribute: Attribute
    {
        /// <summary>
        /// The name of the function as it will be called in Mathcad (e.g., "my.func").
        /// </summary>
        public string Name { get; }

        /// <summary>
        /// A brief description of the function's purpose.
        /// </summary>
        public string Description { get; }

        /// <summary>
        /// A comma-separated string of parameter names (e.g., "filePath, encoding").
        /// </summary>
        public string Parameters { get; }

        public ComputableAttribute( string name, string parameters, string description )
        {
            Name = name;
            Parameters = parameters;
            Description = description;
        }
    }
}