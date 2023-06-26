Attribute VB_Name = "Functions"
Option Explicit

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
Dim A As Double, b As Double, C As Double
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
If denominator <> 0 Then A = ((x2 - x3) * (y1 - y2) - (x1 - x2) * (y2 - y3)) / denominator Else Exit Function
If x1 <> x2 Then b = ((y1 - y2) - A * (x1 ^ 2 - x2 ^ 2)) / (x1 - x2) Else Exit Function
C = y1 - b * x1 - A * x1 ^ 2
InterpolateColumns = A * x ^ 2 + b * x + C
If bLog Then InterpolateColumns = ExpPower(InterpolateColumns)
End Function

Function VolumeFraction(ByVal i As Integer, ByVal j As Integer, ByRef nLeft As Integer, ByRef nRight As Integer, ByRef nRear As Integer, ByRef nFront As Integer) As Double
'Returns the unfolded volume fraction not removed by the etch at the surface location (i,j)
'Also sets near-node indices equal to either i or j where that node lies within the etched void
VolumeFraction = 1 / 2
Select Case nTextureType
    Case grooves
        nRight = i
        If i = 1 Then
            nLeft = i
            VolumeFraction = 1 / 4
        End If
        If i = nNodesX Then
            nRight = nLeft
            VolumeFraction = 3 / 4
        End If
    Case pyramids
        If j >= nNodesY - i + 1 Then nRight = i
        If j <= nNodesY - i + 1 Then nRear = j
        If j = nNodesY - i + 1 Then VolumeFraction = 1 / 3
        If i = nNodesX Then VolumeFraction = 3 / 4
        If j = 1 Then VolumeFraction = 3 / 4
        If i = nNodesX And j = 1 Then VolumeFraction = 2 / 3
        If i = 1 And j = nNodesY Then
            nLeft = i: nFront = j
            VolumeFraction = 1 / 6
        End If
    Case inverted
        If j >= i Then nRight = i
        If j <= i Then nFront = j
        If j = i Then VolumeFraction = 2 / 3
        If i = 1 Then
            nLeft = i
            VolumeFraction = 1 / 4
        End If
        If j = 1 Then
            nRear = j
            VolumeFraction = 1 / 4
        End If
        If i = 1 And j = 1 Then VolumeFraction = 1 / 3
        If i = nNodesX And j = nNodesY Then VolumeFraction = 5 / 6
End Select
End Function

Function VolumeAverage(ByRef vArray As Variant) As Double
Dim i As Integer, j As Integer, k As Integer
Dim nLeft As Integer, nRight As Integer, nRear As Integer, nFront As Integer        'Dummy variables for function call
Dim localVolumeFraction As Double
'Returns the average of a 3D (volume) array of nNodesX x nNodesY x nNodesZ, omitting the void
'Only counts the portion of symmetry plane nodes that lie within the solution region
VolumeAverage = 0
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    If k = sTexture(i, j) Then localVolumeFraction = VolumeFraction(i, j, nLeft, nRight, nRear, nFront) Else localVolumeFraction = 1
    If i = 1 Or i = nNodesX Then localVolumeFraction = localVolumeFraction / 2
    If j = 1 Or j = nNodesY Then localVolumeFraction = localVolumeFraction / 2
    If k = nNodesZ Then localVolumeFraction = localVolumeFraction / 2       'Top surface fraction already accounted for
    VolumeAverage = VolumeAverage + vArray(i, j, k) * localVolumeFraction
Next k: Next j: Next i
VolumeAverage = VolumeAverage / (nNodesX - 1)
VolumeAverage = VolumeAverage / (nNodesY - 1)
VolumeAverage = VolumeAverage / (nNodesZ - 1)
End Function

Function SurfaceAverage(ByRef sArray As Variant) As Double
'Returns the average value of a 2D (surface) array of arbitrary size
Dim i As Integer, j As Integer
Dim localValue As Double
SurfaceAverage = 0
For i = LBound(sArray, 1) To UBound(sArray, 1): For j = LBound(sArray, 2) To UBound(sArray, 2)
    localValue = sArray(i, j)
    If i = LBound(sArray, 1) Or i = UBound(sArray, 1) Then localValue = 0.5 * localValue
    If j = LBound(sArray, 2) Or j = UBound(sArray, 2) Then localValue = 0.5 * localValue
    SurfaceAverage = SurfaceAverage + localValue
Next j: Next i
SurfaceAverage = SurfaceAverage / (UBound(sArray, 1) - LBound(sArray, 1))
SurfaceAverage = SurfaceAverage / (UBound(sArray, 2) - LBound(sArray, 2))
End Function

