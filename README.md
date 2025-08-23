# NetEFI Framework for Mathcad

[![NuGet](https://img.shields.io/nuget/v/NetEFI.Framework?label=NuGet)](https://www.nuget.org/packages/NetEFI.Framework/)
[![Nuget](https://img.shields.io/nuget/dt/NetEFI.Framework)](https://www.nuget.org/packages/NetEFI.Framework/)
[![GitHub](https://img.shields.io/github/license/ViacheslavMezentsev/NetEFI)](https://github.com/ViacheslavMezentsev/NetEFI/blob/master/LICENSE)

This framework allows you to create user-defined functions for **Mathcad 15** and **Mathcad Prime** using any .NET language (C#, VB.NET, F#). It removes the need to write C++ code by providing a simple, attribute-based API, allowing you to focus on your function's logic in a familiar, strongly-typed managed environment.

## Key Features

* **Write in .NET**: Develop your custom functions in C#, VB.NET, or any other .NET language.
* **Simple & Elegant API**: Define functions by inheriting from a generic base class (`MathcadFunction<..._>`) and describing them with a `[Computable]` attribute. No more boilerplate!
* **Full Compatibility**: Supports both the classic Mathcad 15 and modern versions of Mathcad Prime (via the legacy Custom Functions API).
* **Rich Data Types**: Natively handles complex scalars (`Complex`), complex matrices (`Complex[,]`), and strings (`string`).
* **Easy Setup**: The NuGet package automatically includes the required C++/CLI host (`netefi.dll`).
* **Advanced Features**: Provides an API for custom error handling, logging, and checking for user interruption (Esc key).

---

## Quick Start Guide (New Architecture v0.4+)

**For users of versions prior to 0.4, please note this is a BREAKING CHANGE. See the migration guide below.**

1. Create a new **Class Library** project targeting **.NET Framework 4.8**.
2. Install the `NetEFI.Framework` NuGet package.
3. Create a public class that:
    a. Inherits from `NetEFI.Functions.MathcadFunction<...>` with your desired input and output types.
    b. Is decorated with the `[Computable]` attribute.
4. Implement the abstract `Execute(...)` method with your function's logic.
5. **Build** your project (for `x86` or `x64`, depending on your Mathcad version).
6. **Deploy** the required files from your output directory (e.g., `bin\x64\Debug`) to your Mathcad installation folder:
    * `YourLibrary.dll` (your compiled project)
    * `NetEFI.Abstractions.dll` (the API library)
    * `netefi.dll` (the C++/CLI host)
    * *Any other dependencies, like `Newtonsoft.Json.dll`.*
    * **Target Folder for Mathcad Prime**: `%ProgramFiles%\PTC\Mathcad Prime X.X.X.X\Custom Functions`
    * **Target Folder for Mathcad 15**: `%ProgramFiles%\Mathcad\Mathcad 15\userefi`

### Example (C#)

```csharp
using System.Numerics;
using NetEFI.Computables; // For ComputableAttribute
using NetEFI.Design;      // For Context
using NetEFI.Functions;   // For MathcadFunction base class

[Computable("my.sum", "a, b", "Calculates the sum of two complex numbers")]
public class MySumFunction : MathcadFunction<Complex, Complex, Complex>
{
    public override Complex Execute(Complex a, Complex b, Context context)
    {
        context.LogInfo($"Executing my.sum with inputs {a} and {b}");
        return a + b;
    }
}
```

### Example (VB.NET)

```vb
Imports System.Numerics
Imports NetEFI.Computables
Imports NetEFI.Design
Imports NetEFI.Functions

<Computable("my.product", "a, b", "Calculates the product of two complex numbers")>
Public Class MyProductFunction
    Inherits MathcadFunction(Of Complex, Complex, Complex)

    Public Overrides Function Execute(a As Complex, b As Complex, context As Context) As Complex
        context.LogInfo($"Executing my.product with inputs {a} and {b}")
        Return a * b
    End Function
End Class
```

---

## Migration Guide for Existing Users (from v0.3 and below)

Version 0.4 introduces a new, simpler architecture. You will need to update your existing function classes.

**Old Way (implementing `IComputable`):**

```csharp
public class OldSum : IComputable
{
    public FunctionInfo Info => new FunctionInfo("my.sum", ...);
    public bool NumericEvaluation(object[] args, out object result, ...)
    {
        var a = (Complex)args;
        var b = (Complex)args;
        result = a + b;
        return true;
    }
    // ...
}
```

**New Way (inheriting `MathcadFunction`):**

1. Remove the `IComputable` interface.
2. Inherit from `MathcadFunction<...>` with your types.
3. Add the `[Computable]` attribute with your function's metadata.
4. Replace the `NumericEvaluation` method and `Info` property with a single, strongly-typed `Execute` method.

---

## Advanced Topics

### Custom Error Handling

To provide meaningful error messages in Mathcad, define a static `Errors` array and throw a `NetEFI.Runtime.EFIException` from your `Execute` method.

```csharp
using NetEFI.Runtime;

[Computable("my.sqrt", "x", "Calculates the square root of a non-negative number")]
public class SqrtFunction : MathcadFunction<Complex, Complex>
{
    public static string[] Errors = { "Input cannot be negative." };

    public override Complex Execute(Complex x, Context context)
    {
        if (x.Real < 0 && x.Imaginary == 0)
        {
            // Throw error #1, caused by argument #1
            throw new EFIException(1, 1);
        }
        return Complex.Sqrt(x);
    }
}
```

### Using the `Context` Object

The `context` parameter passed to `Execute` allows you to interact with the host environment.

* `context.LogInfo("message")` / `context.LogError("message")`: Writes a message to the `netefi.log` file.
* `context.IsUserInterrupted`: Returns `true` if the user has pressed the **Esc** key.
* `context["functionName"]`: Allows you to call other registered NetEFI functions.
