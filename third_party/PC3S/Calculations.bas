Attribute VB_Name = "Calculations"
Option Explicit

Sub CalculateTexture()
'Set resolution in x and y axes to match requested texture angle
'Find smallest k at each i,j location that is NOT removed by texture etch
'Do not alter any spreadsheet cells in this routine that could trigger a reset call
'Sets deviceWidth, deviceLength, and deviceHeight
Dim i As Integer, j As Integer
Dim zSurface As Double
If nTextureType = planar Then
    nHarmonicsX = WorksheetFunction.Round(nHarmonicsZ / 2, 0)
    nHarmonicsY = WorksheetFunction.Round(nHarmonicsZ / 2, 0)
    deviceWidth = baseHeight
    deviceLength = baseHeight
    deviceHeight = baseHeight
Else
    nHarmonicsX = WorksheetFunction.Round(nHarmonicsZ * textureDepth / (textureDepth + baseHeight), 0)
    nHarmonicsY = WorksheetFunction.Round(nHarmonicsZ * textureDepth / (textureDepth + baseHeight), 0)
    deviceWidth = textureDepth / Tan(textureAngle)
    deviceLength = textureDepth / Tan(textureAngle)
    deviceHeight = nHarmonicsZ / nHarmonicsX * textureDepth
End If
Range(deviceWidthCell) = deviceWidth * 10000#
Range(deviceLengthCell) = deviceLength * 10000#
Range(deviceHeightCell) = deviceHeight * 10000#
Range(deviceAreaCell).Value = 100000000# * deviceWidth * deviceLength
If nDimensions < 3 Then nHarmonicsY = 1    'At least one harmonic is required for interpolation routines
If nDimensions < 2 Then nHarmonicsX = 1
nNodesX = nHarmonicsX + 1
nNodesY = nHarmonicsY + 1
rDeltaX = deviceWidth / (nNodesX - 1): rDeltaX2 = rDeltaX ^ 2: k2x = 1 / rDeltaX2
rDeltaY = deviceLength / (nNodesY - 1): rDeltaY2 = rDeltaY ^ 2: k2y = 1 / rDeltaY2
rDeltaZ = deviceHeight / (nNodesZ - 1): rDeltaZ2 = rDeltaZ ^ 2: k2z = 1 / rDeltaZ2
If nDimensions < 3 Then k2y = 0
If nDimensions < 2 Then k2x = 0
k2xyz = k2x + k2y + k2z
ReDim sTexture(1 To nNodesX, 1 To nNodesY) As Integer
Select Case nTextureType
    Case planar
        For i = 1 To nNodesX: For j = 1 To nNodesY
            sTexture(i, j) = 1
        Next j: Next i
    Case grooves
        For i = 1 To nNodesX: For j = 1 To nNodesY
            zSurface = Tan(textureAngle) * (i - 1) * rDeltaX
            If zSurface > deviceHeight Then zSurface = deviceHeight
            sTexture(i, j) = Round(zSurface / rDeltaZ, 1) + 1
        Next j: Next i
    Case pyramids
        For i = 1 To nNodesX: For j = 1 To nNodesY
            If j > nNodesY - i + 1 Then zSurface = Tan(textureAngle) * (i - 1) * rDeltaX Else zSurface = Tan(textureAngle) * (nNodesY - j) * rDeltaY
            If zSurface > deviceHeight Then zSurface = deviceHeight
            sTexture(i, j) = Round(zSurface / rDeltaZ, 1) + 1
        Next j: Next i
    Case inverted
        For i = 1 To nNodesX: For j = 1 To nNodesY
            If j > i Then zSurface = Tan(textureAngle) * (i - 1) * rDeltaX Else zSurface = Tan(textureAngle) * (j - 1) * rDeltaY
            If zSurface > deviceHeight Then zSurface = deviceHeight
            sTexture(i, j) = Round(zSurface / rDeltaZ, 1) + 1
        Next j: Next i
End Select
End Sub

