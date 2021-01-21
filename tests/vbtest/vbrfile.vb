Imports System.IO
Imports NetEFI


Public Class vbrfile
    Implements IFunction

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return New FunctionInfo("vbrfile", "file", "return file content", _
                GetType(String), New Type() {GetType(String)})
        End Get

    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo

        Return Info

    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        Dim path = CStr( args(0) )

        result = if( File.Exists( path ), File.ReadAllText( path ), "" )

        Return True

    End Function

End Class
