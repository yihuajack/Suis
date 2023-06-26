Attribute VB_Name = "Functions"
Option Explicit

Function UniformRows(ByRef rArray As Variant) As Boolean
'Returns true if, for all columns of rArray, all rows have the same value.
'This is used to test whether a 2D solution can be used.
Dim i As Integer, j As Integer
Dim rValue As Double
UniformRows = True
For j = LBound(rArray, 2) To UBound(rArray, 2)
    rValue = rArray(1, j)
    For i = LBound(rArray, 1) To UBound(rArray, 1)
        If rArray(i, j) <> rValue Then
            UniformRows = False: Exit Function
        End If
    Next i
Next j
End Function

Function InterpolateColumns(ByVal x As Double, ByVal mCol As Integer, ByRef xArray As Variant, _
                            ByVal nCol As Integer, ByRef yArray As Variant, ByRef bLog As Boolean, ByRef bZero As Boolean) As Double
'Returns the value of y for the given value of x by parabolic interpolation of nCol of yArray vs mCol of xArray
'If bLog is true, then Log(yArray) is used in place of yArray and Exp(y) is returned in place of y
'For x values outside the range of xArray, returns zero if bZero is true and bLog is false, otherwise returns the end-point value from yArray
'bLog is set false if any non-positive yArray values are detected
'xArray must be in increasing order. If not, only the range starting with the first entry that is increasing is searched.
Dim index As Integer, nValues As Integer
Dim x1 As Double, x2 As Double, x3 As Double
Dim y1 As Double, y2 As Double, y3 As Double
Dim a As Double, b As Double, c As Double
Dim denominator As Double
'Find nValues and clear bLog if yArray value is non-positive
nValues = LBound(xArray)
If yArray(nValues, nCol) <= 0 Then bLog = False
For index = LBound(xArray) + 1 To UBound(xArray)
    If xArray(index, mCol) > xArray(nValues, mCol) Then nValues = index Else Exit For
    If yArray(nValues, nCol) <= 0 Then bLog = False
Next index
'Consider endpoints
If x < xArray(1, mCol) Then
    If bZero And Not bLog Then InterpolateColumns = 0 Else InterpolateColumns = yArray(1, nCol)
    Exit Function
End If
If x > xArray(nValues, mCol) Then
    If bZero And Not bLog Then InterpolateColumns = 0 Else InterpolateColumns = yArray(nValues, nCol)
    Exit Function
End If
'Find midpoint index for parabolic interpolation
For index = 1 To nValues - 1
    If x < (xArray(index, mCol) + xArray(index + 1, mCol)) / 2 Then Exit For
Next index
If index = 1 Then index = 2
If index = nValues Then index = nValues - 1
'Use parabolic interpolation
x1 = xArray(index - 1, mCol)
x2 = xArray(index, mCol)
x3 = xArray(index + 1, mCol)
y1 = yArray(index - 1, nCol): If bLog Then y1 = Log(y1)
y2 = yArray(index, nCol): If bLog Then y2 = Log(y2)
y3 = yArray(index + 1, nCol): If bLog Then y3 = Log(y3)
denominator = (x1 ^ 2 - x2 ^ 2) * (x2 - x3) - (x2 ^ 2 - x3 ^ 2) * (x1 - x2)
If denominator <> 0 Then a = ((x2 - x3) * (y1 - y2) - (x1 - x2) * (y2 - y3)) / denominator Else Exit Function
If x1 <> x2 Then b = ((y1 - y2) - a * (x1 ^ 2 - x2 ^ 2)) / (x1 - x2) Else Exit Function
c = y1 - b * x1 - a * x1 ^ 2
InterpolateColumns = a * x ^ 2 + b * x + c
If bLog Then InterpolateColumns = Exp(InterpolateColumns)
End Function

Function SurfaceAverage(ByVal nNodesXmin As Integer, ByVal nNodesXmax As Integer, _
        ByVal nNodesYmin As Integer, ByVal nNodesYmax As Integer, ByRef sArray As Variant) As Double
Dim i As Integer, j As Integer
Dim localValue As Double
'Calculates the spatial average of a surface array spanning the specified range of x and y nodes. Accepts
'sArray that has only i and j arguments.
SurfaceAverage = 0
For i = nNodesXmin To nNodesXmax: For j = nNodesYmin To nNodesYmax
    localValue = sArray(i, j)
    If (i = nNodesXmin Or i = nNodesXmax) And nNodesXmin <> nNodesXmax Then localValue = 0.5 * localValue
    If (j = nNodesYmin Or j = nNodesYmax) And nNodesYmin <> nNodesYmax Then localValue = 0.5 * localValue
    SurfaceAverage = SurfaceAverage + localValue
Next j: Next i
If (nNodesXmax - nNodesXmin) > 1 Then SurfaceAverage = SurfaceAverage / (nNodesXmax - nNodesXmin)
If (nNodesYmax - nNodesYmin) > 1 Then SurfaceAverage = SurfaceAverage / (nNodesYmax - nNodesYmin)
End Function

Function SurfaceAverageFR(ByVal nSurface As Integer, ByVal nNodesXmin As Integer, ByVal nNodesXmax As Integer, _
                    ByVal nNodesYmin As Integer, ByVal nNodesYmax As Integer, ByRef sArray As Variant) As Double
Dim i As Integer, j As Integer
Dim localValue As Double
'Calculates the spatial average of a surface array spanning the specified range of x and y nodes
'for the given surface. Accepts sArray that has nSurface as its first argument. Used for Voc estimation.
SurfaceAverageFR = 0
For i = nNodesXmin To nNodesXmax: For j = nNodesYmin To nNodesYmax
    localValue = sArray(nSurface, i, j)
    If (i = nNodesXmin Or i = nNodesXmax) And nNodesXmin <> nNodesXmax Then localValue = 0.5 * localValue
    If (j = nNodesYmin Or j = nNodesYmax) And nNodesYmin <> nNodesYmax Then localValue = 0.5 * localValue
    SurfaceAverageFR = SurfaceAverageFR + localValue
Next j: Next i
If (nNodesXmax - nNodesXmin) > 1 Then SurfaceAverageFR = SurfaceAverageFR / (nNodesXmax - nNodesXmin)
If (nNodesYmax - nNodesYmin) > 1 Then SurfaceAverageFR = SurfaceAverageFR / (nNodesYmax - nNodesYmin)
End Function

Function ContactAverage(ByVal nCarrier As Integer, _
                    ByVal nNodesXmin As Integer, ByVal nNodesXmax As Integer, _
                    ByVal nNodesYmin As Integer, ByVal nNodesYmax As Integer, ByRef sArray As Variant) As Double
Dim i As Integer, j As Integer, iSurface As Integer
Dim localValue As Double, localNode As Double, valueSum As Double, nodeSum As Double
'Calculates the spatial average of a surface array spanning the specified range of x and y nodes
'for nodes where the designated carrier is contacted, across both surfaces.
'sArray is (front to rear, holes to electrons, 1 to nnodes, 1 to nnodes)
valueSum = 0: nodeSum = 0
For iSurface = front To rear
    For i = nNodesXmin To nNodesXmax: For j = nNodesYmin To nNodesYmax
        If sContact(iSurface, nCarrier, i, j) > 0 Then
            localValue = sArray(iSurface, nCarrier, i, j): localNode = 1
            If (i = nNodesXmin Or i = nNodesXmax) And nNodesXmin <> nNodesXmax Then
                localValue = 0.5 * localValue: localNode = 0.5 * localNode
            End If
            If (j = nNodesYmin Or j = nNodesYmax) And nNodesYmin <> nNodesYmax Then
                localValue = 0.5 * localValue: localNode = 0.5 * localNode
            End If
            valueSum = valueSum + localValue: nodeSum = nodeSum + localNode
        End If
    Next j: Next i
Next iSurface
If nodeSum > 0 Then ContactAverage = valueSum / nodeSum Else ContactAverage = 0
End Function

Function VolumeAverage(ByVal nNodesX, ByVal nNodesY, ByVal nNodesZ, ByRef vArray As Variant) As Double
Dim i As Integer, j As Integer, k As Integer
Dim localValue As Double
'Calculates the spatial average of a volume array of nNodesX x nNodesY x nNodesZ
VolumeAverage = 0
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    localValue = vArray(i, j, k)
    If (i = 1 Or i = nNodesX) And nNodesX <> 1 Then localValue = 0.5 * localValue
    If (j = 1 Or j = nNodesY) And nNodesY <> 1 Then localValue = 0.5 * localValue
    If (k = 1 Or k = nNodesZ) And nNodesZ <> 1 Then localValue = 0.5 * localValue
    VolumeAverage = VolumeAverage + localValue
Next k: Next j: Next i
If nNodesX > 1 Then VolumeAverage = VolumeAverage / (nNodesX - 1)
If nNodesY > 1 Then VolumeAverage = VolumeAverage / (nNodesY - 1)
If nNodesZ > 1 Then VolumeAverage = VolumeAverage / (nNodesZ - 1)
End Function

Function VolumeMin(ByRef vArray As Variant) As Double
'Finds the minimum value of a volume array of nNodesX x nNodesY x nNodesZ
Dim i As Integer, j As Integer, k As Integer
VolumeMin = vArray(1, 1, 1)
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    If vArray(i, j, k) < VolumeMin Then VolumeMin = vArray(i, j, k)
Next k: Next j: Next i
End Function

