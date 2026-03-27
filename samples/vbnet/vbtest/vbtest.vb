Imports System.Linq
Imports System.Reflection
Imports NetEFI.Functions
Imports NetEFI.Runtime

Namespace VbTest

    <Computable("vbtest", "cmd", "A utility function to inspect the VB.NET test assembly.")>
    Public Class VbTest
        Inherits CustomFunction(Of String, String)

        Public Overrides Function Execute(cmd As String, context As Context) As String
            Dim currentAssembly = Me.GetType().Assembly

            Try
                If cmd.Equals("info", StringComparison.OrdinalIgnoreCase) Then
                    Dim assemblyName = currentAssembly.GetName()

                    Return String.Format("{0}: {1}", assemblyName.Name, assemblyName.Version)
                End If

                If cmd.Equals("list", StringComparison.OrdinalIgnoreCase) Then
                    ' Find all function types in this assembly.
                    Dim functionTypes = currentAssembly.GetTypes().Where(
                        Function(t) t.IsPublic AndAlso Not t.IsAbstract AndAlso GetType(CustomFunctionBase).IsAssignableFrom(t))

                    Dim names = functionTypes.
                        Select(Function(t) t.GetCustomAttribute(Of ComputableAttribute)(False)).
                        Where(Function(attr) attr IsNot Nothing).
                        Select(Function(attr) attr.Name)

                    Return String.Join(", ", names)
                End If

            Catch ex As Exception
                context.LogError($"vbtest failed: {ex.Message}")
                Return $"ERROR: {ex.Message}"
            End Try

            Return "help: info, list"
        End Function

    End Class

End Namespace