Sub CalculateRecombination()
'This routine calculates the recombination at each node in the solution volume (cm-3/s) and at each node on the (textured) surface (cm-2/s)
'Returns zero in the void
'The recombination in the half-element adjacent to the surface is adjusted to accommodate the rapid change that can occur near a charged surface
'This adjustment needs to match the adjustment that is made in function Update (Solver module) that is used to find Psi at the surface
Dim i As Integer, j As Integer, k As Integer
Dim localP As Double, localN As Double, localExcessPN As Double, localNiEff As Double, localVp As Double, localVn As Double
Dim localRecombination As Double, maxR As Double, rDebye As Double
Dim bRedBulk As Boolean, bRedSurface As Boolean
bRedBulk = False: bRedSurface = False
ReDim vRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    localVp = vVp(i, j, k)
    localVn = vVn(i, j, k)
    localNiEff = rNi * ExpPower((localVp + localVn) / (2 * rVt))
    localP = rNi * ExpPower((vPhiP(i, j, k) - vPsi(i, j, k) + localVp) / rVt)
    localN = rNi * ExpPower((vPsi(i, j, k) - vPhiN(i, j, k) + localVn) / rVt)
    If k = sTexture(i, j) Then  'Use Debye length to adjust average carrier density to account for the rapid change that can occur near a charged surface
       If localN > localP Then
            rDebye = Sqr(2 * rVt / (localN * rQ_Eps))
            If rDebye < rDeltaZ * rCosTextureAngle Then localN = vDopingN(i, j, k) + (localN - vDopingN(i, j, k)) * rDebye / (rDeltaZ * rCosTextureAngle)
            'Empirical adjustment for accumulation when surface doping is already enough to make Debye length less than node spacing
            If vDopingN(i, j, k) > 2 * rVt / rQ_Eps / (rDeltaZ * rCosTextureAngle) ^ 2 And localN > vDopingN(i, j, k) Then localN = localN * Sqr(localN / vDopingN(i, j, k))
        Else
            rDebye = Sqr(2 * rVt / (localP * rQ_Eps))
            If rDebye < rDeltaZ * rCosTextureAngle Then localP = vDopingP(i, j, k) + (localP - vDopingP(i, j, k)) * rDebye / (rDeltaZ * rCosTextureAngle)
            'Empirical adjustment for accumulation when surface doping is already enough to make Debye length less than node spacing
            If vDopingP(i, j, k) > 2 * rVt / rQ_Eps / (rDeltaZ * rCosTextureAngle) ^ 2 And localP > vDopingP(i, j, k) Then localP = localP * Sqr(localP / vDopingP(i, j, k))
        End If
    End If
    localExcessPN = localP * localN - localNiEff ^ 2
    localRecombination = localExcessPN / ((localP + localNiEff) * rTauN + (localN + localNiEff) * rTauP) + (rCp * localP + rCn * localN) * localExcessPN
    If bEnablePointDefect Or bEnableLineDefect Or bEnablePlaneDefect Then
        If (bEnablePointDefect And PointDefect(i, j, k)) Or _
            (bEnableLineDefect And LineDefect(i, j, k)) Or _
            (bEnablePlaneDefect And PlaneDefect(i, j, k)) Then
            localRecombination = localRecombination + localExcessPN / ((localP + localNiEff) * rDefectTauN + (localN + localNiEff) * rDefectTauP)
        End If
    End If
    vRecombination(i, j, k) = localRecombination
Next k: Next j: Next i
'Calculate surface recombination rate at each node on the textured surface, limited to the maximum minority flow for the given nodal resolution
For i = 1 To nNodesX: For j = 1 To nNodesY
    k = sTexture(i, j)
    localVp = vVp(i, j, k)
    localVn = vVn(i, j, k)
    localNiEff = rNi * ExpPower((localVp + localVn) / (2 * rVt))
    localP = rNi * ExpPower((vPhiP(i, j, k) - vPsi(i, j, k) + localVp) / rVt)
    localN = rNi * ExpPower((vPsi(i, j, k) - vPhiN(i, j, k) + localVn) / rVt)
    localExcessPN = localP * localN - localNiEff ^ 2
    localRecombination = sVelocity(i, j) * localExcessPN / (localP + localN + 2 * localNiEff)
    sRecombination(i, j) = localRecombination
Next j: Next i
End Sub