Function VolumeMax(ByRef vArray As Variant) As Double
'Finds the maximum value of a volume array of nNodesX x nNodesY x nNodesZ
Dim i As Integer, j As Integer, k As Integer
VolumeMax = vArray(1, 1, 1)
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    If vArray(i, j, k) > VolumeMax Then VolumeMax = vArray(i, j, k)
Next k: Next j: Next i
End Function

Function MMult3D(ByRef a As Variant, ByRef b As Variant) As Variant
'Returns an MxNxP matrix (row,col,plane) that is MxM matrix A (row,col)
' times MxNxP matrix B (row,col,plane), where each plane of B is multiplied by A
Dim c As Variant
Dim q As Variant
Dim r As Variant
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer, P As Integer
m = UBound(b, 1)
n = UBound(b, 2)
P = UBound(b, 3)
ReDim c(1 To m, 1 To n, 1 To P) As Double
ReDim q(1 To m, 1 To n) As Double
ReDim r(1 To m, 1 To n) As Double
For k = 1 To P
    For i = 1 To m: For j = 1 To n
        q(i, j) = b(i, j, k)
    Next j: Next i
    r = Application.MMult(a, q)
    For i = 1 To m: For j = 1 To n
        c(i, j, k) = r(i, j)
    Next j: Next i
Next k
MMult3D = c
End Function

Function MTrans3Drc(ByRef a As Variant) As Variant
'Returns an NxMxP matrix that has the row and column indices of MxNxP matrix A transposed.
'The last index (plane) is preserved.
Dim c As Variant
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer, P As Integer
m = UBound(a, 1)
n = UBound(a, 2)
P = UBound(a, 3)
ReDim c(1 To n, 1 To m, 1 To P) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To P
    c(j, i, k) = a(i, j, k)
Next k: Next j: Next i
MTrans3Drc = c
End Function

Function MTrans3Drp(ByRef a As Variant) As Variant
'Returns an PxNxM matrix that has the row and plane indices of MxNxP matrix A transposed.
'The second index (column) is preserved.
Dim c As Variant
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer, P As Integer
m = UBound(a, 1)
n = UBound(a, 2)
P = UBound(a, 3)
ReDim c(1 To P, 1 To n, 1 To m) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To P
    c(k, j, i) = a(i, j, k)
Next k: Next j: Next i
MTrans3Drp = c
End Function

Function MTrans3Dback(ByRef a As Variant) As Variant
'Returns an NxPxM matrix that has the indices of MxNxP matrix A rotated back by one index.
Dim c As Variant
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer, P As Integer
m = UBound(a, 1)
n = UBound(a, 2)
P = UBound(a, 3)
ReDim c(1 To n, 1 To P, 1 To m) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To P
    c(j, k, i) = a(i, j, k)
Next k: Next j: Next i
MTrans3Dback = c
End Function

Function MAdd3D(a As Variant, b As Variant) As Variant
'Returns the entry-by-entry sum of two 3D matrices A and B.
Dim i As Integer, j As Integer, k As Integer, m As Integer, n As Integer, P As Integer
Dim q As Variant
m = UBound(a, 1)
n = UBound(a, 2)
P = UBound(a, 3)
ReDim q(1 To m, 1 To n, 1 To P) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To P
    q(i, j, k) = a(i, j, k) + b(i, j, k)
Next k: Next j: Next i
MAdd3D = q
End Function

Function MScale3D(a As Variant, b As Double) As Variant
'Returns a 3D matrix with each entry being B times the corresponding entry in 3D matrix A.
Dim i As Integer, j As Integer, k As Integer, m As Integer, n As Integer, P As Integer
Dim q As Variant
m = UBound(a, 1)
n = UBound(a, 2)
P = UBound(a, 3)
ReDim q(1 To m, 1 To n, 1 To P) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To P
    q(i, j, k) = a(i, j, k) * b
Next k: Next j: Next i
MScale3D = q
End Function

Function MSolve2x2(ByRef matrix As Variant, ByRef vector As Variant) As Variant
Dim det As Double, a As Double, b As Double, c As Double, d As Double
Dim result(1 To 2) As Double
a = matrix(1, 1)
b = matrix(1, 2)
c = matrix(2, 1)
d = matrix(2, 2)
det = a * d - b * c
result(1) = (-d * vector(1) + b * vector(2)) / det
result(2) = (c * vector(1) - a * vector(2)) / det
MSolve2x2 = result
End Function

Function Sinh(ByVal x As Double) As Double
If x > 100 Then x = 100
If x < -100 Then x = -100
Sinh = (Exp(x) - Exp(-x)) / 2
End Function

Function Cosh(ByVal x As Double) As Double
If x > 100 Then x = 100
If x < -100 Then x = -100
Cosh = (Exp(x) + Exp(-x)) / 2
End Function

Function FunctionGamma(ByVal rPhiP As Double, ByVal rPhiN As Double) As Double
'Returns excess carrier density, gamma, as a function of phiP and phiN, assuming charge neutrality.
Dim EqPN2 As Double, Ni2 As Double
EqPN2 = (rEqP + rEqN) / 2
If rPhiP - rPhiN > 0 Then
    FunctionGamma = (-EqPN2 + Sqr(EqPN2 ^ 2 + rNi ^ 2 * (Exp((rPhiP - rPhiN) / rVt) - 1)))
Else: FunctionGamma = 0
End If
End Function

Function FunctionPsi(ByVal PhiP As Double, ByVal PhiN As Double) As Double
'Returns excess electric potential, psi, as a function of phiP and phiN, assuming charge neutrality.
Dim N2ni As Double
If pType Then       'Note that if Ndoping=0, both expressions give Psi = (PhiP+PhiN)/2
    N2ni = (rEqP - rEqN) / (2 * rNi)
    FunctionPsi = PhiP - rVt * Log(N2ni + Sqr(N2ni ^ 2 + Exp((PhiP - PhiN) / rVt))) - rPsiEq
Else
    N2ni = (rEqN - rEqP) / (2 * rNi)
    FunctionPsi = PhiN + rVt * Log(N2ni + Sqr(N2ni ^ 2 + Exp((PhiP - PhiN) / rVt))) - rPsiEq
End If
End Function

Function PhiP(ByVal gamma As Double, ByVal psi As Double) As Double
'Returns hole quasi-Fermi potential for given excess hole density and excess electrostatic potential.
If gamma <= 0 Then gamma = rMinEq
PhiP = psi + rVt * Log((rEqP + gamma) / rEqP)
End Function

Function PhiN(ByVal gamma As Double, ByVal psi As Double) As Double
'Returns electron quasi-Fermi potential for given excess electron density and excess electrostatic potential.
If gamma <= 0 Then gamma = rMinEq
PhiN = psi - rVt * Log((rEqN + gamma) / rEqN)
End Function

Function FunctionDeltaGamma() As Double
'Increment used to construct Jacobian in Adjust Surface routines.
'Needs to be small enough to stay locally linear but large enough to avoid numerical truncation errors.
FunctionDeltaGamma = 0.01 * FunctionGamma(rVoltage, 0) + rMinEq
End Function

Function FunctionDeltaPsi() As Double
'Increment used to construct Jacobian in Adjust Surface routines.
'Needs to be small enough to stay locally linear but large enough to avoid numerical truncation errors.
FunctionDeltaPsi = 0.01 * rVt
End Function

Function VocEstimate(ByVal totalGeneration As Double) As Double
'Estimates the Voc based on recombination parameters, assuming flat-band condition (gamma uniform over volume).
'Iteration is used to accommodate nonlinearity of SRH and Auger recombination.
Dim gamma As Double, Voc As Double, Jo1 As Double
Dim i As Integer
gamma = 0
For i = 1 To 5
    Jo1 = SurfaceAverageFR(front, 1, nNodesX, 1, nNodesY, sJo1) + SurfaceAverageFR(rear, 1, nNodesX, 1, nNodesY, sJo1) _
        + q * rNi ^ 2 * deviceHeight / ((rEqN + gamma) * rTauP + (rEqP + gamma) * rTauN) _
        + q * rNi ^ 2 * deviceHeight * rCa * (rEqN + rEqP + 2 * gamma)
    Voc = rVt * Log(1 + q * totalGeneration / Jo1)
    gamma = FunctionGamma(Voc, 0)
Next i
VocEstimate = Voc
End Function

Function SurfaceFlux(ByVal nSurface As Integer, ByVal nCarrier As Integer, _
                     ByVal i As Integer, ByVal j As Integer) As Double
'Requires Gamma and Gradients of Gamma and Psi to return the flux of the designated carrier INTO the chosen surface node.
If nSurface = front Then
    If nCarrier = holes Then
        SurfaceFlux = rDp * (sFrontGradGamma(i, j) + (rEqP + sFrontGamma(i, j)) * sFrontGradPsi(i, j) / rVt)
    Else
        SurfaceFlux = rDn * (sFrontGradGamma(i, j) - (rEqN + sFrontGamma(i, j)) * sFrontGradPsi(i, j) / rVt)
    End If
Else
    If nCarrier = holes Then
        SurfaceFlux = -rDp * (sRearGradGamma(i, j) + (rEqP + sRearGamma(i, j)) * sRearGradPsi(i, j) / rVt)
    Else
        SurfaceFlux = -rDn * (sRearGradGamma(i, j) - (rEqN + sRearGamma(i, j)) * sRearGradPsi(i, j) / rVt)
    End If
End If
End Function

