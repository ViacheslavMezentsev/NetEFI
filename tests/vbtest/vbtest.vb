Imports System.Linq
Imports System.Reflection
Imports NetEFI

Public Class vbtest
    Implements IFunction

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return New FunctionInfo( "vbtest", "cmd", "return info", _
                GetType(String), New Type() {GetType(String)})
        End Get

    End Property

    Public Function GetFunctionInfo( lang As String ) As FunctionInfo Implements IFunction.GetFunctionInfo

        Return Info

    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        result = "help: info, list"

        Dim assembl = Assembly.GetExecutingAssembly()

        Try

            Dim cmd = CType(args(0), String)

            If cmd = "info" Then

                Dim name = assembl.GetName()

                result = $"{name.Name}: {name.Version}"

            ElseIf cmd = "list" Then

                Dim types = assembl.GetTypes().Where( Function(t) t.IsPublic AndAlso Not t.IsAbstract AndAlso GetType( IFunction ).IsAssignableFrom(t) )

                Dim names = types.Select( Function(t) DirectCast( Activator.CreateInstance(t), IFunction ).Info.Name ).ToArray()

                result = String.Join( ", ", names )

            End If

        Catch ex As Exception

            result = Nothing
            Return False

        End Try

        Return True

    End Function

End Class