Sub CalculateSurfaceVelocity()
'Calculate surface recombination velocity at each point on the textured surface
Dim i As Integer, j As Integer
For i = 1 To nNodesX: For j = 1 To nNodesY
    Select Case Range(frontSshapeCell)
        Case "Uniform"
            sVelocity(i, j) = rFrontSpeak
        Case "Linear"
            Select Case nTextureType
                Case planar
                    sVelocity(i, j) = rFrontSpeak
                Case grooves
                    sVelocity(i, j) = rFrontSpeak + (rFrontSvalley - rFrontSpeak) * (i - 1) / (nNodesX - 1)
                Case pyramids
                    sVelocity(i, j) = rFrontSpeak + (rFrontSvalley - rFrontSpeak) * WorksheetFunction.max((i - 1) / (nNodesX - 1), (nNodesY - j) / (nNodesY - 1))
                Case inverted
                    sVelocity(i, j) = rFrontSpeak + (rFrontSvalley - rFrontSpeak) * WorksheetFunction.max((i - 1) / (nNodesX - 1), (j - 1) / (nNodesY - 1))
            End Select
        Case "Log"
            If rFrontSpeak <= 0 Then rFrontSpeak = 1
            If rFrontSvalley <= 0 Then rFrontSvalley = 1
            Select Case nTextureType
                Case planar
                    sVelocity(i, j) = rFrontSpeak
                Case grooves
                    sVelocity(i, j) = ExpPower(Log(rFrontSpeak) + (Log(rFrontSvalley) - Log(rFrontSpeak)) * (i - 1) / (nNodesX - 1))
                Case pyramids
                    sVelocity(i, j) = ExpPower(Log(rFrontSpeak) + (Log(rFrontSvalley) - Log(rFrontSpeak)) * WorksheetFunction.max((i - 1) / (nNodesX - 1), (nNodesY - j) / (nNodesY - 1)))
                Case inverted
                    sVelocity(i, j) = ExpPower(Log(rFrontSpeak) + (Log(rFrontSvalley) - Log(rFrontSpeak)) * WorksheetFunction.max((i - 1) / (nNodesX - 1), (j - 1) / (nNodesY - 1)))
            End Select
    End Select
Next j: Next i
End Sub

Sub CalculateSurfaceCharge()
'Calculate charge density at each point on the textured surface
Dim i As Integer, j As Integer
For i = 1 To nNodesX: For j = 1 To nNodesY
    Select Case Range(frontChargeShapeCell)
        Case "Uniform"
            sCharge(i, j) = rFrontQpeak
        Case "Linear"
            Select Case nTextureType
                Case planar
                    sCharge(i, j) = rFrontQpeak
                Case grooves
                    sCharge(i, j) = rFrontQpeak + (rFrontQvalley - rFrontQpeak) * (i - 1) / (nNodesX - 1)
                Case pyramids
                    sCharge(i, j) = rFrontQpeak + (rFrontQvalley - rFrontQpeak) * WorksheetFunction.max((i - 1) / (nNodesX - 1), (nNodesY - j) / (nNodesY - 1))
                Case inverted
                    sCharge(i, j) = rFrontQpeak + (rFrontQvalley - rFrontQpeak) * WorksheetFunction.max((i - 1) / (nNodesX - 1), (j - 1) / (nNodesY - 1))
            End Select
        Case "Log"
            If rFrontQpeak = 0 Then rFrontQpeak = 1
            If rFrontQvalley = 0 Then rFrontQvalley = 1
            Select Case nTextureType
                Case planar
                    sCharge(i, j) = rFrontQpeak
                Case grooves
                    sCharge(i, j) = Sgn(rFrontQpeak) * ExpPower(Log(Abs(rFrontQpeak)) + (Log(Abs(rFrontQvalley)) - Log(Abs(rFrontQpeak))) * (i - 1) / (nNodesX - 1))
                Case pyramids
                    sCharge(i, j) = Sgn(rFrontQpeak) * ExpPower(Log(Abs(rFrontQpeak)) + (Log(Abs(rFrontQvalley)) - Log(Abs(rFrontQpeak))) * WorksheetFunction.max((i - 1) / (nNodesX - 1), (nNodesY - j) / (nNodesY - 1)))
                Case inverted
                    sCharge(i, j) = Sgn(rFrontQpeak) * ExpPower(Log(Abs(rFrontQpeak)) + (Log(Abs(rFrontQvalley)) - Log(Abs(rFrontQpeak))) * WorksheetFunction.max((i - 1) / (nNodesX - 1), (j - 1) / (nNodesY - 1)))
            End Select
    End Select
Next j: Next i
End Sub

Sub CalculateSigma(ByVal nCarrier As Integer)
'Calculates a value of conductance (actually, square root of conductance) for the designated carrier at every node in the solution volume
Dim i As Integer, j As Integer, k As Integer
Dim localP As Double, localN As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    Select Case nCarrier
        Case holes
            localP = rNi * ExpPower((vPhiP(i, j, k) - vPsi(i, j, k) + vVp(i, j, k)) / rVt)
            vSqrSigmaP(i, j, k) = Sqr(rQ * localP * vDp(i, j, k) / rVt)
        Case electrons
            localN = rNi * ExpPower((vPsi(i, j, k) - vPhiN(i, j, k) + vVn(i, j, k)) / rVt)
            vSqrSigmaN(i, j, k) = Sqr(rQ * localN * vDn(i, j, k) / rVt)
    End Select
Next k: Next j: Next i
End Sub