Function SurfaceRecombinationJo1(ByVal nSurface As Integer, ByVal i As Integer, ByVal j As Integer, ByVal gamma As Double) As Double
'This routine returns the recombination rate due to Jo1 at the specified node at the specified surface.
Dim voltage As Double
voltage = rVt * Log(((rEqP + gamma) * (rEqN + gamma)) / rNi ^ 2)
If nSurface = front Then
    SurfaceRecombinationJo1 = sJo1(front, i, j) * (Exp(voltage / rVt) - 1) / q
Else
    SurfaceRecombinationJo1 = sJo1(rear, i, j) * (Exp(voltage / rVt) - 1) / q
End If
End Function

Function SurfaceRecombinationJo2(ByVal nSurface As Integer, ByVal i As Integer, ByVal j As Integer, ByVal gamma As Double) As Double
'This routine returns the recombination rate due to Jo2 and Gsh at the specified node at the specified surface.
Dim voltage As Double
voltage = rVt * Log(((rEqP + gamma) * (rEqN + gamma)) / rNi ^ 2)
If nSurface = front Then
    SurfaceRecombinationJo2 = sJo2(front, i, j) * (Exp(voltage / (2 * rVt)) - 1) / q _
                    + sGsh(front, i, j) * voltage / q
Else
    SurfaceRecombinationJo2 = sJo2(rear, i, j) * (Exp(voltage / (2 * rVt)) - 1) / q _
                    + sGsh(rear, i, j) * voltage / q
End If
End Function

