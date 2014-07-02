Imports NetEFI


Public Class vbsum
    Implements IFunction

    Public ReadOnly Property Info() As FunctionInfo Implements IFunction.Info

        Get
            Return New FunctionInfo("vbsum", "a,b", "complex sum of scalars a and b",
                    GetType(TComplex),
                    New Type() {GetType(TComplex), GetType(TComplex)})
        End Get
    End Property

    Public Function GetFunctionInfo(lang As String) As FunctionInfo Implements IFunction.GetFunctionInfo
        Return Info
    End Function

    Public Function NumericEvaluation(args As Object(), ByRef result As Object, ByRef context As Context) As Boolean _
        Implements IFunction.NumericEvaluation

        result = Evaluate(CType(args(0), TComplex), CType(args(1), TComplex))

        Return True
    End Function

    Public Function Evaluate(a As TComplex, b As TComplex) As TComplex

        Return New TComplex(a.Real + b.Real, a.Imaginary + b.Imaginary)

    End Function
End Class