Sub CalculateGamma(ByVal nCarrier As Integer)
'Calculates gamma and its spatial derivatives for the specified carrier for current extracted at z = nNodesZ.
'Qgamma is zero at front surface, rear surface, and contacted nodes of the specified nCarrier
'Returns zeros for solution modes that do not extract current in the z axis.
'Gamma = Harmonic Gamma + Gamma1*(z/H) + 1/2 Gamma2*(z/H)^2
'Gamma1 and Gamma2 are fit to match the total surface recombination and the total G-R (including surface recombination)
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer, p As Integer
Dim rGamma1 As Double, rGamma2 As Double, rJtop As Double, rJbot As Double, sRec As Double
Dim vQ As Variant, fQ As Variant
Dim vGamma As Variant, fGamma As Variant
Dim vGrad As Variant, vGradGamma As Variant
ReDim vQ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGrad(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGamma(xAxis To zAxis, 1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim fQ(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
ReDim fGamma(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1, 1 To nHarmonicsZ + 1) As Double
ReDim vGamma(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGamma(xAxis To zAxis, 1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
'Don't attempt to compute Gamma for solution modes for which it is not relevant.
If nSolutionMode = lateral Or nCarrier = combined Then Exit Sub
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    vQ(i, j, k) = Qgamma(nCarrier, i, j, k)
Next k: Next j: Next i
fQ = MTrans3Dback(MMult3D(r2fZ, MTrans3Drp(MMult3D(r2fY, MTrans3Drc(MMult3D(r2fX, vQ))))))
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For p = 0 To nHarmonicsZ
    If kmnp2(m + 1, n + 1, p + 1) > 0 Then fGamma(m + 1, n + 1, p + 1) = -fQ(m + 1, n + 1, p + 1) / kmnp2(m + 1, n + 1, p + 1)
Next p: Next n: Next m
vGamma = MTrans3Dback(MMult3D(f2rZ, MTrans3Drp(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fGamma))))))
'Calculate gradients
vGrad = MTrans3Dback(MMult3D(f2rZ, MTrans3Drp(MMult3D(f2rY, MTrans3Drc(MMult3D(f2gX, fGamma))))))
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vGradGamma(xAxis, i, j, k) = vGrad(i, j, k)
Next k: Next j: Next i
vGrad = MTrans3Dback(MMult3D(f2rZ, MTrans3Drp(MMult3D(f2gY, MTrans3Drc(MMult3D(f2rX, fGamma))))))
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vGradGamma(yAxis, i, j, k) = vGrad(i, j, k)
Next k: Next j: Next i
vGrad = MTrans3Dback(MMult3D(f2gZ, MTrans3Drp(MMult3D(f2rY, MTrans3Drc(MMult3D(f2rX, fGamma))))))
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vGradGamma(zAxis, i, j, k) = vGrad(i, j, k)
Next k: Next j: Next i
'Calculate and apply linear and parabolic terms to match top and bottom z-axis current density
rJtop = TopCurrent(nCarrier)
rJbot = BottomCurrent(nCarrier)
Select Case nTextureType
    Case planar
        sRec = rSurfaceRecombination
    Case grooves
        sRec = 0.5 * sRecombination(1, 1) / rCosTextureAngle / (nNodesX - 1)
    Case pyramids
        sRec = 0.25 * sRecombination(1, 1) / rCosTextureAngle / ((nNodesX - 1) * (nNodesY - 1))
    Case inverted
        sRec = 0.25 * sRecombination(1, 1) / rCosTextureAngle / ((nNodesX - 1) * (nNodesY - 1))
End Select
Select Case nCarrier
    Case holes
        rGamma1 = deviceHeight * (rJtop + rQ * sRec)
        If nSolutionMode = bipolar And nBackgroundType = ntype Then rGamma1 = rGamma1 + deviceHeight * rQ * (rTotalGeneration - rTotalRecombination)
        rGamma2 = deviceHeight * rJbot - rGamma1
        If nSolutionMode = floating Or nSolutionMode = flatband Or _
          (nSolutionMode = bipolar And nBackgroundType = ptype) Then rGamma2 = rGamma2 - deviceHeight * rQ * (rTotalGeneration - rTotalRecombination)
    Case electrons
        rGamma1 = deviceHeight * (rJtop - rQ * sRec)
        If nSolutionMode = bipolar And nBackgroundType = ptype Then rGamma1 = rGamma1 - deviceHeight * rQ * (rTotalGeneration - rTotalRecombination)
        rGamma2 = deviceHeight * rJbot - rGamma1
        If nSolutionMode = floating Or nSolutionMode = flatband Or _
          (nSolutionMode = bipolar And nBackgroundType = ntype) Then rGamma2 = rGamma2 + deviceHeight * rQ * (rTotalGeneration - rTotalRecombination)
End Select
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vGamma(i, j, k) = vGamma(i, j, k) + rGamma1 * (k - 1) / (nNodesZ - 1) + 0.5 * rGamma2 * ((k - 1) / (nNodesZ - 1)) ^ 2
    vGradGamma(zAxis, i, j, k) = vGradGamma(zAxis, i, j, k) + rGamma1 / deviceHeight + rGamma2 * ((k - 1) / (nNodesZ - 1)) / deviceHeight
Next k: Next j: Next i
Select Case nCarrier
    Case holes: vGammaP = vGamma: vGradGammaP = vGradGamma
    Case electrons: vGammaN = vGamma: vGradGammaN = vGradGamma
End Select
End Sub

Sub CalculateTransforms()
Dim i As Integer, j As Integer, k As Integer, m As Integer, n As Integer, p As Integer
'Del-squared harmonic factor
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY: For p = 0 To nHarmonicsZ
    kmnp2(m + 1, n + 1, p + 1) = ((m * pi / deviceWidth) ^ 2 + (n * pi / deviceLength) ^ 2 + (p * pi / deviceHeight) ^ 2)
Next p: Next n: Next m
'Fourier transforms for harmonic representation
For i = 1 To nNodesX: For m = 0 To nHarmonicsX
    f2rX(i, m + 1) = Cos(pi * m * (i - 1) / (nNodesX - 1))
Next m: Next i
For j = 1 To nNodesY: For n = 0 To nHarmonicsY
    f2rY(j, n + 1) = Cos(pi * n * (j - 1) / (nNodesY - 1))
Next n: Next j
For k = 1 To nNodesZ: For p = 0 To nHarmonicsZ
    f2rZ(k, p + 1) = Cos(pi * p * (k - 1) / (nNodesZ - 1))
Next p: Next k
r2fX = Application.MInverse(f2rX)
r2fY = Application.MInverse(f2rY)
r2fZ = Application.MInverse(f2rZ)
'Gradient transforms for use with harmonic representation, includes sinc function for smoothing
For i = 1 To nNodesX: For m = 0 To nHarmonicsX
    f2gX(i, m + 1) = -m * pi / deviceWidth * Sin(pi * m * (i - 1) / (nNodesX - 1)) * Sinc(pi * m / nHarmonicsX)
Next m: Next i
For j = 1 To nNodesY: For n = 0 To nHarmonicsY
    f2gY(j, n + 1) = -n * pi / deviceLength * Sin(pi * n * (j - 1) / (nNodesY - 1)) * Sinc(pi * n / nHarmonicsY)
Next n: Next j
For k = 1 To nNodesZ: For p = 0 To nHarmonicsZ
    f2gZ(k, p + 1) = -p * pi / deviceHeight * Sin(pi * p * (k - 1) / (nNodesZ - 1)) * Sinc(pi * p / nHarmonicsZ)
Next p: Next k
'Transforms for arrow-plotting interpolation with nArrowNodes-1 resolution using linear interpolation via triangular functionals
For i = 1 To nArrowNodes: For m = 0 To nHarmonicsX
    r2aX(i, m + 1) = Tri((i - 1) / (nArrowNodes - 1) * nHarmonicsX - m)
Next m: Next i
For j = 1 To nArrowNodes: For n = 0 To nHarmonicsY
    r2aY(j, n + 1) = Tri((j - 1) / (nArrowNodes - 1) * nHarmonicsY - n)
Next n: Next j
For k = 1 To nArrowNodes: For p = 0 To nHarmonicsZ
    r2aZ(k, p + 1) = Tri((k - 1) / (nArrowNodes - 1) * nHarmonicsZ - p)
Next p: Next k
'Transforms to nPlotNodes-1 resolution using linear interpolation via triangular functionals
For i = 1 To nPlotNodes: For m = 0 To nHarmonicsX
    r2tX(i, m + 1) = Tri((i - 1) / (nPlotNodes - 1) * nHarmonicsX - m)
Next m: Next i
For j = 1 To nPlotNodes: For n = 0 To nHarmonicsY
    r2tY(j, n + 1) = Tri((j - 1) / (nPlotNodes - 1) * nHarmonicsY - n)
Next n: Next j
For k = 1 To nPlotNodes: For p = 0 To nHarmonicsZ
    r2tZ(k, p + 1) = Tri((k - 1) / (nPlotNodes - 1) * nHarmonicsZ - p)
Next p: Next k
End Sub