Function PointDefectRecombination(ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Double
'Calculates and returns point defect recombination only if the given node is the one closest to the defect position.
Dim voltage As Double, localGamma As Double, localP As Double, localN As Double
Dim deltaGamma As Double, deltaRdeltaGamma As Double, thisIo As Double, damping As Double
PointDefectRecombination = 0
thisIo = rPointDefectIo
If i - 1 = Round(rDefectPosition(point, xAxis) * (nNodesX - 1) / 100, 0) And _
   j - 1 = (nNodesY - 1) - Round(rDefectPosition(point, yAxis) * (nNodesY - 1) / 100, 0) And _
   k - 1 = Round(rDefectPosition(point, zAxis) * (nNodesZ - 1) / 100, 0) Then
    localGamma = vGamma(i, j, k): localP = rEqP + localGamma: localN = rEqN + localGamma
    If i = 1 Or i = nNodesX Then thisIo = 2 * thisIo
    If j = 1 Or j = nNodesY Then thisIo = 2 * thisIo
    If k = 1 Or k = nNodesZ Then thisIo = 2 * thisIo
    If localGamma > 0 Then
        voltage = rVt * Log(localP * localN / rNi ^ 2)
        PointDefectRecombination = rPointDefectIo * (Exp(voltage / (rPointDefectN * rVt)) - 1) * (nNodesX - 1) * (nNodesY - 1) * (nNodesZ - 1) / (q * deviceWidth * deviceLength * deviceHeight)
        deltaRdeltaGamma = thisIo / rPointDefectN * (nNodesX - 1) * (nNodesY - 1) * (nNodesZ - 1) / (q * rNi ^ 2 * deviceWidth * deviceLength * deviceHeight) * (localP + localN) * Exp(voltage / (rPointDefectN * rVt)) / Exp(voltage / rVt)
        damping = rPointDamping / (1 - rDeltaGammaDeltaR * deltaRdeltaGamma)
        vPointRecombination(i, j, k) = vPointRecombination(i, j, k) + damping * (PointDefectRecombination - vPointRecombination(i, j, k))
    End If
    PointDefectRecombination = vPointRecombination(i, j, k)
End If
End Function

Function LineDefectRecombination(ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Double
'Calculates and returns line defect recombination only at nodes closest to the defect line position.
'Use a more conservative damping mode for Voc calculation.
Dim voltage As Double, localGamma As Double, localP As Double, localN As Double
Dim deltaGamma As Double, deltaRdeltaGamma As Double, thisIo As Double, damping As Double
LineDefectRecombination = 0
thisIo = rLineDefectIo
If lineDefectDirection = "Side" And _
      j - 1 = (nNodesY - 1) - Round(rDefectPosition(line, yAxis) * (nNodesY - 1) / 100, 0) And _
      k - 1 = Round(rDefectPosition(line, zAxis) * (nNodesZ - 1) / 100, 0) And _
      i - 1 <= rDefectPosition(line, xAxis) / 100 * (nNodesX - 1) Then
    localGamma = vGamma(i, j, k): localP = rEqP + localGamma: localN = rEqN + localGamma
    If j = 1 Or j = nNodesY Then thisIo = 2 * thisIo
    If k = 1 Or k = nNodesZ Then thisIo = 2 * thisIo
    If localGamma > 0 Then
        voltage = rVt * Log(localP * localN / rNi ^ 2)
        LineDefectRecombination = thisIo * (Exp(voltage / (rLineDefectN * rVt)) - 1) * (nNodesY - 1) * (nNodesZ - 1) / (q * deviceLength * deviceHeight)
        deltaRdeltaGamma = thisIo / rLineDefectN * (nNodesY - 1) * (nNodesZ - 1) / (q * rNi ^ 2 * deviceLength * deviceHeight) * (localP + localN) * Exp(voltage / (rLineDefectN * rVt)) / Exp(voltage / rVt)
        damping = rLineDamping / (1 - 2 * rDeltaGammaDeltaR * deltaRdeltaGamma)
        vLineRecombination(i, j, k) = vLineRecombination(i, j, k) + damping * (LineDefectRecombination - vLineRecombination(i, j, k))
    End If
    LineDefectRecombination = vLineRecombination(i, j, k)
End If
If lineDefectDirection = "Face" And _
      i - 1 = Round(rDefectPosition(line, xAxis) * (nNodesX - 1) / 100, 0) And _
      k - 1 = Round(rDefectPosition(line, zAxis) * (nNodesZ - 1) / 100, 0) And _
      nNodesY - j <= rDefectPosition(line, yAxis) / 100 * (nNodesY - 1) Then
    localGamma = vGamma(i, j, k): localP = rEqP + localGamma: localN = rEqN + localGamma
    If i = 1 Or i = nNodesX Then thisIo = 2 * thisIo
    If k = 1 Or k = nNodesZ Then thisIo = 2 * thisIo
    If localGamma > 0 Then
        voltage = rVt * Log(localP * localN / rNi ^ 2)
        LineDefectRecombination = thisIo * (Exp(voltage / (rLineDefectN * rVt)) - 1) * (nNodesX - 1) * (nNodesZ - 1) / (q * deviceWidth * deviceHeight)
        deltaRdeltaGamma = thisIo / rLineDefectN * (nNodesX - 1) * (nNodesZ - 1) / (q * rNi ^ 2 * deviceWidth * deviceHeight) * (localP + localN) * Exp(voltage / (rLineDefectN * rVt)) / Exp(voltage / rVt)
        damping = rLineDamping / (1 - 2 * rDeltaGammaDeltaR * deltaRdeltaGamma)
        vLineRecombination(i, j, k) = vLineRecombination(i, j, k) + damping * (LineDefectRecombination - vLineRecombination(i, j, k))
    End If
    LineDefectRecombination = vLineRecombination(i, j, k)
End If
If lineDefectDirection = "Top" And _
      i - 1 = Round(rDefectPosition(line, xAxis) * (nNodesX - 1) / 100, 0) And _
      j - 1 = (nNodesY - 1) - Round(rDefectPosition(line, yAxis) * (nNodesY - 1) / 100, 0) And _
      k - 1 <= rDefectPosition(line, zAxis) / 100 * (nNodesZ - 1) Then
    localGamma = vGamma(i, j, k): localP = rEqP + localGamma: localN = rEqN + localGamma
    If i = 1 Or i = nNodesX Then thisIo = 2 * thisIo
    If j = 1 Or j = nNodesY Then thisIo = 2 * thisIo
    If localGamma > 0 Then
        voltage = rVt * Log(localP * localN / rNi ^ 2)
        LineDefectRecombination = thisIo * (Exp(voltage / (rLineDefectN * rVt)) - 1) * (nNodesX - 1) * (nNodesY - 1) / (q * deviceWidth * deviceLength)
        deltaRdeltaGamma = thisIo / rLineDefectN * (nNodesX - 1) * (nNodesY - 1) / (q * rNi ^ 2 * deviceWidth * deviceLength) * (localP + localN) * Exp(voltage / (rLineDefectN * rVt)) / Exp(voltage / rVt)
        damping = rLineDamping / (1 - 2 * rDeltaGammaDeltaR * deltaRdeltaGamma)
        vLineRecombination(i, j, k) = vLineRecombination(i, j, k) + damping * (LineDefectRecombination - vLineRecombination(i, j, k))
    End If
    LineDefectRecombination = vLineRecombination(i, j, k)
End If
End Function

Function PlaneDefectRecombination(ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Double
'Calculates and returns plane defect recombination only at nodes closest to the defect plane position.
Dim voltage As Double, localGamma As Double, localP As Double, localN As Double
Dim deltaGamma As Double, deltaRdeltaGamma As Double, thisIo As Double, damping As Double
PlaneDefectRecombination = 0
thisIo = rPlaneDefectIo
If planeDefectDirection = "Side" And i - 1 = Round(rDefectPosition(plane, xAxis) * (nNodesX - 1) / 100, 0) And _
        nNodesY - j <= rDefectPosition(plane, yAxis) / 100 * (nNodesY - 1) And _
        k - 1 <= rDefectPosition(plane, zAxis) / 100 * (nNodesZ - 1) Then
    localGamma = vGamma(i, j, k): localP = rEqP + localGamma: localN = rEqN + localGamma
    If i = 1 Or i = nNodesX Then thisIo = 2 * thisIo
    If localGamma > 0 Then
        voltage = rVt * Log(localP * localN / rNi ^ 2)
        PlaneDefectRecombination = thisIo * (Exp(voltage / (rPlaneDefectN * rVt)) - 1) * (nNodesX - 1) / (q * deviceWidth)
        deltaRdeltaGamma = thisIo / rPlaneDefectN * (nNodesX - 1) / (q * rNi ^ 2 * deviceWidth) * (localP + localN) * Exp(voltage / (rPlaneDefectN * rVt)) / Exp(voltage / rVt)
        damping = rPlaneDamping / (1 - 4 * rDeltaGammaDeltaR * deltaRdeltaGamma)
        vPlaneRecombination(i, j, k) = vPlaneRecombination(i, j, k) + damping * (PlaneDefectRecombination - vPlaneRecombination(i, j, k))
    End If
    PlaneDefectRecombination = vPlaneRecombination(i, j, k)
End If
If planeDefectDirection = "Face" And j - 1 = (nNodesY - 1) - Round(rDefectPosition(plane, yAxis) * (nNodesY - 1) / 100, 0) And _
        i - 1 <= rDefectPosition(plane, xAxis) / 100 * (nNodesX - 1) And _
        k - 1 <= rDefectPosition(plane, zAxis) / 100 * (nNodesZ - 1) Then
    localGamma = vGamma(i, j, k): localP = rEqP + localGamma: localN = rEqN + localGamma
    If j = 1 Or j = nNodesY Then thisIo = 2 * thisIo
    If localGamma > 0 Then
        voltage = rVt * Log(localP * localN / rNi ^ 2)
        PlaneDefectRecombination = rPlaneDefectIo * (Exp(voltage / (rPlaneDefectN * rVt)) - 1) * (nNodesY - 1) / (q * deviceLength)
        deltaRdeltaGamma = thisIo / rPlaneDefectN * (nNodesY - 1) / (q * rNi ^ 2 * deviceLength) * (localP + localN) * Exp(voltage / (rPlaneDefectN * rVt)) / Exp(voltage / rVt)
        damping = rPlaneDamping / (1 - 4 * rDeltaGammaDeltaR * deltaRdeltaGamma)
        vPlaneRecombination(i, j, k) = vPlaneRecombination(i, j, k) + damping * (PlaneDefectRecombination - vPlaneRecombination(i, j, k))
    End If
    PlaneDefectRecombination = vPlaneRecombination(i, j, k)
End If
If planeDefectDirection = "Top" And k - 1 = Round(rDefectPosition(plane, zAxis) * (nNodesZ - 1) / 100, 0) And _
        i - 1 <= rDefectPosition(plane, xAxis) / 100 * (nNodesX - 1) And _
        nNodesY - j <= rDefectPosition(plane, yAxis) / 100 * (nNodesY - 1) Then
    localGamma = vGamma(i, j, k): localP = rEqP + localGamma: localN = rEqN + localGamma
    If k = 1 Or k = nNodesZ Then thisIo = 2 * thisIo
    If localGamma > 0 Then
        voltage = rVt * Log(localP * localN / rNi ^ 2)
        PlaneDefectRecombination = rPlaneDefectIo * (Exp(voltage / (rPlaneDefectN * rVt)) - 1) * (nNodesZ - 1) / (q * deviceHeight)
        deltaRdeltaGamma = thisIo / rPlaneDefectN * (nNodesZ - 1) / (q * rNi ^ 2 * deviceHeight) * (localP + localN) * Exp(voltage / (rPlaneDefectN * rVt)) / Exp(voltage / rVt)
        damping = rPlaneDamping / (1 - 4 * rDeltaGammaDeltaR * deltaRdeltaGamma)
        vPlaneRecombination(i, j, k) = vPlaneRecombination(i, j, k) + damping * (PlaneDefectRecombination - vPlaneRecombination(i, j, k))
    End If
    PlaneDefectRecombination = vPlaneRecombination(i, j, k)
End If
End Function

Function DeltaGammaDeltaR(ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Double
'Analytic estimate of derivative required for local Jacobian used in Refine Surfaces routine.
Const deltaRG = 1
Dim kLocal As Integer, m As Integer, n As Integer, P As Integer
Dim kmnp2 As Double
Dim deltaQgamma As Variant, vG As Variant, vC As Variant
Dim fA As Variant, fB As Variant, fAB As Variant, fC As Variant, fQ As Variant
Dim rGamma As Double, rPsiFactor As Double, rGammaFactor As Double, deltaQpsi As Double
ReDim deltaQgamma(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vG(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vC(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim fA(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fAB(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nNodesZ) As Double
ReDim fC(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
ReDim fQ(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
rPsiFactor = rVt * (rDn - rDp) / (rDn * rDp)
rGammaFactor = (rEqN - rEqP) / (2 * rVt)
deltaQpsi = (deltaRG * rPsiFactor) / (rEqN + rEqP)
deltaQgamma(i, j, k) = deltaRG / rDamb + rGammaFactor * deltaQpsi
'Calculate fQ
fQ = MTrans3Dback(MMult3D(r2fZ, MTrans3Drp(MMult3D(r2fY, MTrans3Drc(MMult3D(r2fX, deltaQgamma))))))
'Calculate fC
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For P = 0 To nHarmonicsZ
    kmnp2 = (m * Pi / deviceWidth) ^ 2 + (n * Pi / deviceLength) ^ 2 + (P * Pi / deviceHeight) ^ 2
    fC(m + 1, n + 1, P + 1) = fQ(m + 1, n + 1, P + 1) / (kmnp2 + rInvL2)
Next P: Next n: Next m
'Calculate fA and fB
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
    fA(m + 1, n + 1) = 0: fB(m + 1, n + 1) = 0
    For P = 0 To nHarmonicsZ
        fA(m + 1, n + 1) = fA(m + 1, n + 1) + fC(m + 1, n + 1, P + 1)
        fB(m + 1, n + 1) = fB(m + 1, n + 1) + fC(m + 1, n + 1, P + 1) * Cos(Pi * P)
    Next P
Next n: Next m
'Calculate real C
vC = MTrans3Dback(MMult3D(f2rZ, MTrans3Drp(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fC))))))
'Calculate real G at i,j,k
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For kLocal = 1 To nNodesZ
    fAB(m + 1, n + 1, kLocal) = fA(m + 1, n + 1) * vSinhTermsGammaA(m + 1, n + 1, kLocal) _
                  + fB(m + 1, n + 1) * vSinhTermsGammaB(m + 1, n + 1, kLocal)
Next kLocal: Next n: Next m
vG = MTrans3Drc(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fAB))))
rGamma = vG(i, j, k) - vC(i, j, k)
DeltaGammaDeltaR = rGamma
End Function

Function SiElectronDiffusivity(ByVal rN As Double) As Double
'Diffusivity model matches PC1D v5.9
Const max As Double = 1417
Const majMin As Double = 60, majNref As Double = 9.64E+16, majAlpha As Double = 0.664
Const minMin As Double = 160, minNref As Double = 5.6E+16, minAlpha As Double = 0.647
Const beta1 As Double = -0.57, beta2 As Double = -2.33, beta3 As Double = 2.4, beta4 As Double = -0.146
Dim mobility As Double, Tn As Double
Tn = rVt / rVt300
If nType Then       'Electrons are majority carriers
    mobility = majMin * Tn ^ beta1 + (max - majMin) * Tn ^ beta2 / (1 + (Abs(rN) / (majNref * Tn ^ beta3)) ^ (majAlpha * Tn ^ beta4))
Else                'Electrons are minority carriers
    mobility = minMin * Tn ^ beta1 + (max - minMin) * Tn ^ beta2 / (1 + (Abs(rN) / (minNref * Tn ^ beta3)) ^ (minAlpha * Tn ^ beta4))
End If
SiElectronDiffusivity = rVt * mobility
End Function

Function SiHoleDiffusivity(ByVal rN As Double) As Double
'Diffusivity model matches PC1D v5.9
Const max As Double = 470
Const majMin As Double = 37.4, majNref As Double = 2.82E+17, majAlpha As Double = 0.642
Const minMin As Double = 155, minNref As Double = 1E+17, minAlpha As Double = 0.9
Const beta1 As Double = -0.57, beta2 As Double = -2.23, beta3 As Double = 2.4, beta4 As Double = -0.146
Dim mobility As Double, Tn As Double
Tn = rVt / rVt300
If pType Then       'Holes are majority carriers
    mobility = majMin * Tn ^ beta1 + (max - majMin) * Tn ^ beta2 / (1 + (Abs(rN) / (majNref * Tn ^ beta3)) ^ (majAlpha * Tn ^ beta4))
Else                'Holes are minority carriers
    mobility = minMin * Tn ^ beta1 + (max - minMin) * Tn ^ beta2 / (1 + (Abs(rN) / (minNref * Tn ^ beta3)) ^ (minAlpha * Tn ^ beta4))
End If
SiHoleDiffusivity = rVt * mobility
End Function

Function IDelement(ByVal rPosition As Double, _
                   ByRef rBoundaries As Variant) As Integer
'Returns the surface element number at the given node location (rPosition) based on upper element boundaries in 2D rBoundaries.
'Nodes that lie exactly on a boundary are assigned to the lower-numbered element.
Dim iElement As Integer, nElements As Integer
nElements = UBound(rBoundaries)
IDelement = 1
For iElement = 1 To nElements - 1
    If rPosition > rBoundaries(iElement) Then IDelement = iElement + 1
Next iElement
End Function

Function LateralNodeFlowIn(ByVal nSurface As Integer, ByVal nCarrier As Integer, i As Integer, j As Integer) As Double
'Returns lateral flux of the designated carrier into the region surrounding the designated node.
'Expressed per unit of nodal area = dx x dy, i.e. cm-2/s.
'Note that sQuadSigma is negative for electron conductance, which provides the needed flux polarity inversion.
'Works in 2D only if sPhi is uniform in y axis.
Dim iLeft As Integer, iRight As Integer, jTop As Integer, jBot As Integer
Dim dx As Double, dy As Double
dx = deviceWidth / (nNodesX - 1)
dy = deviceLength / (nNodesY - 1)
If i = 1 Then iLeft = i + 1 Else iLeft = i - 1
If i = nNodesX Then iRight = i - 1 Else iRight = i + 1
LateralNodeFlowIn = 0
If j = 1 Then jTop = j + 1 Else jTop = j - 1
If j = nNodesY Then jBot = j - 1 Else jBot = j + 1
If sQuadSigma(nSurface, nCarrier, i, j, TR) <> 0 Then
    LateralNodeFlowIn = LateralNodeFlowIn + sQuadSigma(nSurface, nCarrier, i, j, TR) * dx / dy _
        * (3 * (sPhi(nSurface, nCarrier, i, jTop) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iRight, jTop) - sPhi(nSurface, nCarrier, iRight, j))) / 8
    LateralNodeFlowIn = LateralNodeFlowIn + sQuadSigma(nSurface, nCarrier, i, j, TR) * dy / dx _
        * (3 * (sPhi(nSurface, nCarrier, iRight, j) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iRight, jTop) - sPhi(nSurface, nCarrier, i, jTop))) / 8
End If
If sQuadSigma(nSurface, nCarrier, i, j, TL) <> 0 Then
    LateralNodeFlowIn = LateralNodeFlowIn + sQuadSigma(nSurface, nCarrier, i, j, TL) * dx / dy _
        * (3 * (sPhi(nSurface, nCarrier, i, jTop) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iLeft, jTop) - sPhi(nSurface, nCarrier, iLeft, j))) / 8
    LateralNodeFlowIn = LateralNodeFlowIn + sQuadSigma(nSurface, nCarrier, i, j, TL) * dy / dx _
        * (3 * (sPhi(nSurface, nCarrier, iLeft, j) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iLeft, jTop) - sPhi(nSurface, nCarrier, i, jTop))) / 8
