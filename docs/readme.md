# NetEFI Framework for Mathcad

[![NuGet](https://img.shields.io/nuget/v/NetEFI.Framework?label=NuGet)](https://www.nuget.org/packages/NetEFI.Framework/)
[![Nuget](https://img.shields.io/nuget/dt/NetEFI.Framework)](https://www.nuget.org/packages/NetEFI.Framework/)
[![GitHub](https://img.shields.io/github/license/ViacheslavMezentsev/NetEFI)](https://github.com/ViacheslavMezentsev/NetEFI/blob/master/LICENSE)

This framework allows you to create user-defined functions for **Mathcad 15** and **Mathcad Prime** using any .NET language (C#, VB.NET, F#, C++/CLI). It removes the need to write C++ code by providing a simple, attribute-based API, allowing you to focus on your function's logic in a familiar, strongly-typed managed environment.

## Key Features

* **Write in .NET**: Develop your custom functions in C#, VB.NET, F#, or C++/CLI.
* **Simple & Elegant API**: Define functions by inheriting from a generic base class (`CustomFunction<...>`) and describing them with a `[Computable]` attribute. No more boilerplate!
* **Full Compatibility**: Supports both the classic Mathcad 15 and modern versions of Mathcad Prime (via the legacy Custom Functions API).
* **Rich Data Types**: Natively handles complex scalars (`Complex`), complex matrices (`Complex[,]`), and strings (`string`).
* **Easy Setup**: The NuGet package automatically includes the required C++/CLI host (`NetEFI.dll`).
* **Advanced Features**: Provides an API for custom error handling, logging, inter-function calls, and checking for user interruption (Esc key).
* **CustFunc Integration**: Includes a tool to automatically generate XML definition files for use with the popular `CustFunc` add-in.

---

## Quick Start Guide (v0.5+)

1. Create a new **Class Library** project targeting **.NET Framework 4.8**.
2. Install the `NetEFI.Framework` NuGet package.
3. Create a public class that:
    a. Inherits from `NetEFI.Functions.CustomFunction<...>` with your desired input and output types.
    b. Is decorated with the `[Computable]` attribute.
4. Implement the abstract `Execute(...)` method with your function's logic.
5. **Build** your project (for `x86` or `x64`, depending on your Mathcad version).
6. **Deploy** the required files from your output directory (e.g., `bin\x64\Debug`) to your Mathcad installation folder:
    * `YourLibrary.dll` (your compiled project)
    * `NetEFI.Abstractions.dll` (the API library)
    * `netefi.dll` (the C++/CLI host)
    * *Any other dependencies, like `Newtonsoft.Json.dll` or `FSharp.Core.dll`.*
    * **Target Folder for Mathcad Prime**: `%ProgramFiles%\PTC\Mathcad Prime X.X.X.X\Custom Functions`
    * **Target Folder for Mathcad 15**: `%ProgramFiles%\Mathcad\Mathcad 15\userefi`

---

## Examples

### Example (C#)

```csharp
using System.Numerics;
using NetEFI.Runtime;
using NetEFI.Functions;

[Computable("cs_sum", "a, b", "Calculates the sum of two complex numbers")]
public class MySumFunction : CustomFunction<Complex, Complex, Complex>
{
    public override Complex Execute(Complex a, Complex b, Context context)
    {
        return a + b;
    }
}
```

### Example (VB.NET)

```vb
Imports System.Numerics
Imports NetEFI.Runtime
Imports NetEFI.Functions

<Computable("vb_product", "a, b", "Calculates the product of two complex numbers")>
Public Class MyProductFunction
    Inherits CustomFunction(Of Complex, Complex, Complex)

    Public Overrides Function Execute(a As Complex, b As Complex, context As Context) As Complex
        Return a * b
    End Function
End Class
```

### Example (C++/CLI)

```cpp
#pragma once
#include "stdafx.h" // Includes all necessary NetEFI namespaces

[Computable("cpp_diff", "a, b", "Calculates the difference of two complex numbers")]
public ref class CppDiffFunction : public CustomFunction<Complex, Complex, Complex>
{
public:
    virtual Complex Execute(Complex a, Complex b, Context^ context) override
    {
        return a - b;
    }
};
```

### Example (F#)

```fsharp
namespace FSharpExamples

open System.Numerics
open NetEFI.Runtime
open NetEFI.Functions

[<Computable("fs_power", "a, b", "Calculates a raised to the power of b")>]
type FsPowerFunction() =
    inherit CustomFunction<Complex, Complex, Complex>()
    override this.Execute(a, b, context) =
        Complex.Pow(a, b)
```

---

## Advanced Topics

### Custom Error Handling

Decorate your class with `[Error]` attributes and throw an `EFIException` using the error's key.

```csharp
using NetEFI.Runtime;

[Error("NegativeInput", "Input cannot be negative.")]
[Computable("my_sqrt", "x", "Calculates the square root of a non-negative number")]
public class SqrtFunction : CustomFunction<Complex, Complex>
{
    public override Complex Execute(Complex x, Context context)
    {
        if (x.Real < 0 && x.Imaginary == 0)
        {
            // Throw using the descriptive key. No more magic numbers!
            throw new EFIException("NegativeInput", 1);
        }
        return Complex.Sqrt(x);
    }
}
```

You can also throw a standard `System.Exception` with a message that exactly matches an error key. The framework will automatically convert it: `throw new Exception("NegativeInput");`

### Using the `Context` Object

* `context.LogInfo("message")` / `context.LogError("message")`: Writes to the `netefi.log` file.
* `context.IsUserInterrupted`: Returns `true` if the user has pressed the **Esc** key.
* `context.Invoke<TResult>("functionName", args...)`: Invokes another registered NetEFI function in a type-safe way.

### Integration with CustFunc Add-in

NetEFI can automatically generate XML definition files compatible with the popular `CustFunc` add-in (which provides an F3 dialog for custom functions).

1. Ensure `CustFunc.dll` is installed in Mathcad.
2. Deploy your custom function library (e.g., `MyLibrary.dll`).
3. In a Mathcad worksheet, execute the command:
    `netefi("generate_xml,MyLibrary")`
4. This will create `MyLibrary.xml` in your `%APPDATA%\Mathsoft\Mathcad\docs` folder.
5. Copy this XML file to your Mathcad installation's `Custom Functions\docs` directory.
6. Restart Mathcad. Your functions will now appear in the `<F3>` dialog!

---

## Limitations of the Mathcad API

The NetEFI framework is powerful, but it is built upon the original Mathcad C++ API, which has some fundamental limitations:

* **Maximum 10 Arguments:** A custom function cannot have more than 10 input parameters.
* **Strict Matrix Types:** Functions can only accept or return matrices of type `Complex[,]`. Matrices of `string` or `double` are not supported, nor are jagged arrays (`Complex[][]`).
* **No Nested Matrices:** The API does not support nested matrices (matrices where elements are themselves matrices).
* **Static Error Messages:** The text for custom error messages is defined at compile time via `[Error]` attributes and cannot be changed dynamically by the function's code during execution.
* **String Encoding (ANSI only):** Mathcad's legacy API expects all strings to be in a single-byte **ANSI** encoding. The NetEFI host automatically handles this conversion, but any Unicode character that does not have a single-byte ANSI representation will be corrupted. Use of complex Unicode characters (e.g., emoji 😊) is not supported.
* **F# Deployment:** When deploying a library written in F#, you must also copy the F# runtime library, **`FSharp.Core.dll`**, to the `Custom Functions` directory.
