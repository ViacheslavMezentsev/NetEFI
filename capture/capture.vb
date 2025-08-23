Imports System.Numerics
Imports System.Windows.Forms
Imports System.Drawing
Imports System.Drawing.Imaging
Imports System.Runtime.InteropServices

' Import the new NetEFI namespaces
Imports NetEFI.Computables
Imports NetEFI.Design
Imports NetEFI.Functions

Namespace capture

    ' 1. Describe the function using the Computable attribute. All metadata is here.
    ' 2. Inherit from the strongly-typed base class.
    '    It takes a Complex (for deviceId) and returns a Complex(,) (the image matrix).
    <Computable("capture", "deviceId", "Takes a snapshot from the specified USB webcam.")>
    Public Class Capture
        Inherits MathcadFunction(Of Complex, Complex(,))

        ' A field to store the captured image between calls
        Public img As Bitmap = Nothing

        ' 3. Implement the simple, strongly-typed Execute method.
        '    The framework will handle the type casting automatically.
        Public Overrides Function Execute(deviceIdComplex As Complex, context As Context) As Complex(,)
            Try
                ' Extract the device ID from the real part of the complex number
                Dim deviceId As Integer = CInt(deviceIdComplex.Real)

                ' Create and show the capture form
                Dim formCapture = New FormCapture(deviceId, Me)

                If formCapture.ShowDialog() = DialogResult.OK AndAlso img IsNot Nothing Then
                    ' If the user clicked OK and an image was captured,
                    ' convert it to a matrix and return it.
                    Return ConvertBitmapToMatrix(img)
                Else
                    ' If the user cancelled the capture or an error occurred,
                    ' return an empty 1x1 matrix.
                    Return New Complex(0, 0) {}
                End If
            Catch ex As Exception
                context.LogError($"Capture failed: {ex.Message}")
                ' In case of any other error, also return an empty matrix.
                Return New Complex(0, 0) {}
            End Try
        End Function

        ' 4. A helper function to convert a Bitmap to a Mathcad-compatible matrix.
        Private Function ConvertBitmapToMatrix(matimg As Bitmap) As Complex(,)
            ' Check the image format.
            If (matimg.PixelFormat <> PixelFormat.Format24bppRgb) AndAlso (matimg.PixelFormat <> PixelFormat.Format32bppRgb) Then
                Throw New Exception("Wrong image format. Must be 24bpp or 32bpp.")
            End If

            ' Lock the bitmap's bits for direct memory access.
            Dim bmpData = matimg.LockBits(New Rectangle(0, 0, matimg.Width, matimg.Height), ImageLockMode.ReadOnly, matimg.PixelFormat)
            Dim ptr = bmpData.Scan0

            ' Declare an array to hold the bytes of the bitmap.
            Dim length = bmpData.Stride * matimg.Height
            Dim rgbValues(length - 1) As Byte

            ' Copy the RGB values into the array.
            Marshal.Copy(ptr, rgbValues, 0, length)

            ' Unlock the bits.
            matimg.UnlockBits(bmpData)

            Dim bytesPerPixel = Image.GetPixelFormatSize(bmpData.PixelFormat) \ 8
            Dim height = matimg.Height
            Dim width = matimg.Width

            ' The resulting matrix will have dimensions [height x (3 * width)]
            ' to store the R, G, and B planes side-by-side.
            Dim matrix As Complex(,) = New Complex(height - 1, (3 * width) - 1) {}

            For y = 0 To height - 1
                Dim strideY = bmpData.Stride * y
                For x = 0 To width - 1
                    Dim index = strideY + x * bytesPerPixel

                    ' The byte order in a Bitmap is BGR.
                    ' We will arrange them in the matrix as [R plane, G plane, B plane].

                    ' Blue component
                    matrix(y, 2 * width + x) = New Complex(rgbValues(index), 0)
                    index += 1

                    ' Green component
                    matrix(y, width + x) = New Complex(rgbValues(index), 0)
                    index += 1

                    ' Red component
                    matrix(y, x) = New Complex(rgbValues(index), 0)
                Next
            Next

            Return matrix
        End Function

    End Class

End Namespace