End If
If sQuadSigma(nSurface, nCarrier, i, j, BL) <> 0 Then
    LateralNodeFlowIn = LateralNodeFlowIn + sQuadSigma(nSurface, nCarrier, i, j, BL) * dx / dy _
        * (3 * (sPhi(nSurface, nCarrier, i, jBot) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iLeft, jBot) - sPhi(nSurface, nCarrier, iLeft, j))) / 8
    LateralNodeFlowIn = LateralNodeFlowIn + sQuadSigma(nSurface, nCarrier, i, j, BL) * dy / dx _
        * (3 * (sPhi(nSurface, nCarrier, iLeft, j) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iLeft, jBot) - sPhi(nSurface, nCarrier, i, jBot))) / 8
End If
If sQuadSigma(nSurface, nCarrier, i, j, BR) <> 0 Then
    LateralNodeFlowIn = LateralNodeFlowIn + sQuadSigma(nSurface, nCarrier, i, j, BR) * dx / dy _
        * (3 * (sPhi(nSurface, nCarrier, i, jBot) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iRight, jBot) - sPhi(nSurface, nCarrier, iRight, j))) / 8
    LateralNodeFlowIn = LateralNodeFlowIn + sQuadSigma(nSurface, nCarrier, i, j, BR) * dy / dx _
        * (3 * (sPhi(nSurface, nCarrier, iRight, j) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iRight, jBot) - sPhi(nSurface, nCarrier, i, jBot))) / 8
End If
LateralNodeFlowIn = LateralNodeFlowIn / (q * dx * dy)   'Convert from amps to cm-2/s
End Function

Function LateralNodeCurrentX(ByVal nSurface As Integer, i As Integer, j As Integer) As Double
'Returns lateral current (A/cm) in the x-direction at the designated node.
'Note that sQuadSigma is negative for electron conductance, so Abs value is necessary.
Dim iLeft As Integer, iRight As Integer, jTop As Integer, jBot As Integer, nCarrier As Integer
Dim dx As Double, dy As Double
dx = deviceWidth / (nNodesX - 1)
dy = deviceLength / (nNodesY - 1)
If i = 1 Then iLeft = 2 Else iLeft = i - 1
If i = nNodesX Then iRight = nNodesX - 1 Else iRight = i + 1
If j = 1 Then jTop = 2 Else jTop = j - 1
If j = nNodesY Then jBot = nNodesY - 1 Else jBot = j + 1
LateralNodeCurrentX = 0
For nCarrier = holes To electrons
    If sQuadSigma(nSurface, nCarrier, i, j, TR) <> 0 Then
        LateralNodeCurrentX = LateralNodeCurrentX - 0.5 * Abs(sQuadSigma(nSurface, nCarrier, i, j, TR) / dx) _
            * (3 * (sPhi(nSurface, nCarrier, iRight, j) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iRight, jTop) - sPhi(nSurface, nCarrier, i, jTop))) / 8
    End If
    If sQuadSigma(nSurface, nCarrier, i, j, TL) <> 0 Then
        LateralNodeCurrentX = LateralNodeCurrentX + 0.5 * Abs(sQuadSigma(nSurface, nCarrier, i, j, TL) / dx) _
            * (3 * (sPhi(nSurface, nCarrier, iLeft, j) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iLeft, jTop) - sPhi(nSurface, nCarrier, i, jTop))) / 8
    End If
    If sQuadSigma(nSurface, nCarrier, i, j, BL) <> 0 Then
        LateralNodeCurrentX = LateralNodeCurrentX + 0.5 * Abs(sQuadSigma(nSurface, nCarrier, i, j, BL) / dx) _
            * (3 * (sPhi(nSurface, nCarrier, iLeft, j) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iLeft, jBot) - sPhi(nSurface, nCarrier, i, jBot))) / 8
    End If
    If sQuadSigma(nSurface, nCarrier, i, j, BR) <> 0 Then
        LateralNodeCurrentX = LateralNodeCurrentX - 0.5 * Abs(sQuadSigma(nSurface, nCarrier, i, j, BR) / dx) _
            * (3 * (sPhi(nSurface, nCarrier, iRight, j) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iRight, jBot) - sPhi(nSurface, nCarrier, i, jBot))) / 8
    End If
Next nCarrier
End Function

Function LateralNodeCurrentY(ByVal nSurface As Integer, i As Integer, j As Integer) As Double
'Returns lateral current (A/cm) in the y-direction at the designated node.
'Note that sQuadSigma is negative for electron conductance, so Abs value is necessary.
'Returns zero for 2D.
Dim iLeft As Integer, iRight As Integer, jTop As Integer, jBot As Integer, nCarrier As Integer
Dim dx As Double, dy As Double
LateralNodeCurrentY = 0
If Not b3D Then Exit Function
dx = deviceWidth / (nNodesX - 1)
dy = deviceLength / (nNodesY - 1)
If i = 1 Then iLeft = 2 Else iLeft = i - 1
If i = nNodesX Then iRight = nNodesX - 1 Else iRight = i + 1
If j = 1 Then jTop = 2 Else jTop = j - 1
If j = nNodesY Then jBot = nNodesY - 1 Else jBot = j + 1
For nCarrier = holes To electrons
    If sQuadSigma(nSurface, nCarrier, i, j, TR) <> 0 Then
        LateralNodeCurrentY = LateralNodeCurrentY + 0.5 * Abs(sQuadSigma(nSurface, nCarrier, i, j, TR) / dy) _
            * (3 * (sPhi(nSurface, nCarrier, i, jTop) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iRight, jTop) - sPhi(nSurface, nCarrier, iRight, j))) / 8
    End If
    If sQuadSigma(nSurface, nCarrier, i, j, TL) <> 0 Then
        LateralNodeCurrentY = LateralNodeCurrentY + 0.5 * Abs(sQuadSigma(nSurface, nCarrier, i, j, TL) / dy) _
            * (3 * (sPhi(nSurface, nCarrier, i, jTop) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iLeft, jTop) - sPhi(nSurface, nCarrier, iLeft, j))) / 8
    End If
    If sQuadSigma(nSurface, nCarrier, i, j, BL) <> 0 Then
        LateralNodeCurrentY = LateralNodeCurrentY - 0.5 * Abs(sQuadSigma(nSurface, nCarrier, i, j, BL) / dy) _
            * (3 * (sPhi(nSurface, nCarrier, i, jBot) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iLeft, jBot) - sPhi(nSurface, nCarrier, iLeft, j))) / 8
    End If
    If sQuadSigma(nSurface, nCarrier, i, j, BR) <> 0 Then
        LateralNodeCurrentY = LateralNodeCurrentY - 0.5 * Abs(sQuadSigma(nSurface, nCarrier, i, j, BR) / dy) _
            * (3 * (sPhi(nSurface, nCarrier, i, jBot) - sPhi(nSurface, nCarrier, i, j)) + (sPhi(nSurface, nCarrier, iRight, jBot) - sPhi(nSurface, nCarrier, iRight, j))) / 8
    End If
