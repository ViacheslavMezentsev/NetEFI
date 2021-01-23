Imports System.Reflection
Imports NetEFI

Public Class vbtest
    Implements IFunction

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return New FunctionInfo("vbtest", "cmd", "return info", _
                GetType(String), New Type() {GetType(String)})
        End Get

    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo

        Return Info

    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        Try

            Dim cmd = CType(args(0), String)

            result = "help: info, list"

            If cmd.Equals( "info" ) Then

                Dim name = Assembly.GetExecutingAssembly().GetName()

                result = $"{name.Name}: {name.Version}"

            ElseIf cmd.Equals( "list" ) Then

                Dim list = New List(Of String)()

                Dim types = Assembly.GetExecutingAssembly().GetTypes()

                For Each type As Type In types

                    If Not type.IsPublic OrElse type.IsAbstract OrElse Not GetType( IFunction ).IsAssignableFrom( type ) Then Continue For

                    Dim f = DirectCast( Activator.CreateInstance( type ), IFunction )

                    list.Add( f.Info.Name )

                Next

                result = String.Join(", ", list.ToArray())

            End If

        Catch ex As Exception

            result = Nothing
            Return False
        End Try

        Return True

    End Function

End Class