Function CurrentDensity(ByVal bGamma As Boolean, ByVal nCarrier As Integer, ByVal nAxis As Integer, ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Double
'Returns the current density in the selected axis that is the current crossing the plane through that node divided by the area of that normal plane associated with that node
'Assumes that gamma gradients have been calculated if gamma correction is requested (via bGamma)
Dim nLeft As Integer, nRight As Integer, nRear As Integer, nFront As Integer, nTop As Integer, nBot As Integer
Dim thisJx As Double, thisJy As Double, thisJz As Double
CurrentDensity = 0
If k < sTexture(i, j) Then Exit Function
'Assign near-nodes, but reposition out-of-volume nodes back onto the boundaries (no symmetry assumptions)
If i = 1 Then nLeft = i Else nLeft = i - 1
If i = nNodesX Then nRight = i Else nRight = i + 1
If j = 1 Then nRear = j Else nRear = j - 1
If j = nNodesY Then nFront = j Else nFront = j + 1
If k = 1 Then nTop = k Else nTop = k - 1
If k = nNodesZ Then nBot = k Else nBot = k + 1
'Reposition near-nodes that are in the void back onto the surface
If k = sTexture(i, j) Then
    nTop = k
    Select Case nTextureType
        Case grooves
            nRight = i
        Case pyramids
            If j >= nNodesY - i + 1 Then nRight = i
            If j <= nNodesY - i + 1 Then nRear = j
        Case inverted
            If j >= i Then nRight = i
            If j <= i Then nRear = j
    End Select
End If
Select Case nCarrier
    Case holes
        thisJx = (vSqrSigmaP(nLeft, j, k) * vSqrSigmaP(i, j, k) * (vPhiP(nLeft, j, k) - vPhiP(i, j, k)) _
                    + vSqrSigmaP(nRight, j, k) * vSqrSigmaP(i, j, k) * (vPhiP(i, j, k) - vPhiP(nRight, j, k))) / (2 * rDeltaX)
        thisJy = (vSqrSigmaP(i, nRear, k) * vSqrSigmaP(i, j, k) * (vPhiP(i, nRear, k) - vPhiP(i, j, k)) _
                    + vSqrSigmaP(i, nFront, k) * vSqrSigmaP(i, j, k) * (vPhiP(i, j, k) - vPhiP(i, nFront, k))) / (2 * rDeltaY)
        thisJz = (vSqrSigmaP(i, j, nTop) * vSqrSigmaP(i, j, k) * (vPhiP(i, j, nTop) - vPhiP(i, j, k)) _
                     + vSqrSigmaP(i, j, nBot) * vSqrSigmaP(i, j, k) * (vPhiP(i, j, k) - vPhiP(i, j, nBot))) / (2 * rDeltaZ)
    Case electrons
        thisJx = (vSqrSigmaN(nLeft, j, k) * vSqrSigmaN(i, j, k) * (vPhiN(nLeft, j, k) - vPhiN(i, j, k)) _
                    + vSqrSigmaN(nRight, j, k) * vSqrSigmaN(i, j, k) * (vPhiN(i, j, k) - vPhiN(nRight, j, k))) / (2 * rDeltaX)
        thisJy = (vSqrSigmaN(i, nRear, k) * vSqrSigmaN(i, j, k) * (vPhiN(i, nRear, k) - vPhiN(i, j, k)) _
                    + vSqrSigmaN(i, nFront, k) * vSqrSigmaN(i, j, k) * (vPhiN(i, j, k) - vPhiN(i, nFront, k))) / (2 * rDeltaY)
        thisJz = (vSqrSigmaN(i, j, nTop) * vSqrSigmaN(i, j, k) * (vPhiN(i, j, nTop) - vPhiN(i, j, k)) _
                     + vSqrSigmaN(i, j, nBot) * vSqrSigmaN(i, j, k) * (vPhiN(i, j, k) - vPhiN(i, j, nBot))) / (2 * rDeltaZ)
End Select
'Correct factor-of-two error where one of the two terms in the above sums was excluded (set to zero) due to being out-of-bounds
If nLeft = i Or nRight = i Then thisJx = 2 * thisJx
If nRear = j Or nFront = j Then thisJy = 2 * thisJy
If nTop = k Or nBot = k Then thisJz = 2 * thisJz
'Add gamma correction if requested
If bGamma Then
    Select Case nCarrier
        Case holes
            thisJx = thisJx - vGradGammaP(xAxis, i, j, k)
            thisJy = thisJy - vGradGammaP(yAxis, i, j, k)
            thisJz = thisJz - vGradGammaP(zAxis, i, j, k)
    Case electrons
            thisJx = thisJx - vGradGammaN(xAxis, i, j, k)
            thisJy = thisJy - vGradGammaN(yAxis, i, j, k)
            thisJz = thisJz - vGradGammaN(zAxis, i, j, k)
    End Select
End If
Select Case nAxis
    Case xAxis: CurrentDensity = thisJx
    Case yAxis: CurrentDensity = thisJy
    Case zAxis: CurrentDensity = thisJz
    Case vector: CurrentDensity = Sqr(thisJx ^ 2 + thisJy ^ 2 + thisJz ^ 2)
End Select
End Function

Function Qgamma(ByVal nCarrier As Integer, ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Double
'Calculate error between local G-R and divergence of current obtained from approximate solution for J
'Internal surface recombination (i.e. not at k=1) is included in the G-R term
'Return zero at top and bottom solution-volume surface, which are treated via boundary conditions instead
'Returns zero in the etched void
'Returns zero at contacted nodes
Dim nLeft As Integer, nRight As Integer, nRear As Integer, nFront As Integer, nTop As Integer, nBot As Integer
Dim localGR As Double, localDivJ As Double, localSurface As Double, localVolumeFraction As Double
Qgamma = 0
If nCarrier = combined Then Exit Function       'Qgamma has no meaning for "combined" carriers. Plot shows sum of Q for holes + electrons
If k = 1 Or k = nNodesZ Then Exit Function 'Return zero at top and bottom surfaces
If k < sTexture(i, j) Then Exit Function    'Return zero in the void
If Contact(nCarrier, i, j, k) Then Exit Function    'Return zero at contacted nodes
'Assign near-nodes, utilizing symmetry planes where applicable
If i = 1 Then nLeft = i + 1 Else nLeft = i - 1
If i = nNodesX Then nRight = i - 1 Else nRight = i + 1
If j = 1 Then nRear = j + 1 Else nRear = j - 1
If j = nNodesY Then nFront = j - 1 Else nFront = j + 1
If k = nNodesZ Then nBot = k - 1 Else nBot = k + 1
localGR = vGeneration(i, j, k) - vRecombination(i, j, k)
If k = sTexture(i, j) Then
    nTop = k
    localVolumeFraction = VolumeFraction(i, j, nLeft, nRight, nRear, nFront)
    localSurface = sRecombination(i, j) / (rDeltaZ * rCosTextureAngle)
Else
    nTop = k - 1
    localVolumeFraction = 1
    localSurface = 0
End If
Select Case nCarrier
    Case holes
        localDivJ = -vSqrSigmaP(i, j, k) _
                        * (k2x * vSqrSigmaP(nLeft, j, k) * (vPhiP(nLeft, j, k) - vPhiP(i, j, k)) + k2x * vSqrSigmaP(nRight, j, k) * (vPhiP(nRight, j, k) - vPhiP(i, j, k)) _
                        + k2y * vSqrSigmaP(i, nRear, k) * (vPhiP(i, nRear, k) - vPhiP(i, j, k)) + k2y * vSqrSigmaP(i, nFront, k) * (vPhiP(i, nFront, k) - vPhiP(i, j, k)) _
                        + k2z * vSqrSigmaP(i, j, nTop) * (vPhiP(i, j, nTop) - vPhiP(i, j, k)) + k2z * vSqrSigmaP(i, j, nBot) * (vPhiP(i, j, nBot) - vPhiP(i, j, k)))
        Qgamma = localDivJ - rQ * localGR * localVolumeFraction + rQ * localSurface
    Case electrons
        localDivJ = -vSqrSigmaN(i, j, k) _
                        * (k2x * vSqrSigmaN(nLeft, j, k) * (vPhiN(nLeft, j, k) - vPhiN(i, j, k)) + k2x * vSqrSigmaN(nRight, j, k) * (vPhiN(nRight, j, k) - vPhiN(i, j, k)) _
                        + k2y * vSqrSigmaN(i, nRear, k) * (vPhiN(i, nRear, k) - vPhiN(i, j, k)) + k2y * vSqrSigmaN(i, nFront, k) * (vPhiN(i, nFront, k) - vPhiN(i, j, k)) _
                        + k2z * vSqrSigmaN(i, j, nTop) * (vPhiN(i, j, nTop) - vPhiN(i, j, k)) + k2z * vSqrSigmaN(i, j, nBot) * (vPhiN(i, j, nBot) - vPhiN(i, j, k)))
        Qgamma = localDivJ + rQ * localGR * localVolumeFraction - rQ * localSurface
    Case combined
        localDivJ = -vSqrSigmaP(i, j, k) _
                        * (k2x * vSqrSigmaP(nLeft, j, k) * (vPhiP(nLeft, j, k) - vPhiP(i, j, k)) + k2x * vSqrSigmaP(nRight, j, k) * (vPhiP(nRight, j, k) - vPhiP(i, j, k)) _
                        + k2y * vSqrSigmaP(i, nRear, k) * (vPhiP(i, nRear, k) - vPhiP(i, j, k)) + k2y * vSqrSigmaP(i, nFront, k) * (vPhiP(i, nFront, k) - vPhiP(i, j, k)) _
                        + k2z * vSqrSigmaP(i, j, nTop) * (vPhiP(i, j, nTop) - vPhiP(i, j, k)) + k2z * vSqrSigmaP(i, j, nBot) * (vPhiP(i, j, nBot) - vPhiP(i, j, k))) _
                         - vSqrSigmaN(i, j, k) _
                        * (k2x * vSqrSigmaN(nLeft, j, k) * (vPhiN(nLeft, j, k) - vPhiN(i, j, k)) + k2x * vSqrSigmaN(nRight, j, k) * (vPhiN(nRight, j, k) - vPhiN(i, j, k)) _
                        + k2y * vSqrSigmaN(i, nRear, k) * (vPhiN(i, nRear, k) - vPhiN(i, j, k)) + k2y * vSqrSigmaN(i, nFront, k) * (vPhiN(i, nFront, k) - vPhiN(i, j, k)) _
                        + k2z * vSqrSigmaN(i, j, nTop) * (vPhiN(i, j, nTop) - vPhiN(i, j, k)) + k2z * vSqrSigmaN(i, j, nBot) * (vPhiN(i, j, nBot) - vPhiN(i, j, k)))
        Qgamma = localDivJ
End Select
End Function

Function ElectricField(ByVal nAxis As Integer, ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Double
Dim nLeft As Integer, nRight As Integer, nRear As Integer, nFront As Integer, nTop As Integer, nBot As Integer
Dim thisEx As Double, thisEy As Double, thisEz As Double
ElectricField = 0
If k < sTexture(i, j) Then Exit Function
'Assign near-nodes, but reposition out-of-volume nodes back onto the boundaries (no symmetry assumptions)
If i = 1 Then nLeft = i Else nLeft = i - 1
If i = nNodesX Then nRight = i Else nRight = i + 1
If j = 1 Then nRear = j Else nRear = j - 1
If j = nNodesY Then nFront = j Else nFront = j + 1
If k = 1 Then nTop = k Else nTop = k - 1
If k = nNodesZ Then nBot = k Else nBot = k + 1
'Reposition near-nodes that are in the void back onto the surface
If k = sTexture(i, j) Then
    nTop = k
    Select Case nTextureType
        Case grooves
            nRight = i
        Case pyramids
            If j >= nNodesY - i + 1 Then nRight = i
            If j <= nNodesY - i + 1 Then nRear = j
        Case inverted
            If j >= i Then nRight = i
            If j <= i Then nRear = j
    End Select
End If
thisEx = ((vPsi(nLeft, j, k) - vPsi(i, j, k)) + (vPsi(i, j, k) - vPsi(nRight, j, k))) / (2 * rDeltaX)
thisEy = ((vPsi(i, nRear, k) - vPsi(i, j, k)) + (vPsi(i, j, k) - vPsi(i, nFront, k))) / (2 * rDeltaY)
thisEz = ((vPsi(i, j, nTop) - vPsi(i, j, k)) + (vPsi(i, j, k) - vPsi(i, j, nBot))) / (2 * rDeltaZ)
'Correct factor-of-two error where one of the two terms in the above sums was excluded (set to zero) due to being out-of-bounds
If nLeft = i Or nRight = i Then thisEx = 2 * thisEx
If nRear = j Or nFront = j Then thisEy = 2 * thisEy
If nTop = k Or nBot = k Then thisEz = 2 * thisEz
Select Case nAxis
    Case xAxis: ElectricField = thisEx
    Case yAxis: ElectricField = thisEy
    Case zAxis: ElectricField = thisEz
    Case vector: ElectricField = Sqr(thisEx ^ 2 + thisEy ^ 2 + thisEz ^ 2)
End Select
End Function

Function LateralCurrent(ByVal i As Integer) As Double
'Returns the total current (A) for the carrier of the surface type crossing the x-node plane having index i
Dim j As Integer, k As Integer
Dim localAreaFraction As Double
LateralCurrent = 0
For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    localAreaFraction = 1
    'Special treatment required at surface nodes
    If k = sTexture(i, j) Then
        localAreaFraction = 1 / 2   'Default at top surface, can override for special cases below
        Select Case nTextureType
            Case pyramids
                If j = 1 Then localAreaFraction = 3 / 4
                If j = nNodesY - i + 1 Then localAreaFraction = 3 / 8
                If i = nNodesX And j = 1 Then localAreaFraction = 1 / 2
                If i = 1 And j = nNodesY Then localAreaFraction = 1 / 4
            Case inverted
                If j = 1 Then localAreaFraction = 1 / 4
                If j = i Then localAreaFraction = 5 / 8
                If i = 1 And j = 1 Then localAreaFraction = 1 / 2
                If i = nNodesX And j = nNodesY Then localAreaFraction = 3 / 4
        End Select
    End If
    'Only count the portion of the unfolded area that is within the solution region
    If k = nNodesZ Then localAreaFraction = localAreaFraction / 2       'Top surface fraction accounted-for above
    If j = 1 Or j = nNodesY Then localAreaFraction = localAreaFraction / 2
    LateralCurrent = LateralCurrent + localAreaFraction * CurrentDensity(False, nSurfaceType, xAxis, i, j, k) * rDeltaY * rDeltaZ
Next k: Next j
End Function

Function BottomCurrent(ByVal nCarrier) As Double
'Returns the average current density (A/cm2) in the z-direction for the specified carrier at k=nNodesZ in the absence of Gamma correction
Dim i As Integer, j As Integer
Dim localAreaFraction As Double
BottomCurrent = 0
For i = 1 To nNodesX: For j = 1 To nNodesY
    localAreaFraction = 1
    If i = 1 Or i = nNodesX Then localAreaFraction = localAreaFraction / 2
    If j = 1 Or j = nNodesY Then localAreaFraction = localAreaFraction / 2
    BottomCurrent = BottomCurrent + localAreaFraction * CurrentDensity(False, nCarrier, zAxis, i, j, nNodesZ)
Next j: Next i
BottomCurrent = BottomCurrent / (nNodesX - 1)
BottomCurrent = BottomCurrent / (nNodesY - 1)
End Function

Function TopCurrent(ByVal nCarrier) As Double
'Returns the average current density (A/cm2) in the z-direction for the specified carrier at k=1 in the absence of Gamma correction
Dim i As Integer, j As Integer
Dim localAreaFraction As Double
TopCurrent = 0
For i = 1 To nNodesX: For j = 1 To nNodesY
    localAreaFraction = 1
    If i = 1 Or i = nNodesX Then localAreaFraction = localAreaFraction / 2
    If j = 1 Or j = nNodesY Then localAreaFraction = localAreaFraction / 2
   TopCurrent = TopCurrent + localAreaFraction * CurrentDensity(False, nCarrier, zAxis, i, j, 1)
Next j: Next i
TopCurrent = TopCurrent / (nNodesX - 1)
TopCurrent = TopCurrent / (nNodesY - 1)
End Function

Function VolumeMin(ByRef vArray As Variant) As Double
'Finds the minimum value of a volume array of nNodesX x nNodesY x nNodesZ, ignoring nodes that have been removed by the texture etch
Dim i As Integer, j As Integer, k As Integer
Dim thisValue As Double
VolumeMin = vArray(nNodesX, nNodesY, nNodesZ)
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    thisValue = vArray(i, j, k)
    If thisValue < VolumeMin Then VolumeMin = thisValue
Next k: Next j: Next i
End Function

Function VolumeMax(ByRef vArray As Variant) As Double
'Finds the minimum value of a volume array of nNodesX x nNodesY x nNodesZ, ignoring nodes that have been removed by the texture etch
Dim i As Integer, j As Integer, k As Integer
Dim thisValue As Double
VolumeMax = vArray(nNodesX, nNodesY, nNodesZ)
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    thisValue = vArray(i, j, k)
    If thisValue > VolumeMax Then VolumeMax = thisValue
Next k: Next j: Next i
End Function

Function MMult3D(ByRef A As Variant, ByRef b As Variant) As Variant
'Returns an RxNxP matrix (row,col,plane) that is RxM matrix A (row,col)
' times MxNxP matrix B (row,col,plane), where each plane of B is multiplied by A
'Note that the columns of A must match the rows of B
Dim C As Variant
Dim Q As Variant
Dim d As Variant
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer, p As Integer, r As Integer
m = UBound(b, 1)
n = UBound(b, 2)
p = UBound(b, 3)
r = UBound(A, 1)
ReDim C(1 To r, 1 To n, 1 To p) As Double
ReDim Q(1 To m, 1 To n) As Double
ReDim d(1 To r, 1 To n) As Double
For k = 1 To p
    For i = 1 To m: For j = 1 To n
        Q(i, j) = b(i, j, k)
    Next j: Next i
    d = Application.MMult(A, Q)
    For i = 1 To r: For j = 1 To n
        C(i, j, k) = d(i, j)
    Next j: Next i
Next k
MMult3D = C
End Function

Function MTrans3Drc(ByRef A As Variant) As Variant
'Returns an NxMxP matrix that has the row and column indices of MxNxP matrix A transposed.
'The last index (plane) is preserved.
Dim C As Variant
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer, p As Integer
m = UBound(A, 1)
n = UBound(A, 2)
p = UBound(A, 3)
ReDim C(1 To n, 1 To m, 1 To p) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To p
    C(j, i, k) = A(i, j, k)
Next k: Next j: Next i
MTrans3Drc = C
End Function

Function MTrans3Drp(ByRef A As Variant) As Variant
'Returns an PxNxM matrix that has the row and plane indices of MxNxP matrix A transposed.
'The second index (column) is preserved.
Dim C As Variant
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer, p As Integer
m = UBound(A, 1)
n = UBound(A, 2)
p = UBound(A, 3)
ReDim C(1 To p, 1 To n, 1 To m) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To p
    C(k, j, i) = A(i, j, k)
Next k: Next j: Next i
MTrans3Drp = C
End Function

Function MTrans3Dback(ByRef A As Variant) As Variant
'Returns an NxPxM matrix that has the indices of MxNxP matrix A rotated back by one index.
Dim C As Variant
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer, p As Integer
m = UBound(A, 1)
n = UBound(A, 2)
p = UBound(A, 3)
ReDim C(1 To n, 1 To p, 1 To m) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To p
    C(j, k, i) = A(i, j, k)
Next k: Next j: Next i
MTrans3Dback = C
End Function

Function MAdd3D(A As Variant, b As Variant) As Variant
'Returns the entry-by-entry sum of two 3D matrices A and B.
Dim i As Integer, j As Integer, k As Integer, m As Integer, n As Integer, p As Integer
Dim Q As Variant
m = UBound(A, 1)
n = UBound(A, 2)
p = UBound(A, 3)
ReDim Q(1 To m, 1 To n, 1 To p) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To p
    Q(i, j, k) = A(i, j, k) + b(i, j, k)
Next k: Next j: Next i
MAdd3D = Q
End Function

Function MSub3D(A As Variant, b As Variant) As Variant
'Returns the entry-by-entry subtraction of 3D matrix B from A.
Dim i As Integer, j As Integer, k As Integer, m As Integer, n As Integer, p As Integer
Dim Q As Variant
m = UBound(A, 1)
n = UBound(A, 2)
p = UBound(A, 3)
ReDim Q(1 To m, 1 To n, 1 To p) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To p
    Q(i, j, k) = A(i, j, k) - b(i, j, k)
Next k: Next j: Next i
MSub3D = Q
End Function

Function MScale3D(A As Variant, b As Double) As Variant
'Returns a 3D matrix with each entry being B times the corresponding entry in 3D matrix A.
Dim i As Integer, j As Integer, k As Integer, m As Integer, n As Integer, p As Integer
Dim Q As Variant
m = UBound(A, 1)
n = UBound(A, 2)
p = UBound(A, 3)
ReDim Q(1 To m, 1 To n, 1 To p) As Double
For i = 1 To m: For j = 1 To n: For k = 1 To p
    Q(i, j, k) = A(i, j, k) * b
Next k: Next j: Next i
MScale3D = Q
End Function

Function Tri(ByVal x As Double) As Double
Tri = 0
If x <= 0 And x > -1 Then Tri = 1 + x
If x > 0 And x < 1 Then Tri = 1 - x
End Function

Function Sinc(ByVal x As Double) As Double
If x = 0 Then Sinc = 1 Else Sinc = Sin(x) / x
End Function

Function Cosh(ByVal x As Double) As Double
    Cosh = (ExpPower(x) + ExpPower(-x)) / 2
End Function

Function Sinh(ByVal x As Double) As Double
    Sinh = (ExpPower(x) - ExpPower(-x)) / 2
End Function

Function ExpPower(ByVal x As Double) As Double
    Select Case x
        Case Is < -300: ExpPower = 1E-100
        Case Is > 300: ExpPower = 1E+100
        Case Else: ExpPower = Exp(x)
    End Select
End Function

Function PointDefect(ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Boolean
'Returns true if the node i,j,k is a point defect
PointDefect = False
If i - 1 = Round(rDefectPosition(point, xAxis) * (nNodesX - 1) / 100, 0) And _
   j - 1 = (nNodesY - 1) - Round(rDefectPosition(point, yAxis) * (nNodesY - 1) / 100, 0) And _
   k - 1 = Round(rDefectPosition(point, zAxis) * (nNodesZ - 1) / 100, 0) Then
        PointDefect = True
End If
End Function

Function LineDefect(ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Boolean
'Returns true if the node i,j,k is a line defect
Dim voltage As Double, localP As Double, localN As Double, thisIo As Double
LineDefect = False
If lineDefectDirection = "Side" And _
     j - 1 = (nNodesY - 1) - Round(rDefectPosition(line, yAxis) * (nNodesY - 1) / 100, 0) And _
     k - 1 = Round(rDefectPosition(line, zAxis) * (nNodesZ - 1) / 100, 0) And _
     i - 1 <= rDefectPosition(line, xAxis) / 100 * (nNodesX - 1) Then LineDefect = True
If lineDefectDirection = "Face" And _
      i - 1 = Round(rDefectPosition(line, xAxis) * (nNodesX - 1) / 100, 0) And _
      k - 1 = Round(rDefectPosition(line, zAxis) * (nNodesZ - 1) / 100, 0) And _
      nNodesY - j <= rDefectPosition(line, yAxis) / 100 * (nNodesY - 1) Then LineDefect = True
If lineDefectDirection = "Top" And _
      i - 1 = Round(rDefectPosition(line, xAxis) * (nNodesX - 1) / 100, 0) And _
      j - 1 = (nNodesY - 1) - Round(rDefectPosition(line, yAxis) * (nNodesY - 1) / 100, 0) And _
      k - 1 <= rDefectPosition(line, zAxis) / 100 * (nNodesZ - 1) Then LineDefect = True
End Function

Function PlaneDefect(ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Boolean
'Returns true if the node i,j,k is a plane defect
PlaneDefect = False
If planeDefectDirection = "Side" And i - 1 = Round(rDefectPosition(plane, xAxis) * (nNodesX - 1) / 100, 0) And _
      nNodesY - j <= rDefectPosition(plane, yAxis) / 100 * (nNodesY - 1) And _
      k - 1 <= rDefectPosition(plane, zAxis) / 100 * (nNodesZ - 1) Then PlaneDefect = True
If planeDefectDirection = "Face" And j - 1 = (nNodesY - 1) - Round(rDefectPosition(plane, yAxis) * (nNodesY - 1) / 100, 0) And _
      i - 1 <= rDefectPosition(plane, xAxis) / 100 * (nNodesX - 1) And _
      k - 1 <= rDefectPosition(plane, zAxis) / 100 * (nNodesZ - 1) Then PlaneDefect = True
If planeDefectDirection = "Top" And k - 1 = Round(rDefectPosition(plane, zAxis) * (nNodesZ - 1) / 100, 0) And _
      i - 1 <= rDefectPosition(plane, xAxis) / 100 * (nNodesX - 1) And _
      nNodesY - j <= rDefectPosition(plane, yAxis) / 100 * (nNodesY - 1) Then PlaneDefect = True
End Function

Function SiElectronDiffusivity(ByVal rNa As Double, ByVal rNd As Double) As Double
'Diffusivity model matches PC1D v5.9
Const max As Double = 1417
Const majMin As Double = 60, majNref As Double = 9.64E+16, majAlpha As Double = 0.664
Const minMin As Double = 160, minNref As Double = 5.6E+16, minAlpha As Double = 0.647
Const beta1 As Double = -0.57, beta2 As Double = -2.33, beta3 As Double = 2.4, beta4 As Double = -0.146
Dim mobility As Double, Tn As Double
Tn = rVt / rVt300
If rNd > rNa Then     'Electrons are majority carriers
    mobility = majMin * Tn ^ beta1 + (max - majMin) * Tn ^ beta2 / (1 + ((rNa + rNd) / (majNref * Tn ^ beta3)) ^ (majAlpha * Tn ^ beta4))
Else                'Electrons are minority carriers
    mobility = minMin * Tn ^ beta1 + (max - minMin) * Tn ^ beta2 / (1 + ((rNa + rNd) / (minNref * Tn ^ beta3)) ^ (minAlpha * Tn ^ beta4))
End If
SiElectronDiffusivity = rVt * mobility
End Function

Function SiHoleDiffusivity(ByVal rNa As Double, ByVal rNd As Double) As Double
'Diffusivity model matches PC1D v5.9
Const max As Double = 470
Const majMin As Double = 37.4, majNref As Double = 2.82E+17, majAlpha As Double = 0.642
Const minMin As Double = 155, minNref As Double = 1E+17, minAlpha As Double = 0.9
Const beta1 As Double = -0.57, beta2 As Double = -2.23, beta3 As Double = 2.4, beta4 As Double = -0.146
Dim mobility As Double, Tn As Double
Tn = rVt / rVt300
If rNa > rNd Then     'Holes are majority carriers
    mobility = majMin * Tn ^ beta1 + (max - majMin) * Tn ^ beta2 / (1 + ((rNa + rNd) / (majNref * Tn ^ beta3)) ^ (majAlpha * Tn ^ beta4))
Else                'Holes are minority carriers
    mobility = minMin * Tn ^ beta1 + (max - minMin) * Tn ^ beta2 / (1 + ((rNa + rNd) / (minNref * Tn ^ beta3)) ^ (minAlpha * Tn ^ beta4))
End If
SiHoleDiffusivity = rVt * mobility
End Function

Function SiIntrinsicConcentration(ByVal rTemperature As Double) As Double
'For the given temperature in kelvin, returns the intrinsic concentration for silicon, scaled to match the value at 25C, which is globally stored as rNi25C.
'Obtains intrinsic concentration from parabolic fit of ln(ni) vs 1/T
Const rToffset = 273.15      'Conversion from Celsius to Kelvin
Const rNi200 = 47000#
Const rNi400 = 5600000000000#
Dim A As Double, b As Double, C As Double
Dim x1 As Double, x2 As Double, x3 As Double
Dim y1 As Double, y2 As Double, y3 As Double
x1 = 1 / 200
x2 = 1 / (25 + rToffset)
x3 = 1 / 400
y1 = Log(rNi200)
y2 = Log(rNi25C)
y3 = Log(rNi400)
A = ((y3 - y2) * (x2 - x1) - (y2 - y1) * (x3 - x2)) / ((x3 ^ 2 - x2 ^ 2) * (x2 - x1) - (x2 ^ 2 - x1 ^ 2) * (x3 - x2))
b = ((y2 - y1) - A * (x2 ^ 2 - x1 ^ 2)) / (x2 - x1)
C = y1 - A * x1 ^ 2 - b * x1
SiIntrinsicConcentration = ExpPower(A * (1 / rTemperature) ^ 2 + b * (1 / rTemperature) + C)
End Function

Function TwoDiodeFit(ByRef dataIV As Variant, ByVal bIncludeGsh As Boolean, ByVal bincludeJo2 As Boolean, ByVal bincludeJo1 As Boolean, _
                                      ByRef Jo1 As Double, ByRef Jo2 As Double, ByRef Gsh As Double) As Boolean
'Array dataIV has voltages in column 1 and currents in column 2
'The data points are weighted by inverse current squared, to simulate a best-fit on a logarithmic scale
'Returns true if all three recombination parameters are >= 0
Dim i As Integer, n As Integer
Dim vJ As Double, Ji As Double, weight As Double
Dim x As Variant, y As Variant, m As Variant, v As Variant, F As Variant
ReDim x(0 To 6) As Double
ReDim y(0 To 3) As Double
For i = 1 To UBound(dataIV, 1)
    If dataIV(i, 1) <> "" Then
        vJ = dataIV(i, 1)
        Ji = dataIV(i, 2)
        n = n + 1
        weight = 1 / Ji ^ 2
        x(0) = x(0) + weight
        x(1) = x(1) + weight * (ExpPower(vJ / rVt) - 1)
        x(2) = x(2) + weight * (ExpPower(vJ / rVt) - 1) ^ 2
        x(3) = x(3) + weight * ((ExpPower(vJ / (2 * rVt)) - 1) * (ExpPower(vJ / rVt) - 1))
        x(4) = x(4) + weight * vJ * (ExpPower(vJ / rVt) - 1)
        x(5) = x(5) + weight * vJ * (ExpPower(vJ / (2 * rVt)) - 1)
        x(6) = x(6) + weight * vJ ^ 2
        y(0) = y(0) + weight * Ji
        y(1) = y(1) + weight * Ji * (ExpPower(vJ / rVt) - 1)
        y(2) = y(2) + weight * Ji * (ExpPower(vJ / (2 * rVt)) - 1)
        y(3) = y(3) + weight * Ji * vJ
    End If
Next i
If n < 3 Then Exit Function
If bIncludeGsh And bincludeJo2 And bincludeJo1 Then
    ReDim m(1 To 3, 1 To 3) As Double
    ReDim v(1 To 3, 1 To 1) As Double
    ReDim F(1 To 3, 1 To 1) As Double
    m(1, 1) = x(2): m(1, 2) = x(3): m(1, 3) = x(4)
    m(2, 1) = x(3): m(2, 2) = x(1): m(2, 3) = x(5)
    m(3, 1) = x(4): m(3, 2) = x(5): m(3, 3) = x(6)
    v(1, 1) = y(1): v(2, 1) = y(2): v(3, 1) = y(3)
    F = Application.MMult(Application.MInverse(m), v)
    Jo1 = F(1, 1): Jo2 = F(2, 1): Gsh = F(3, 1)
End If
If Not bIncludeGsh And bincludeJo2 And bincludeJo1 Then
    ReDim m(1 To 2, 1 To 2) As Double
    ReDim v(1 To 2, 1 To 1) As Double
    ReDim F(1 To 2, 1 To 1) As Double
    m(1, 1) = x(2): m(1, 2) = x(3)
    m(2, 1) = x(3): m(2, 2) = x(1)
    v(1, 1) = y(1): v(2, 1) = y(2)
    F = Application.MMult(Application.MInverse(m), v)
    Jo1 = F(1, 1): Jo2 = F(2, 1): Gsh = 0
End If
If bIncludeGsh And Not bincludeJo2 And bincludeJo1 Then
    ReDim m(1 To 2, 1 To 2) As Double
    ReDim v(1 To 2, 1 To 1) As Double
    ReDim F(1 To 2, 1 To 1) As Double
    m(1, 1) = x(2): m(1, 2) = x(4)
    m(2, 1) = x(4): m(2, 2) = x(6)
    v(1, 1) = y(1): v(2, 1) = y(3)
    F = Application.MMult(Application.MInverse(m), v)
    Jo1 = F(1, 1): Jo2 = 0: Gsh = F(2, 1)
End If
If bIncludeGsh And bincludeJo2 And Not bincludeJo1 Then
    ReDim m(1 To 2, 1 To 2) As Double
    ReDim v(1 To 2, 1 To 1) As Double
    ReDim F(1 To 2, 1 To 1) As Double
    m(1, 1) = x(1): m(1, 2) = x(5)
    m(2, 1) = x(5): m(2, 2) = x(6)
    v(1, 1) = y(2): v(2, 1) = y(3)
    F = Application.MMult(Application.MInverse(m), v)
    Jo1 = 0: Jo2 = F(1, 1): Gsh = F(2, 1)
End If
TwoDiodeFit = (Gsh >= 0 And Jo2 >= 0 And Jo1 >= 0)
End Function

Function Requires2D() As Boolean
'Returns true if the current device specfication, defects, or contact mode have x-axis  dependence.
'Bulk defects are ignored if not enabled.
Requires2D = True
If nTextureType > planar Then Exit Function
If nSolutionMode = lateral Then Exit Function
If bEnablePointDefect Or bEnableLineDefect Or (bEnablePlaneDefect And _
  ((planeDefectDirection = "Face" Or planeDefectDirection = "Side") Or (rDefectPosition(plane, yAxis) < 100) Or rDefectPosition(plane, xAxis) < 100)) Then _
    Exit Function
Requires2D = False
End Function

Function Requires3D() As Boolean
'Returns true if the current device specfication, defects, or contact mode have y-axis dependence.
'Bulk defects are ignored if not enabled.
Requires3D = True
If nTextureType > grooves Then Exit Function
If bEnablePointDefect Or _
  (bEnableLineDefect And (lineDefectDirection <> "Face" Or rDefectPosition(line, yAxis) < 100)) Or _
  (bEnablePlaneDefect And (planeDefectDirection = "Face" Or rDefectPosition(plane, yAxis) < 100)) Then _
    Exit Function
Requires3D = False
End Function

Function Contact(ByVal nCarrier As Integer, ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Boolean
Contact = True
Select Case nCarrier
    Case holes
        Select Case nSolutionMode
            Case bipolar
                If k = nNodesZ And nBackgroundType = ptype Then Exit Function
                If k = sTexture(i, j) And nBackgroundType = ntype Then Exit Function
            Case floating: If k = nNodesZ Then Exit Function
            Case lateral: If i = 1 Or i = nNodesX Then Exit Function
        End Select
    Case electrons
        Select Case nSolutionMode
            Case bipolar
                If k = nNodesZ And nBackgroundType = ntype Then Exit Function
                If k = sTexture(i, j) And nBackgroundType = ptype Then Exit Function
            Case floating: If k = nNodesZ Then Exit Function
            Case lateral:  If i = 1 Or i = nNodesX Then Exit Function
        End Select
End Select
Contact = False
End Function

Function SurfaceType() As Integer
'Returns with either pType or nType to indicate the expected dopant type of the top surface
Dim rNs As Double
If nBackgroundType = ptype Then rNs = rBackgroundDoping Else rNs = -rBackgroundDoping
If nDiffusionType = ptype Then rNs = rNs + rDiffusionPeak Else rNs = rNs - rDiffusionPeak
If rNs > 0 Then SurfaceType = ptype Else SurfaceType = ntype
End Function