Next nCarrier
End Function

Function ParabolicFit(ByVal x1 As Double, ByVal x2 As Double, ByVal x3 As Double, ByVal y1 As Double, ByVal y2 As Double, ByVal y3 As Double, _
                                    ByRef a As Double, ByRef b As Double, ByRef c As Double) As Boolean
Dim denominator As Double
ParabolicFit = False
denominator = (x1 ^ 2 - x2 ^ 2) * (x2 - x3) - (x2 ^ 2 - x3 ^ 2) * (x1 - x2)
If denominator <> 0 Then a = ((x2 - x3) * (y1 - y2) - (x1 - x2) * (y2 - y3)) / denominator Else Exit Function
If x1 <> x2 Then b = ((y1 - y2) - a * (x1 ^ 2 - x2 ^ 2)) / (x1 - x2) Else Exit Function
c = y1 - b * x1 - a * x1 ^ 2
ParabolicFit = a <> 0
End Function

Function FindPmax(ByRef dataIV As Variant, ByRef Vmax As Double, ByRef Imax As Double, ByRef Pmax As Double) As Boolean
'dataIV starts with index=2 with increasing terminal voltage (index=1 stores the max-power point).
'Fields are 1=terminal voltage, 2=terminal current, 3=terminal power.
'This function enters the max-power values into the first row and returns true if successful.
Dim firstRow As Integer, lastRow As Integer, iRow As Integer, jRow As Integer, nPoints As Integer, maxRow As Integer, nCounter As Integer
Dim Pmid As Double, Pplus As Double, Pminus As Double, Ptrial As Double
Dim Vmid As Double, Vplus As Double, Vminus As Double
Dim Imid As Double, Iplus As Double, Iminus As Double
Dim slopeMinus As Double, slopeMid As Double, slopePlus As Double, ratiomax As Double
Dim a As Double, b As Double, c As Double
Dim cleanDataIV As Variant
FindPmax = False
cleanDataIV = dataIV
'Remove empty rows
For iRow = 2 To UBound(cleanDataIV, 1)
    nCounter = 0
    Do While cleanDataIV(iRow, 2) = 0 And nCounter < UBound(cleanDataIV, 1)
        For jRow = iRow To UBound(cleanDataIV, 1) - 1
            cleanDataIV(jRow, 1) = cleanDataIV(jRow + 1, 1)
            cleanDataIV(jRow, 2) = cleanDataIV(jRow + 1, 2)
            cleanDataIV(jRow, 3) = cleanDataIV(jRow + 1, 3)
        Next jRow
        nCounter = nCounter + 1
    Loop
Next iRow
'Find first row (should be row 2)
firstRow = 0
For iRow = UBound(cleanDataIV, 1) To 2 Step -1
    If cleanDataIV(iRow, 1) <> 0 And cleanDataIV(iRow, 2) <> 0 Then firstRow = iRow
Next iRow
'Find last row
lastRow = 0
For iRow = 2 To UBound(cleanDataIV, 1)
    If cleanDataIV(iRow, 1) <> 0 And cleanDataIV(iRow, 2) <> 0 Then lastRow = iRow
Next iRow
nPoints = lastRow - firstRow + 1
'Find the data point with the highest power
maxRow = 0: Pmax = 0
For iRow = firstRow To lastRow
    If cleanDataIV(iRow, 3) > Pmax Then
      maxRow = iRow
      Vmax = cleanDataIV(iRow, 1)
      Imax = cleanDataIV(iRow, 2)
      Pmax = cleanDataIV(iRow, 3)
    End If
Next iRow
If maxRow = 0 Or nPoints < 3 Then Exit Function    'If no points with positive power, or too few points, then return false
'Set up for parabolic fit
If maxRow = firstRow Then maxRow = firstRow + 1
If maxRow = lastRow Then maxRow = lastRow - 1
Vmid = cleanDataIV(maxRow, 1)
Vplus = cleanDataIV(maxRow + 1, 1)
Vminus = cleanDataIV(maxRow - 1, 1)
Imid = cleanDataIV(maxRow, 2)
Iplus = cleanDataIV(maxRow + 1, 2)
Iminus = cleanDataIV(maxRow - 1, 2)
Pmid = cleanDataIV(maxRow, 3)
Pplus = cleanDataIV(maxRow + 1, 3)
Pminus = cleanDataIV(maxRow - 1, 3)
If Pminus < Pmax / 2 Or Pplus < Pmax / 2 Then Exit Function  'Don't return Pmax if side values are small compared to max value
'Get conductance slope at each fitting point (these are the abscissa for parabolic interpolation of power)
slopeMinus = Iminus / Vminus
slopeMid = Imid / Vmid
slopePlus = Iplus / Vplus
If ParabolicFit(slopeMinus, slopeMid, slopePlus, Pminus, Pmid, Pplus, a, b, c) Then
    ratiomax = -b / (2 * a)
    Ptrial = c - b ^ 2 / (4 * a)
    If ratiomax > 0 And Ptrial > Pmax Then
        Pmax = Ptrial
        Vmax = Sqr(Pmax / ratiomax)
        Imax = Pmax / Vmax
        FindPmax = True
    End If
End If
If Vmax < Vminus Or Vmax > Vplus Then       'Don't report Pmax as found if it's outside the parabolic fit interval
    FindPmax = False
    Exit Function
End If
'Load Pmax data into spreadsheet (for plotting on Dashboard)
dataIV(1, 1) = Vmax
dataIV(1, 2) = Imax
dataIV(1, 3) = Pmax
End Function

Function FindVoc(ByRef dataIV As Variant, ByRef Voc As Double) As Boolean
'Finds and returns Voc, provided the cross-over is found and is not too far away from the closest data points.
Dim V1 As Double, V2 As Double, V3 As Double
Dim I1 As Double, I2 As Double, I3 As Double
Dim denominator As Double, a As Double, b As Double, c As Double
Dim nPoints As Integer, nCounter As Integer
Dim firstRow As Integer, lastRow As Integer, iRow As Integer, jRow As Integer, rowMid As Integer, rowMinus As Integer, rowPlus As Integer
Dim cleanDataIV As Variant
Voc = 0
FindVoc = False
cleanDataIV = dataIV
'Remove empty rows
For iRow = 2 To UBound(cleanDataIV, 1)
    nCounter = 0
    Do While cleanDataIV(iRow, 2) = 0 And nCounter < UBound(cleanDataIV, 1)
        For jRow = iRow To UBound(cleanDataIV, 1) - 1
            cleanDataIV(jRow, 1) = cleanDataIV(jRow + 1, 1)
            cleanDataIV(jRow, 2) = cleanDataIV(jRow + 1, 2)
            cleanDataIV(jRow, 3) = cleanDataIV(jRow + 1, 3)
        Next jRow
        nCounter = nCounter + 1
    Loop
Next iRow
'Find first row (should be 2)
firstRow = 0
For iRow = UBound(cleanDataIV, 1) To 1 Step -1
    If cleanDataIV(iRow, 1) <> 0 And cleanDataIV(iRow, 2) <> 0 Then firstRow = iRow
Next iRow
'Find last row
lastRow = 0
For iRow = 1 To UBound(cleanDataIV, 1)
    If cleanDataIV(iRow, 1) <> 0 And cleanDataIV(iRow, 2) <> 0 Then lastRow = iRow
Next iRow
nPoints = lastRow - firstRow + 1

If nPoints < 2 Then Exit Function 'Need at least two data points

If nPoints = 2 Then   'Only two data points, use linear interpolation/extrapolation
    V1 = cleanDataIV(firstRow, 1)
    V2 = cleanDataIV(lastRow, 1)
    I1 = cleanDataIV(firstRow, 2)
    I2 = cleanDataIV(lastRow, 2)
    If (I2 - I1) < 0 Then Voc = (V1 * I2 - V2 * I1) / (I2 - I1) Else Exit Function
    If Abs(Voc - (V2 + V1) / 2) > (V2 - V1) Then Exit Function
    FindVoc = True: Exit Function
End If

'With at least three data points, use parabolic interpolation/extrapolation.
'Find data point that is closest to the cross-over, but with at least one data point on each side.
rowMid = 0
For iRow = firstRow To lastRow
    If cleanDataIV(iRow, 2) <> 0 Then
        I2 = cleanDataIV(iRow, 2): rowMid = iRow: Exit For
    End If
Next iRow
For iRow = firstRow + 1 To lastRow
    If Abs(cleanDataIV(iRow, 2)) < I2 And cleanDataIV(iRow, 2) <> 0 Then
        I2 = Abs(cleanDataIV(iRow, 2)): rowMid = iRow
    End If
Next iRow
If rowMid = 0 Then Exit Function
If rowMid = firstRow Then rowMid = firstRow + 1
If rowMid = lastRow Then rowMid = lastRow - 1
rowPlus = rowMid + 1: rowMinus = rowMid - 1
V1 = cleanDataIV(rowMinus, 1): V2 = cleanDataIV(rowMid, 1): V3 = cleanDataIV(rowPlus, 1)
I1 = cleanDataIV(rowMinus, 2): I2 = cleanDataIV(rowMid, 2): I3 = cleanDataIV(rowPlus, 2)
If ParabolicFit(V1, V2, V3, I1, I2, I3, a, b, c) Then
    If a <> 0 Then Voc = (-b - Sqr(b ^ 2 - 4 * a * c)) / (2 * a) Else Exit Function
    If Abs(Voc - V2) > (V3 - V1) Then Exit Function
    FindVoc = True
