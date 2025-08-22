
# NetEFI Framework for Mathcad

[![NuGet](https://img.shields.io/nuget/v/NetEFI.Framework?label=NuGet)](https://www.nuget.org/packages/NetEFI.Framework/)
[![Nuget](https://img.shields.io/nuget/dt/NetEFI.Framework)](https://www.nuget.org/packages/NetEFI.Framework/)
[![GitHub](https://img.shields.io/github/license/ViacheslavMezentsev/NetEFI)](https://github.com/ViacheslavMezentsev/NetEFI/blob/master/LICENSE)

This framework allows you to create user-defined functions for **Mathcad 15** and **Mathcad Prime** using any .NET language (C#, VB.NET, etc.). It removes the need to write C++ code, allowing you to focus on your function's logic in a familiar managed environment.

## Key Features

* **Write in .NET**: Develop your custom functions in C#, VB.NET, or any other .NET language.
* **Full Compatibility**: Supports both the classic Mathcad 15 and modern versions of Mathcad Prime (via the legacy Custom Functions API).
* **Rich Data Types**: Natively handles complex scalars (`Complex`), complex matrices (`Complex[,]`), and strings (`string`).
* **Easy Setup**: The NuGet package automatically includes the required C++/CLI host (`netefi.dll`), so you only need to focus on your C# code.
* **Advanced Features**: Provides an API for error handling, logging, and checking for user interruption (Esc key).

## Quick Start

1. Create a new **Class Library** project targeting **.NET Framework 4.8**.
2. Install this NuGet package (`NetEFI.Framework`).
3. Create a public class that implements the `NetEFI.Computables.IComputable` interface.
4. Implement the `Info` property and the `NumericEvaluation` method.
5. **Build** your project (for x86 or x64, depending on your Mathcad version).
6. **Deploy** the files: Copy the following three files from your project's output directory (e.g., `bin\x64\Debug`) to your Mathcad installation folder:
    * `YourLibrary.dll` (your compiled project)
    * `NetEFI.Abstractions.dll` (the API library from the NuGet package)
    * `netefi.dll` (the C++/CLI host from the NuGet package)
    * **Target Folder for Mathcad Prime**: `%ProgramFiles%\PTC\Mathcad Prime X.X.X.X\Custom Functions`
    * **Target Folder for Mathcad 15**: `%ProgramFiles%\Mathcad\Mathcad 15\userefi`

---

## The `IComputable` Interface

This is the core contract for any function you create.

* `FunctionInfo Info { get; }`
    A property that must return a `FunctionInfo` object. This object tells Mathcad the name of your function, its parameters, description, and the data types it accepts and returns.

* `FunctionInfo GetFunctionInfo(string lang)`
    An optional method to provide localized (translated) function information. You can simply have it return the default `Info` property.

* `bool NumericEvaluation(object[] args, out object result, Context context)`
    The main engine of your function. This method is called by Mathcad with the user's input.
  * `args`: An array of `object` containing the input arguments. You must cast them to their expected types.
  * `result`: An `out` parameter where you must place the return value.
  * `context`: Provides access to advanced features like logging.

### Example (C#)

```csharp
using NetEFI.Computables;
using NetEFI.Design;
using System.Numerics;

public class MyFunctions : IComputable
{
    public FunctionInfo Info => new FunctionInfo(
        "my.sum",                                     // Function Name
        "a, b",                                       // Parameters hint
        "Calculates the sum of two complex numbers",  // Description
        typeof(Complex),                              // Return Type
        typeof(Complex), typeof(Complex)              // Argument Types
    );

    public FunctionInfo GetFunctionInfo(string lang) => Info;

    public bool NumericEvaluation(object[] args, out object result, Context context)
    {
        // Cast arguments from the object array
        var a = (Complex)args[0];
        var b = (Complex)args[1];

        // Perform calculation and assign to the result parameter
        result = a + b;

        // Return true on success
        return true;
    }
}
```

---

## Advanced Topics

### Custom Error Handling

To provide meaningful error messages in Mathcad, you can throw an `EFIException`.

1. Define a static string array named `Errors` in your class. The index of the error message corresponds to the `ErrNum`.
2. In your `NumericEvaluation` method, throw a `NetEFI.Runtime.EFIException(errNum, argNum)` when an error occurs.
    * `errNum`: The 1-based index of the error message in your `Errors` array.
    * `argNum`: The 1-based index of the function argument that caused the error (or 0 if it's a general error).

```csharp
using NetEFI.Runtime;

public class MatrixFunctions : IComputable
{
    public static string[] Errors = { "Matrix must be square." };

    public FunctionInfo Info => new FunctionInfo("my.determinant", "M", "Calculates the determinant",
        typeof(Complex), typeof(Complex[,]));

    // ... GetFunctionInfo ...

    public bool NumericEvaluation(object[] args, out object result, Context context)
    {
        var matrix = (Complex[,])args[0];
        result = null;

        if (matrix.GetLength(0) != matrix.GetLength(1))
        {
            // Throw error #1, caused by argument #1
            throw new EFIException(1, 1);
        }

        // ... calculation logic ...
        return true;
    }
}
```

### Using the `Context` Object

The `context` parameter passed to `NumericEvaluation` allows you to interact with the host environment.

* `context.LogInfo("message")` / `context.LogError("message")`:
    Writes a message to the `netefi.log` file located in Mathcad's application data folder (e.g., `%APPDATA%\Mathsoft\Mathcad`). This is extremely useful for debugging.

* `context.IsUserInterrupted`:
    Returns `true` if the user has pressed the **Esc** key. You should check this property inside long-running loops to allow your function to be cancelled.
