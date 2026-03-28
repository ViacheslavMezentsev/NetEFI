# NetEFI Framework для Mathcad

[![NuGet](https://img.shields.io/nuget/v/NetEFI.Framework?label=NuGet)](https://www.nuget.org/packages/NetEFI.Framework/)
[![Nuget](https://img.shields.io/nuget/dt/NetEFI.Framework)](https://www.nuget.org/packages/NetEFI.Framework/)
[![GitHub](https://img.shields.io/github/license/ViacheslavMezentsev/NetEFI)](https://github.com/ViacheslavMezentsev/NetEFI/blob/master/LICENSE)

Фреймворк для создания пользовательских функций **Mathcad 15** и **Mathcad Prime** на любом .NET-языке (C#, VB.NET, F#, C++/CLI). Избавляет от необходимости писать код на C++: простой API на основе атрибутов позволяет сосредоточиться на логике функции в привычной строго-типизированной управляемой среде.

---

## Возможности

- **Любой .NET-язык**: C#, VB.NET, F# или C++/CLI.
- **Простой и выразительный API**: достаточно унаследоваться от универсального базового класса `CustomFunction<...>` и добавить атрибут `[Computable]`. Никакого шаблонного кода.
- **Полная совместимость**: поддерживается как классический Mathcad 15, так и современные версии Mathcad Prime (через legacy Custom Functions API).
- **Богатые типы данных**: комплексные числа (`Complex`), комплексные матрицы (`Complex[,]`), строки (`string`).
- **Простая установка**: NuGet-пакет автоматически включает C++/CLI-хост (`netefi.dll`).
- **Расширенные возможности**: API для обработки ошибок, логирования, вызова других функций, проверки прерывания пользователем (клавиша Esc).
- **Интеграция с CustFunc**: автоматическая генерация XML-файлов определений для популярного дополнения `CustFunc`.

---

## Быстрый старт (v0.5+)

1. Создайте проект **Class Library** с целевой платформой **.NET Framework 4.8**.
2. Установите NuGet-пакет `NetEFI.Framework`.
3. Создайте публичный класс, который:
   - наследуется от `NetEFI.Functions.CustomFunction<...>` с нужными типами входных и выходных параметров;
   - помечен атрибутом `[Computable]`.
4. Реализуйте абстрактный метод `Execute(...)` с логикой функции.
5. **Соберите** проект (для `x86` или `x64` в зависимости от версии Mathcad).
6. **Разверните** файлы из выходной папки (например, `bin\x64\Debug`) в папку установки Mathcad:
   - `YourLibrary.dll` — ваша скомпилированная библиотека
   - `NetEFI.Abstractions.dll` — managed API
   - `netefi.dll` — C++/CLI-хост
   - *прочие зависимости, например `Newtonsoft.Json.dll` или `FSharp.Core.dll`*
   - **Mathcad Prime**: `%ProgramFiles%\PTC\Mathcad Prime X.X.X.X\Custom Functions\`
   - **Mathcad 15**: `%ProgramFiles(x86)%\Mathcad\Mathcad 15\userefi\`

---

## Примеры

### C#

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

### VB.NET

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

### C++/CLI

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

### F#

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

## Дополнительные возможности

### Обработка ошибок

Пометьте класс атрибутами `[Error]` и бросайте `EFIException` по ключу ошибки.

```csharp
using NetEFI.Runtime;

[Error("NegativeInput", "Input cannot be negative.")]
[Computable("my_sqrt", "x", "Calculates the square root of a non-negative number")]
public class SqrtFunction : CustomFunction<Complex, Complex>
{
    public override Complex Execute(Complex x, Context context)
    {
        if (x.Real < 0 && x.Imaginary == 0)
            throw new EFIException("NegativeInput", 1);

        return Complex.Sqrt(x);
    }
}
```

Можно также бросить стандартное `System.Exception` с сообщением, точно совпадающим с ключом ошибки — фреймворк конвертирует его автоматически: `throw new Exception("NegativeInput");`

### Объект `Context`

- `context.LogInfo("message")` / `context.LogError("message")` — запись в файл `netefi.log`.
- `context.IsUserInterrupted` — возвращает `true`, если пользователь нажал **Esc**.
- `context.Invoke<TResult>("functionName", args...)` — типобезопасный вызов другой зарегистрированной NetEFI-функции.

### Интеграция с дополнением CustFunc

NetEFI умеет автоматически генерировать XML-файлы определений, совместимые с дополнением `CustFunc` (диалог F3 для пользовательских функций).

1. Убедитесь, что `CustFunc.dll` установлен в Mathcad.
2. Разверните вашу библиотеку (например, `MyLibrary.dll`).
3. В рабочем листе Mathcad выполните команду: `netefi("generate_xml,MyLibrary")`
4. В папке `%APPDATA%\Mathsoft\Mathcad\docs` появится файл `MyLibrary.xml`.
5. Скопируйте его в папку `Custom Functions\docs` вашей установки Mathcad.
6. Перезапустите Mathcad. Функции появятся в диалоге `<F3>`.

### Диагностика

Лог-файл создаётся при каждом запуске Mathcad:

```
%AppData%\Mathsoft\Mathcad\NetEFI.log
```

Проверьте версию хоста, вызвав в рабочем листе: `netefi("info")`

---

## Ограничения API Mathcad

- **Максимум 10 аргументов**: пользовательская функция не может иметь более 10 входных параметров.
- **Строгие типы матриц**: функции принимают и возвращают только матрицы типа `Complex[,]`. Матрицы строк, `double` и зубчатые массивы (`Complex[][]`) не поддерживаются.
- **Нет вложенных матриц**: элементы матриц не могут сами быть матрицами.
- **Статические сообщения об ошибках**: текст ошибок задаётся на этапе компиляции через атрибуты `[Error]` и не может меняться во время выполнения.
- **Кодировка строк (только ANSI)**: legacy API Mathcad ожидает однобайтовую ANSI-кодировку. Символы Unicode, не имеющие ANSI-представления (например, эмодзи 😊), будут искажены.
- **Развёртывание F#**: при использовании F# необходимо скопировать рантайм **`FSharp.Core.dll`** в папку `Custom Functions`.

---
---

# NetEFI Framework for Mathcad

This framework allows you to create user-defined functions for **Mathcad 15** and **Mathcad Prime** using any .NET language (C#, VB.NET, F#, C++/CLI). It removes the need to write C++ code by providing a simple, attribute-based API, allowing you to focus on your function's logic in a familiar, strongly-typed managed environment.

---

## Key Features

- **Write in .NET**: Develop your custom functions in C#, VB.NET, F#, or C++/CLI.
- **Simple & Elegant API**: Define functions by inheriting from a generic base class (`CustomFunction<...>`) and describing them with a `[Computable]` attribute. No more boilerplate!
- **Full Compatibility**: Supports both the classic Mathcad 15 and modern versions of Mathcad Prime (via the legacy Custom Functions API).
- **Rich Data Types**: Natively handles complex scalars (`Complex`), complex matrices (`Complex[,]`), and strings (`string`).
- **Easy Setup**: The NuGet package automatically includes the required C++/CLI host (`netefi.dll`).
- **Advanced Features**: Provides an API for custom error handling, logging, inter-function calls, and checking for user interruption (Esc key).
- **CustFunc Integration**: Includes a tool to automatically generate XML definition files for use with the popular `CustFunc` add-in.

---

## Quick Start Guide (v0.5+)

1. Create a new **Class Library** project targeting **.NET Framework 4.8**.
2. Install the `NetEFI.Framework` NuGet package.
3. Create a public class that:
   - inherits from `NetEFI.Functions.CustomFunction<...>` with your desired input and output types;
   - is decorated with the `[Computable]` attribute.
4. Implement the abstract `Execute(...)` method with your function's logic.
5. **Build** your project (for `x86` or `x64`, depending on your Mathcad version).
6. **Deploy** the required files from your output directory (e.g., `bin\x64\Debug`) to your Mathcad installation folder:
   - `YourLibrary.dll` — your compiled project
   - `NetEFI.Abstractions.dll` — the API library
   - `netefi.dll` — the C++/CLI host
   - *Any other dependencies, like `Newtonsoft.Json.dll` or `FSharp.Core.dll`.*
   - **Mathcad Prime**: `%ProgramFiles%\PTC\Mathcad Prime X.X.X.X\Custom Functions\`
   - **Mathcad 15**: `%ProgramFiles(x86)%\Mathcad\Mathcad 15\userefi\`

---

## Examples

### C#

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

### VB.NET

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

### C++/CLI

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

### F#

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
            throw new EFIException("NegativeInput", 1);

        return Complex.Sqrt(x);
    }
}
```

You can also throw a standard `System.Exception` with a message that exactly matches an error key — the framework will automatically convert it: `throw new Exception("NegativeInput");`

### Using the `Context` Object

- `context.LogInfo("message")` / `context.LogError("message")`: Writes to the `netefi.log` file.
- `context.IsUserInterrupted`: Returns `true` if the user has pressed the **Esc** key.
- `context.Invoke<TResult>("functionName", args...)`: Invokes another registered NetEFI function in a type-safe way.

### Integration with CustFunc Add-in

NetEFI can automatically generate XML definition files compatible with the popular `CustFunc` add-in (which provides an F3 dialog for custom functions).

1. Ensure `CustFunc.dll` is installed in Mathcad.
2. Deploy your custom function library (e.g., `MyLibrary.dll`).
3. In a Mathcad worksheet, execute the command: `netefi("generate_xml,MyLibrary")`
4. This will create `MyLibrary.xml` in your `%APPDATA%\Mathsoft\Mathcad\docs` folder.
5. Copy this XML file to your Mathcad installation's `Custom Functions\docs` directory.
6. Restart Mathcad. Your functions will now appear in the `<F3>` dialog.

### Diagnostics

A log file is created on every Mathcad startup:

```
%AppData%\Mathsoft\Mathcad\NetEFI.log
```

To check the host version, call in a worksheet: `netefi("info")`

---

## Limitations of the Mathcad API

- **Maximum 10 Arguments:** A custom function cannot have more than 10 input parameters.
- **Strict Matrix Types:** Functions can only accept or return matrices of type `Complex[,]`. Matrices of `string` or `double` are not supported, nor are jagged arrays (`Complex[][]`).
- **No Nested Matrices:** The API does not support nested matrices (matrices where elements are themselves matrices).
- **Static Error Messages:** The text for custom error messages is defined at compile time via `[Error]` attributes and cannot be changed dynamically during execution.
- **String Encoding (ANSI only):** Mathcad's legacy API expects all strings to be in a single-byte ANSI encoding. Any Unicode character without a single-byte ANSI representation (e.g., emoji 😊) will be corrupted.
- **F# Deployment:** When deploying a library written in F#, you must also copy the F# runtime library **`FSharp.Core.dll`** to the `Custom Functions` directory.