End If
End Function

Function FindIsc(ByRef dataIV As Variant, ByRef Isc As Double) As Boolean
Dim Vminus As Double, Vplus As Double
Dim Iminus As Double, Iplus As Double
Dim iRow As Integer, jRow As Integer, rowMinus As Integer, rowPlus As Integer, nCounter As Integer
Dim cleanDataIV As Variant
cleanDataIV = dataIV
'Remove empty rows
For iRow = 2 To UBound(cleanDataIV, 1)
    nCounter = 0
    Do While cleanDataIV(iRow, 2) = 0 And nCounter < UBound(cleanDataIV, 1)
        For jRow = iRow To UBound(cleanDataIV, 1) - 1
            cleanDataIV(jRow, 1) = cleanDataIV(jRow + 1, 1)
            cleanDataIV(jRow, 2) = cleanDataIV(jRow + 1, 2)
            cleanDataIV(jRow, 3) = cleanDataIV(jRow + 1, 3)
        Next jRow
        nCounter = nCounter + 1
    Loop
Next iRow
    
FindIsc = False
rowMinus = UBound(dataIV, 1)
rowPlus = 2
Isc = 0

For iRow = 2 To UBound(dataIV, 1)
    If (dataIV(iRow, 1) <= 0) And (dataIV(iRow, 2) <> 0) Then rowMinus = iRow
Next iRow

For iRow = UBound(dataIV, 1) To 2 Step -1
    If (dataIV(iRow, 1) > 0) And (dataIV(iRow, 2) <> 0) Then rowPlus = iRow
Next iRow

If Not (rowPlus > rowMinus) Then Exit Function

Vminus = dataIV(rowMinus, 1)
Iminus = dataIV(rowMinus, 2)
Vplus = dataIV(rowPlus, 1)
Iplus = dataIV(rowPlus, 2)
If (Vplus - Vminus) > 0 Then Isc = (Vplus * Iminus - Vminus * Iplus) / (Vplus - Vminus) Else Exit Function
FindIsc = True
End Function

Function TwoDiodeFit(ByVal bIncludeJL As Boolean, ByVal bIncludeGsh As Boolean, ByVal Rs As Double, _
                    ByRef JL As Double, ByRef Jo1 As Double, ByRef Jo2 As Double, ByRef Gsh As Double) As Double
'Returns RMS error for best-fit of dataIV to a two-diode model assuming a value of Rs as given (returns 0 if no fit obtained).
'If NOT bIncludeJL, then the data points are weighted by inverse current squared, to simulate a best-fit on a logarithmic scale
'Returns values for JL, Jo1, Jo2, and Gsh (returns 0 for values not included in the fit)
Dim i As Integer, n As Integer
Dim Vj As Double, Ji As Double, SumSquaredError As Double, weight As Double
Dim x As Variant, y As Variant, m As Variant, V As Variant, f As Variant
Dim dataIV As Variant
dataIV = Range(IVrange)
TwoDiodeFit = 0
    ReDim x(0 To 8) As Double
    ReDim y(0 To 3) As Double
    For i = 1 To UBound(dataIV, 1)
        If dataIV(i, 1) <> "" Then
            Ji = dataIV(i, 2)
            If Not bIncludeJL Then Ji = -Ji
            Vj = dataIV(i, 1) + Ji * Rs
            n = n + 1
            If bIncludeJL Then weight = 1 Else weight = 1 / Ji ^ 2
            x(0) = x(0) + weight
            x(1) = x(1) + weight * (Exp(Vj / rVt) - 1)
            x(2) = x(2) + weight * (Exp(Vj / (2 * rVt)) - 1)
            x(3) = x(3) + weight * Vj
            x(4) = x(4) + weight * (Exp(Vj / rVt) - 1) ^ 2
            x(5) = x(5) + weight * ((Exp(Vj / (2 * rVt)) - 1) * (Exp(Vj / rVt) - 1))
            x(6) = x(6) + weight * Vj * (Exp(Vj / rVt) - 1)
            x(7) = x(7) + weight * Vj * (Exp(Vj / (2 * rVt)) - 1)
            x(8) = x(8) + weight * Vj ^ 2
            y(0) = y(0) + weight * Ji
            y(1) = y(1) + weight * Ji * (Exp(Vj / rVt) - 1)
            y(2) = y(2) + weight * Ji * (Exp(Vj / (2 * rVt)) - 1)
            y(3) = y(3) + weight * Ji * Vj
        End If
    Next i
    If n < 5 Then Exit Function
    If bIncludeJL And bIncludeGsh Then
        ReDim m(1 To 4, 1 To 4) As Double
        ReDim V(1 To 4, 1 To 1) As Double
        ReDim f(1 To 4, 1 To 1) As Double
        m(1, 1) = -x(0): m(1, 2) = x(1): m(1, 3) = x(2): m(1, 4) = x(3)
        m(2, 1) = -x(1): m(2, 2) = x(4): m(2, 3) = x(5): m(2, 4) = x(6)
        m(3, 1) = -x(2): m(3, 2) = x(5): m(3, 3) = x(1): m(3, 4) = x(7)
        m(4, 1) = -x(3): m(4, 2) = x(6): m(4, 3) = x(7): m(4, 4) = x(8)
        V(1, 1) = -y(0): V(2, 1) = -y(1): V(3, 1) = -y(2): V(4, 1) = -y(3)
        f = Application.MMult(Application.MInverse(m), V)
        JL = f(1, 1): Jo1 = f(2, 1): Jo2 = f(3, 1): Gsh = f(4, 1)
        End If
    If bIncludeJL And Not bIncludeGsh Then
        ReDim m(1 To 3, 1 To 3) As Double
        ReDim V(1 To 3, 1 To 1) As Double
        ReDim f(1 To 3, 1 To 1) As Double
        m(1, 1) = -x(0): m(1, 2) = x(1): m(1, 3) = x(2)
        m(2, 1) = -x(1): m(2, 2) = x(4): m(2, 3) = x(5)
        m(3, 1) = -x(2): m(3, 2) = x(5): m(3, 3) = x(1)
        V(1, 1) = -y(0): V(2, 1) = -y(1): V(3, 1) = -y(2)
        f = Application.MMult(Application.MInverse(m), V)
        JL = f(1, 1): Jo1 = f(2, 1): Jo2 = f(3, 1): Gsh = 0
        End If
    If Not bIncludeJL And bIncludeGsh Then
        ReDim m(1 To 3, 1 To 3) As Double
        ReDim V(1 To 3, 1 To 1) As Double
        ReDim f(1 To 3, 1 To 1) As Double
        m(1, 1) = x(4): m(1, 2) = x(5): m(1, 3) = x(6)
        m(2, 1) = x(5): m(2, 2) = x(1): m(2, 3) = x(7)
        m(3, 1) = x(6): m(3, 2) = x(7): m(3, 3) = x(8)
        V(1, 1) = -y(1): V(2, 1) = -y(2): V(3, 1) = -y(3)
        f = Application.MMult(Application.MInverse(m), V)
        JL = 0: Jo1 = f(1, 1): Jo2 = f(2, 1): Gsh = f(3, 1)
        End If
    If Not bIncludeJL And Not bIncludeGsh Then
        ReDim m(1 To 2, 1 To 2) As Double
        ReDim V(1 To 2, 1 To 1) As Double
        ReDim f(1 To 2, 1 To 1) As Double
        m(1, 1) = x(4): m(1, 2) = x(5)
        m(2, 1) = x(5): m(2, 2) = x(1)
        V(1, 1) = -y(1): V(2, 1) = -y(2)
        f = Application.MMult(Application.MInverse(m), V)
        JL = 0: Jo1 = f(1, 1): Jo2 = f(2, 1): Gsh = 0
        End If
    SumSquaredError = 0
    For i = 1 To UBound(dataIV, 1)
        If dataIV(i, 1) <> "" Then
            Ji = dataIV(i, 2)
            If Not bIncludeJL Then Ji = -Ji
            Vj = dataIV(i, 1) + Ji * Rs
            If bIncludeJL Then weight = 1 Else weight = 1 / Ji ^ 2
            SumSquaredError = SumSquaredError + weight * (Ji - (JL - Jo1 * (Exp(Vj / rVt) - 1) - Jo2 * (Exp(Vj / (2 * rVt)) - 1) - Gsh * Vj)) ^ 2
        End If
    Next i
    TwoDiodeFit = Sqr(SumSquaredError / x(0))
End Function

Function TwoDiodePmax(ByVal JL As Double, ByVal Jo1 As Double, ByVal Jo2 As Double, ByVal Gsh As Double, ByVal Rs As Double) As Double
Dim Vj As Double, rV As Double, rJ As Double
Dim Vminus As Double, Vmid As Double, Vplus As Double
Dim Pminus As Double, Pmid As Double, Pplus As Double
Dim Xminus As Double, Xmid As Double, Xplus As Double
Dim a As Double, b As Double, c As Double
TwoDiodePmax = 0
If JL <= 0 Or Jo1 <= 0 Then Exit Function
Do
    Vj = Vj + 0.01
    rJ = JL - (Jo1 * (Exp(Vj / rVt) - 1) + Jo2 * (Exp(Vj / (2 * rVt)) - 1) + Gsh * Vj)
    rV = Vj - rJ * Rs
    Vminus = Vmid: Vmid = Vplus: Vplus = rV
    Pminus = Pmid: Pmid = Pplus: Pplus = rV * rJ
