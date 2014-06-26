Imports System.Reflection
Imports NetEFI

Public Class vbtest

    Implements IFunction

    Private _info As FunctionInfo

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return _info
        End Get
    End Property

    Public Sub New()

        _info = New FunctionInfo("vbtest", "cmd", "return info", _
            New Uri(Assembly.GetExecutingAssembly().CodeBase).LocalPath, _
            GetType(String), _
            New Type() {GetType(String)})
    End Sub

    Public Function GetFunctionInfo(ByVal lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo

        Return Info
    End Function

    Public Function NumericEvaluation(ByVal args As Object(), ByRef result As Object) As Boolean Implements IFunction.NumericEvaluation

        Try

            Dim cmd = DirectCast(args(0), [String])

            result = "empty"

            If cmd.Equals("info") Then

                result = Assembly.GetExecutingAssembly().ToString()

            ElseIf cmd.Equals("list") Then

                Dim list = New List(Of String)()

                Dim types = Assembly.GetExecutingAssembly().GetTypes()

                For Each type As Type In types

                    If Not type.IsPublic OrElse type.IsAbstract OrElse Not GetType(IFunction).IsAssignableFrom(type) Then
                        Continue For
                    End If

                    Dim f = DirectCast(Activator.CreateInstance(type), IFunction)

                    list.Add(f.Info.Name)
                Next

                result = [String].Join(", ", list.ToArray())

            End If

        Catch ex As Exception

            result = Nothing
            Return False
        End Try

        Return True

    End Function

End Class