Loop Until Vj > rVt * Log(JL / Jo1) Or (Pminus > 0 And Pplus < Pmid)
If Pminus > 0 And Pplus < Pmid Then
    Xminus = Pminus / Vminus ^ 2: Xmid = Pmid / Vmid ^ 2: Xplus = Pplus / Vplus ^ 2
    If ParabolicFit(Xminus, Xmid, Xplus, Pminus, Pmid, Pplus, a, b, c) Then TwoDiodePmax = c - b ^ 2 / (4 * a)
End If
End Function

Function TwoDiodeVoc(ByVal JL As Double, ByVal Jo1 As Double, ByVal Jo2 As Double, ByVal Gsh As Double) As Double
Dim rV As Double, rJ As Double
Dim Vminus As Double, Vmid As Double, Vplus As Double
Dim Jminus As Double, Jmid As Double, Jplus As Double
Dim a As Double, b As Double, c As Double
TwoDiodeVoc = 0
If JL <= 0 Or Jo1 <= 0 Then Exit Function
Do
    rV = rV + 0.01
    rJ = JL - (Jo1 * (Exp(rV / rVt) - 1) + Jo2 * (Exp(rV / (2 * rVt)) - 1) + Gsh * rV)
    Vminus = Vmid: Vmid = Vplus: Vplus = rV
    Jminus = Jmid: Jmid = Jplus: Jplus = rJ
Loop Until rV > rVt * Log(JL / Jo1) Or (Jplus < 0)
If ParabolicFit(Vminus, Vmid, Vplus, Jminus, Jmid, Jplus, a, b, c) Then
    If a <> 0 Then TwoDiodeVoc = (-b - Sqr(b ^ 2 - 4 * a * c)) / (2 * a) Else TwoDiodeVoc = -c / b
End If
End Function

Function Requires3D() As Boolean
'Returns true if the current device specfication, defects, or illumination have y-axis dependence.
'Bulk defects are ignored if not enabled.
'Illmination non-uniformity is ignored if Spectrum=None is selected (i.e. neither Spectrum nor Mono).
Requires3D = True
If bEnable3D Then
    If Not UniformRows(rFrontContacts) Then Exit Function
    If Not UniformRows(rRearContacts) Then Exit Function
    If Not UniformRows(rFrontSheetRho) Then Exit Function
    If Not UniformRows(rRearSheetRho) Then Exit Function
    If Not UniformRows(rFrontTrans) Then Exit Function
    If Not UniformRows(rRearTrans) Then Exit Function
    If Not UniformRows(rFrontHaze) Then Exit Function
    If Not UniformRows(rRearHaze) Then Exit Function
    If Not UniformRows(rFrontSpecRef) Then Exit Function
    If Not UniformRows(rRearSpecRef) Then Exit Function
    If Not UniformRows(rFrontDiffRef) Then Exit Function
    If Not UniformRows(rRearDiffRef) Then Exit Function
    If Not UniformRows(rFrontJo1) Then Exit Function
    If Not UniformRows(rRearJo1) Then Exit Function
    If Not UniformRows(rFrontJo2) Then Exit Function
    If Not UniformRows(rRearJo2) Then Exit Function
    If Not UniformRows(rFrontGsh) Then Exit Function
    If Not UniformRows(rRearGsh) Then Exit Function
End If
If bSpectrum Or bMono Then
    If frontShape = "Point" Then Exit Function
    If frontShape = "Line" And frontLineDirection <> "Face" Then Exit Function
    If rearShape = "Point" Then Exit Function
    If rearShape = "Line" And rearLineDirection <> "Face" Then Exit Function
End If
If bEnablePointDefect Or _
  (bEnableLineDefect And (lineDefectDirection <> "Face" Or rDefectPosition(line, yAxis) < 100)) Or _
  (bEnablePlaneDefect And (planeDefectDirection = "Face" Or rDefectPosition(plane, yAxis) < 100)) Then _
    Exit Function
Requires3D = False
End Function

Function MinHarmonics(ByVal nAxis As Integer) As Integer
'Returns 10, 20 or 40 for x or y axis based on surface layout dimensions, illlumination, defects, and diffusion length
'Returns 20 or 40 for z axis based on device dimensions and x and y harmonics
Dim i As Integer, j As Integer, nHarmonics As Integer
Dim deltaX As Double, deltaY As Double, deltaZ As Double, minDelta As Double
Dim columnWidth As Variant, columnUnits As Variant
Dim rowLength As Variant, rowUnits As Variant
If pType Then minDelta = Sqr(rDn * rTauN) Else minDelta = Sqr(rDp * rTauP)
If nAxis = xAxis Then
    MinHarmonics = 10
    columnWidth = Range(frontColumnWidthCells)
    columnUnits = Range(frontWidthUnitCells)
    For i = 1 To nColumns
        If columnUnits(1, i) = "%" Then
            deltaX = columnWidth(1, i) * deviceWidth / 100
        Else
            deltaX = columnWidth(1, i) / 10
        End If
        If i = 1 Or i = nColumns Then deltaX = 2 * deltaX       'Edge symmetry
        If deltaX < minDelta Then minDelta = deltaX
    Next i
    columnWidth = Range(rearColumnWidthCells)
    columnUnits = Range(rearWidthUnitCells)
    For i = 1 To nColumns
        If columnUnits(1, i) = "%" Then
            deltaX = columnWidth(1, i) * deviceWidth / 100
        Else
            deltaX = columnWidth(1, i) / 10
        End If
        If i = 1 Or i = nColumns Then deltaX = 2 * deltaX       'Edge symmetry
        If deltaX < minDelta Then minDelta = deltaX
    Next i
    If (bSpectrum And rFrontSuns > 0) Or (bMono And rFrontMonoPower > 0) Then
        If frontShape = "Point" And rFrontPointDiameter / 10 < minDelta Then minDelta = rFrontPointDiameter / 10
        If rearShape = "Point" And rRearPointDiameter / 10 < minDelta Then minDelta = rRearPointDiameter / 10
    End If
    If (bSpectrum And rRearSuns > 0) Or (bMono And rRearMonoPower > 0) Then
        If frontShape = "Line" And frontLineDirection <> "Side" And rFrontLineWidth / 10 < minDelta Then minDelta = rFrontLineWidth / 10
        If rearShape = "Line" And rearLineDirection <> "Side" And rRearLineWidth / 10 < minDelta Then minDelta = rRearLineWidth / 10
    End If
    nHarmonics = Round(deviceWidth / minDelta, 1)
    'Test conditions that require at least Res20
    If Not b3D Then MinHarmonics = 20
    If nHarmonics > 10 Then MinHarmonics = 20
    If bEnablePointDefect Or (bEnableLineDefect And lineDefectDirection <> "Side") Then MinHarmonics = 20
    'Test conditions that require Res40
    If nHarmonics > 25 Then MinHarmonics = 40
End If
If nAxis = yAxis Then
    MinHarmonics = 10
    rowLength = Range(frontRowLengthCells)
    rowUnits = Range(frontLengthUnitCells)
    For j = 1 To nRows
        If rowUnits(j, 1) = "%" Then
            deltaY = rowLength(j, 1) * deviceLength / 100
        Else
            deltaY = rowLength(j, 1) / 10
        End If
        If j = 1 Or j = nRows Then deltaY = 2 * deltaY       'Edge symmetry
        If deltaY < minDelta Then minDelta = deltaY
    Next j
    rowLength = Range(rearRowLengthCells)
    rowUnits = Range(rearLengthUnitCells)
    For j = 1 To nRows
        If rowUnits(j, 1) = "%" Then
            deltaY = rowLength(j, 1) * deviceLength / 100
        Else
            deltaY = rowLength(j, 1) / 10
        End If
        If j = 1 Or j = nRows Then deltaY = 2 * deltaY       'Edge symmetry
        If deltaY < minDelta Then minDelta = deltaY
    Next j
    If (bSpectrum And rFrontSuns > 0) Or (bMono And rFrontMonoPower > 0) Then
        If frontShape = "Point" And rFrontPointDiameter / 10 < minDelta Then minDelta = rFrontPointDiameter / 10
        If rearShape = "Point" And rRearPointDiameter / 10 < minDelta Then minDelta = rRearPointDiameter / 10
    End If
    If (bSpectrum And rRearSuns > 0) Or (bMono And rRearMonoPower > 0) Then
        If frontShape = "Line" And frontLineDirection <> "Face" And rFrontLineWidth / 10 < minDelta Then minDelta = rFrontLineWidth / 10
        If rearShape = "Line" And rearLineDirection <> "Face" And rRearLineWidth / 10 < minDelta Then minDelta = rRearLineWidth / 10
    End If
    nHarmonics = Round(deviceLength / minDelta, 1)
    'Test conditions that require at least Res20
    If nHarmonics > 10 Then MinHarmonics = 20
    If bEnablePointDefect Or (bEnableLineDefect And lineDefectDirection <> "Face") Then MinHarmonics = 20
    'Test conditions that require Res40
    If nHarmonics > 25 Then MinHarmonics = 40
End If
If nAxis = zAxis Then
    MinHarmonics = 20
    deltaX = deviceWidth / nHarmonicsX
    deltaY = deviceLength / nHarmonicsY
    deltaZ = WorksheetFunction.Min(deltaX, deltaY)
    nHarmonics = WorksheetFunction.Floor(deviceHeight / deltaZ, 1)
    'Test conditions that require Res40
    If nHarmonics > 25 Then MinHarmonics = 40
End If
End Function
