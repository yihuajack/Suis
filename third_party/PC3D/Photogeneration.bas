Attribute VB_Name = "Photogeneration"
Option Explicit
Const rSmallDelta As Double = 0.001
Const nMaxPasses As Integer = 100

Sub SolveGeneration()
Dim vGenLambda As Variant
Dim genLambda As Double
Dim monoAlpha As Double, r0 As Double
Dim frontPower As Double, rearPower As Double, filter As Double
Dim frontFlux As Double, rearFlux As Double
Dim i As Integer, j As Integer, k As Integer, n As Integer
Dim nLambdaMin As Integer, nLambdaMax As Integer
Dim saveCurrentPlot As String
ReDim vGenLambda(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double

Call CalculateIlluminationUniformity(front, sFrontIllumination)
Call CalculateIlluminationUniformity(rear, sRearIllumination)
'Find min and max lambda entries in rSpectrumData. Assumes increasing lambda, but may not fill array space.
nLambdaMin = LBound(rSpectrumData, 1)
nLambdaMax = nLambdaMin
For n = 1 To UBound(rSpectrumData, 1)
    If rSpectrumData(n, 1) > rSpectrumData(nLambdaMax, 1) Then nLambdaMax = n
Next n
If bSpectrum Then
    For n = nLambdaMin To nLambdaMax
        ReDim vGenLambda(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
        filter = 0.01 * rSpectrumData(n, 4)
        frontPower = filter * rFrontSuns * rSpectrumData(n, 2) / 10000 'W/cm2
        rearPower = rRearSuns * rSpectrumData(n, 2) / 10000                         'W/cm2
        If rSpectrumData(n, 3) * deviceHeight * nMaxPasses > rSmallDelta Then
            Range(progressCell).Value = rSpectrumData(n, 1): DoEvents
            Call CalculatePhotogeneration(rSpectrumData(n, 1), rSpectrumData(n, 3), _
                frontPower, rearPower, sFrontIllumination, sRearIllumination, _
                genLambda, vGenLambda)
            rTotalGeneration = rTotalGeneration + genLambda
            vGeneration = MAdd3D(vGeneration, vGenLambda)
        End If
        If frontShape = "Point" And rFrontPointZ > 0 And frontPower > 0 Then 'Buried photogeneration
            r0 = 0.1 * rFrontPointDiameter / 2
            frontFlux = frontPower * rSpectrumData(n, 1) / (q * hc) * Pi * r0 ^ 2
            Call CalculateBuriedPhotogeneration(front, frontFlux, genLambda, vGenLambda)
            rTotalGeneration = rTotalGeneration + genLambda
            vGeneration = MAdd3D(vGeneration, vGenLambda)
        End If
        If rearShape = "Point" And rRearPointZ > 0 And rearPower > 0 Then 'Buried photogeneration
            r0 = 0.1 * rRearPointDiameter / 2
            rearFlux = rearPower * rSpectrumData(n, 1) / (q * hc) * Pi * r0 ^ 2
            Call CalculateBuriedPhotogeneration(rear, rearFlux, genLambda, vGenLambda)
            rTotalGeneration = rTotalGeneration + genLambda
            vGeneration = MAdd3D(vGeneration, vGenLambda)
        End If
    Next n
End If
If bMono Then
    ReDim vGenLambda(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
    monoAlpha = InterpolateColumns(rMonoLambda, 1, rSpectrumData, 3, rSpectrumData, True, False)
    filter = 0.01 * InterpolateColumns(rMonoLambda, 1, rSpectrumData, 4, rSpectrumData, False, False)
    If monoAlpha * deviceHeight * nMaxPasses > rSmallDelta Then
        Range(progressCell).Value = rMonoLambda: DoEvents
        Call CalculatePhotogeneration(rMonoLambda, monoAlpha, _
            filter * rFrontMonoPower, rRearMonoPower, sFrontIllumination, sRearIllumination, _
            genLambda, vGenLambda)
        rTotalGeneration = rTotalGeneration + genLambda
        vGeneration = MAdd3D(vGeneration, vGenLambda)
    End If
    If frontShape = "Point" And rFrontPointZ > 0 And rFrontMonoPower > 0 Then 'Buried photogeneration
        r0 = 0.1 * rFrontPointDiameter / 2
        frontFlux = filter * rFrontMonoPower * rMonoLambda / (q * hc) * Pi * r0 ^ 2
        Call CalculateBuriedPhotogeneration(front, frontFlux, genLambda, vGenLambda)
        rTotalGeneration = rTotalGeneration + genLambda
        vGeneration = MAdd3D(vGeneration, vGenLambda)
    End If
    If rearShape = "Point" And rRearPointZ > 0 And rRearMonoPower > 0 Then 'Buried photogeneration
        r0 = 0.1 * rRearPointDiameter / 2
        rearFlux = rRearMonoPower * rMonoLambda / (q * hc) * Pi * r0 ^ 2
        Call CalculateBuriedPhotogeneration(rear, rearFlux, genLambda, vGenLambda)
        rTotalGeneration = rTotalGeneration + genLambda
        vGeneration = MAdd3D(vGeneration, vGenLambda)
    End If
End If

Range(generationCell).Value = q * rTotalGeneration        'A/cm2
bGeneration = rTotalGeneration > 0
saveCurrentPlot = currentPlot
If bGeneration Then Call PlotGeneration
currentPlot = saveCurrentPlot
End Sub

Sub CalculatePhotogeneration(ByVal lambda As Double, ByVal alpha As Double, _
    ByVal frontPower As Double, ByVal rearPower As Double, _
    ByRef frontIllumination As Variant, ByRef rearIllumination As Variant, _
    ByRef photoGen As Double, ByRef vGen As Variant)
'This routine calcualtes the photogeneration rate as a function of location throughout the solution volume (3D).
'The result is stored in the argument vGen(i,j,k) passed by reference, which has units of cm-3/s.
'Variables local to this subroutine use Top and Bot in place of Front and Rear, which are used for Global variables.
'Front and Rear illumination power have units of W/cm2. Each is multipled by the spectral transmittance at the given lambda.
'The illumination power is further multiplied by the illumination uniformity factor and spatial transmittance at each location.
'Illumination uniformity (front and rear) is passed ByRef for expediency only - Do not modify it in this routine!
Dim i As Integer, j As Integer, k As Integer, m As Integer, n As Integer, iteration As Integer
Dim sTopSpec0 As Variant, sTopDiff0 As Variant, sBotSpec0 As Variant, sBotDiff0 As Variant
Dim sTopSpecIn As Variant, sBotSpecIn As Variant, sTopDiffIn As Variant, sBotDiffIn As Variant
Dim sTopSpecOut As Variant, sBotSpecOut As Variant, sTopDiffOut As Variant, sBotDiffOut As Variant
Dim fTopDiff As Variant, fBotDiff As Variant
Dim sMidDiff As Variant, fMidDiff As Variant       'These get reset for each value of z within the solution volume
Dim vGenRaw As Variant
Dim alphaD As Double, keff As Double, deltaZ As Double, zTop As Double, zBot As Double
Dim xGreen As Double, xGreenTop As Double, xGreenBot As Double      'Values from Green's lambertian transmission approximation
Dim gamma As Double, gammaPrime As Double
Dim gammaPrimeTopModifier As Double, gammaPrimeBotModifier As Double   'Values from Basore's extension to Green's approximation
Dim absorbSum As Double, absorbCalc As Double, absorbLocal As Double
Dim rDiffuse As Double, rSpecular As Double
Dim sumSource As Double, saveSource As Double 'Used to detect no further change in surface brightness due to light trapping
Dim fineTuneFactor As Double
Dim spectralTransmissionFront As Double, spectralTransmissionRear As Double
Dim nElementsXY As Integer

photoGen = 0
alphaD = alpha * deviceHeight
spectralTransmissionFront = InterpolateColumns(lambda, 1, rSpectralLambdas, 1, rSpectralFront, False, False) / 100
spectralTransmissionRear = InterpolateColumns(lambda, 1, rSpectralLambdas, 1, rSpectralRear, False, False) / 100
nElementsXY = (nNodesX - 1) * (nNodesY - 1)

ReDim sTopSpec0(1 To nNodesX, 1 To nNodesY), sTopDiff0(1 To nNodesX, 1 To nNodesY) As Double
ReDim sBotSpec0(1 To nNodesX, 1 To nNodesY), sBotDiff0(1 To nNodesX, 1 To nNodesY) As Double
ReDim sTopSpecIn(1 To nNodesX, 1 To nNodesY), sTopDiffIn(1 To nNodesX, 1 To nNodesY) As Double
ReDim sBotSpecIn(1 To nNodesX, 1 To nNodesY), sBotDiffIn(1 To nNodesX, 1 To nNodesY) As Double
ReDim sTopSpecOut(1 To nNodesX, 1 To nNodesY), sTopDiffOut(1 To nNodesX, 1 To nNodesY) As Double
ReDim sBotSpecOut(1 To nNodesX, 1 To nNodesY), sBotDiffOut(1 To nNodesX, 1 To nNodesY) As Double
ReDim fTopDiff(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim fBotDiff(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
ReDim sMidDiff(1 To nNodesX, 1 To nNodesY) As Double
ReDim vGenRaw(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGen(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double     'Return zeros if no calculation

'**** Initialize Specular Light ****
For i = 1 To nNodesX: For j = 1 To nNodesY
    sTopSpec0(i, j) = frontIllumination(i, j) * sFrontTransmission(i, j) * spectralTransmissionFront _
        * (1 - sFrontHaze(i, j)) * frontPower * lambda / (q * hc) 'cm-2/s
    sBotSpec0(i, j) = rearIllumination(i, j) * sRearTransmission(i, j) * spectralTransmissionRear _
        * (1 - sRearHaze(i, j)) * rearPower * lambda / (q * hc) 'cm-2/s
    sTopSpecOut(i, j) = sTopSpec0(i, j)
    sBotSpecOut(i, j) = sBotSpec0(i, j)
Next j: Next i

'**** Initialize Diffuse Light ****
For i = 1 To nNodesX: For j = 1 To nNodesY
    sTopDiff0(i, j) = frontIllumination(i, j) * sFrontTransmission(i, j) * spectralTransmissionFront _
        * sFrontHaze(i, j) * frontPower * lambda / (q * hc) 'cm-2/s
    sBotDiff0(i, j) = rearIllumination(i, j) * sRearTransmission(i, j) * spectralTransmissionRear _
        * sRearHaze(i, j) * rearPower * lambda / (q * hc) 'cm-2/s
    sTopDiffOut(i, j) = sTopDiff0(i, j)
    sBotDiffOut(i, j) = sBotDiff0(i, j)
Next j: Next i

xGreen = 0.935 * Application.WorksheetFunction.Power(alphaD, 0.67)  'Same for all iterations
deltaZ = deviceHeight / (nNodesZ - 1)

For iteration = 1 To nMaxPasses  'Maximum round-trip passes of the light. Most cases will exit this loop much sooner.
saveSource = 0
For i = 1 To nNodesX: For j = 1 To nNodesY
    saveSource = saveSource + sTopSpecOut(i, j) + sTopDiffOut(i, j) + sBotSpecOut(i, j) + sBotDiffOut(i, j)
Next j: Next i

If saveSource = 0 Then
    photoGen = 0
    Exit Sub     'No point in calculating if there's no light
End If

'Calculate light trapping augmentation
   
    'Calculate front specular flux reaching Bot surface at each node
For i = 1 To nNodesX: For j = 1 To nNodesY
    If alphaD < 10 Then  'Won't underflow exponential
        sBotSpecIn(i, j) = sTopSpecOut(i, j) * Exp(-alphaD)
    Else: sBotSpecIn(i, j) = 0
    End If
Next j: Next i
   
   'Calculate front diffuse flux reaching Bot surface at each node
fTopDiff = Application.Transpose(Application.MMult(r2fY, Application.Transpose(Application.MMult(r2fX, sTopDiffOut))))
For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
    keff = Pi * Sqr((m / deviceWidth) ^ 2 + (n / deviceLength) ^ 2)
    gamma = (2 + xGreen) / (1 + xGreen + 0.4 * keff * deviceHeight)
    If gamma * alphaD < 10 And keff * deviceHeight < 10 Then
        fBotDiff(m + 1, n + 1) = fTopDiff(m + 1, n + 1) * (2 / (1 + Exp(0.91 * keff * deviceHeight))) * Exp(-gamma * alphaD)
    Else: fBotDiff(m + 1, n + 1) = 0
    End If
Next n: Next m

sBotDiffIn = Application.MMult(f2rX, Application.Transpose(Application.MMult(f2rY, Application.Transpose(fBotDiff))))
   
   'Adjust fluxes leaving Bot surface to incorporate reflected fluxes
gamma = (2 + xGreen) / (1 + xGreen)
For i = 1 To nNodesX: For j = 1 To nNodesY
    rSpecular = sBotSpecIn(i, j) * sRearSpecReflect(i, j) * (1 - sRearHaze(i, j)) + sBotDiffIn(i, j) * sRearSpecReflect(i, j) * (2 - gamma) * (1 - sRearHaze(i, j))
    rDiffuse = sBotSpecIn(i, j) * sRearDiffReflect(i, j) * sRearHaze(i, j) + sBotDiffIn(i, j) * sRearDiffReflect(i, j) * ((gamma - 1) + sRearHaze(i, j) * (2 - gamma))
    sBotSpecOut(i, j) = sBotSpec0(i, j) + rSpecular
    sBotDiffOut(i, j) = sBotDiff0(i, j) + rDiffuse
Next j: Next i
   
   'Calculate specular flux from Bot reaching Top surface at each node
For i = 1 To nNodesX: For j = 1 To nNodesY
    If alphaD < 10 Then  'Won't underflow exponential
        sTopSpecIn(i, j) = sBotSpecOut(i, j) * Exp(-alphaD)
    Else: sTopSpecIn(i, j) = 0
    End If
Next j: Next i
   
   'Calculate diffuse flux from Bot reaching Top surface at each node
fBotDiff = Application.Transpose(Application.MMult(r2fY, Application.Transpose(Application.MMult(r2fX, sBotDiffOut))))

For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
    keff = Pi * Sqr((m / deviceWidth) ^ 2 + (n / deviceLength) ^ 2)
    gamma = (2 + xGreen) / (1 + xGreen + 0.4 * keff * deviceHeight)
    If gamma * alphaD < 10 And keff * deviceHeight < 10 Then
        fTopDiff(m + 1, n + 1) = fBotDiff(m + 1, n + 1) * (2 / (1 + Exp(0.91 * keff * deviceHeight))) * Exp(-gamma * alphaD)
    Else: fTopDiff(m + 1, n + 1) = 0
    End If
Next n: Next m
   
sTopDiffIn = Application.MMult(f2rX, Application.Transpose(Application.MMult(f2rY, Application.Transpose(fTopDiff))))
   
   'Adjust fluxes leaving Top surface to incorporate reflected fluxes
gamma = (2 + xGreen) / (1 + xGreen)
For i = 1 To nNodesX: For j = 1 To nNodesY
    rSpecular = sTopSpecIn(i, j) * sFrontSpecReflect(i, j) * (1 - sFrontHaze(i, j)) + sTopDiffIn(i, j) * sFrontSpecReflect(i, j) * (2 - gamma) * (1 - sFrontHaze(i, j))
    rDiffuse = sTopSpecIn(i, j) * sFrontDiffReflect(i, j) * sFrontHaze(i, j) + sTopDiffIn(i, j) * sFrontDiffReflect(i, j) * ((gamma - 1) + sFrontHaze(i, j) * (2 - gamma))
    sTopSpecOut(i, j) = sTopSpec0(i, j) + rSpecular
    sTopDiffOut(i, j) = sTopDiff0(i, j) + rDiffuse
Next j: Next i
   
sumSource = 0
For i = 1 To nNodesX: For j = 1 To nNodesY
    sumSource = sumSource + sTopSpecOut(i, j) + sTopDiffOut(i, j) + sBotSpecOut(i, j) + sBotDiffOut(i, j)
Next j: Next i

If Abs(sumSource - saveSource) < rSmallDelta * saveSource Then Exit For

Next iteration


'**** Calculate photogeneration at each node *****

'   ***Specular Light ***
    
    'Calculate expected total specular generation due to front surface
absorbCalc = 0
For i = 1 To nNodesX: For j = 1 To nNodesY
    absorbLocal = sTopSpecOut(i, j)
    If (i = 1 Or i = nNodesX) Then absorbLocal = 0.5 * absorbLocal
    If (j = 1 Or j = nNodesY) Then absorbLocal = 0.5 * absorbLocal
    absorbCalc = absorbCalc + absorbLocal
Next j: Next i
absorbCalc = absorbCalc * (1 - Exp(-alphaD)) / nElementsXY
photoGen = photoGen + absorbCalc

    'Determine specular photogeneration at each node within the device due to front surface brightness
absorbSum = 0
ReDim vGenRaw(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For k = 1 To nNodesZ
    zTop = (k - 1) * deltaZ
    If Exp(-alpha * zTop) > rSmallDelta Then
        For i = 1 To nNodesX: For j = 1 To nNodesY
            absorbLocal = sTopSpecOut(i, j) * alpha * Exp(-alpha * zTop)
            If absorbLocal < rSmallDelta * saveSource * alpha / nElementsXY Then absorbLocal = 0
            vGenRaw(i, j, k) = absorbLocal
            If (i = 1 Or i = nNodesX) Then absorbLocal = 0.5 * absorbLocal
            If (j = 1 Or j = nNodesY) Then absorbLocal = 0.5 * absorbLocal
            If k = 1 Or k = nNodesZ Then absorbLocal = 0.5 * absorbLocal
            absorbSum = absorbSum + absorbLocal
            Next j: Next i
    End If
Next k

    'Scale photogeneration so that volume total matches calculated. This also corrects for rapid exponential variation in z
absorbSum = absorbSum * deltaZ / nElementsXY
If absorbSum > 0 Then fineTuneFactor = absorbCalc / absorbSum Else fineTuneFactor = 1
For k = 1 To nNodesZ
    zTop = (k - 1) * deltaZ
    If Exp(-alpha * zTop) > rSmallDelta Then
        For i = 1 To nNodesX: For j = 1 To nNodesY
            vGen(i, j, k) = vGen(i, j, k) + vGenRaw(i, j, k) * fineTuneFactor
        Next j: Next i
    End If
Next k

    'Calculate expected total specular generation due to rear surface
absorbCalc = 0
For i = 1 To nNodesX: For j = 1 To nNodesY
    absorbLocal = sBotSpecOut(i, j)
    If (i = 1 Or i = nNodesX) And nNodesX <> 1 Then absorbLocal = 0.5 * absorbLocal
    If (j = 1 Or j = nNodesY) And nNodesY <> 1 Then absorbLocal = 0.5 * absorbLocal
    absorbCalc = absorbCalc + absorbLocal
Next j: Next i
absorbCalc = absorbCalc * (1 - Exp(-alphaD)) / nElementsXY
photoGen = photoGen + absorbCalc

    'Determine specular photogeneration at each node within the device due to rear surface brightness
absorbSum = 0
ReDim vGenRaw(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For k = 1 To nNodesZ
    zBot = (nNodesZ - k) * deltaZ
    If Exp(-alpha * zBot) > rSmallDelta Then
        For i = 1 To nNodesX: For j = 1 To nNodesY
            absorbLocal = sBotSpecOut(i, j) * alpha * Exp(-alpha * zBot)
            If absorbLocal < rSmallDelta * saveSource * alpha / nElementsXY Then absorbLocal = 0
            vGenRaw(i, j, k) = absorbLocal
            If (i = 1 Or i = nNodesX) And nNodesX <> 1 Then absorbLocal = 0.5 * absorbLocal
            If (j = 1 Or j = nNodesY) And nNodesY <> 1 Then absorbLocal = 0.5 * absorbLocal
            If k = 1 Or k = nNodesZ Then absorbLocal = 0.5 * absorbLocal
            absorbSum = absorbSum + absorbLocal
            Next j: Next i
    End If
Next k

    'Scale photogeneration so that volume total matches calculated. This also corrects for rapid exponential variation in z
absorbSum = absorbSum * deltaZ / nElementsXY
If absorbSum > 0 Then fineTuneFactor = absorbCalc / absorbSum Else fineTuneFactor = 1
For k = 1 To nNodesZ
    zBot = (nNodesZ - k) * deltaZ
    If Exp(-alpha * zBot) > rSmallDelta Then
        For i = 1 To nNodesX: For j = 1 To nNodesY
            vGen(i, j, k) = vGen(i, j, k) + vGenRaw(i, j, k) * fineTuneFactor
        Next j: Next i
    End If
Next k

'    *** Diffuse light ***
fTopDiff = Application.Transpose(Application.MMult(r2fY, Application.Transpose(Application.MMult(r2fX, sTopDiffOut))))
fBotDiff = Application.Transpose(Application.MMult(r2fY, Application.Transpose(Application.MMult(r2fX, sBotDiffOut))))

    'Calculate expected total diffuse generation due to front surface (only need to consider n=0, since n>0 has net zero generation)
gamma = (2 + xGreen) / (1 + xGreen)
If gamma * alphaD < 10 Then
    absorbCalc = fTopDiff(1, 1) * (1 - Exp(-gamma * alphaD))
Else: absorbCalc = fTopDiff(1, 1)
End If
photoGen = photoGen + absorbCalc

    'Determine diffuse photogeneration at each node within the device due to front surface brightness
absorbSum = 0
ReDim vGenRaw(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For k = 1 To nNodesZ
    zTop = (k - 1) * deltaZ
    If Exp(-alpha * zTop) > rSmallDelta Then
        ReDim fMidDiff(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double 'Sets all values to zero
        xGreenTop = 0.935 * Application.WorksheetFunction.Power(alpha * zTop, 0.67)  'Same for all harmonics
        gammaPrimeTopModifier = 0.67 * xGreenTop / (1 + xGreenTop) ^ 2
        For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
            keff = Pi * Sqr((m / deviceWidth) ^ 2 + (n / deviceLength) ^ 2)
            gamma = (2 + xGreenTop) / (1 + xGreenTop + 0.47 * keff * zTop)
            gammaPrime = gamma - gammaPrimeTopModifier
            If alpha * zTop < 10 And keff * zTop < 10 Then
                fMidDiff(m + 1, n + 1) = fMidDiff(m + 1, n + 1) + alpha * fTopDiff(m + 1, n + 1) _
                    * gammaPrime * (2 / (1 + Exp(0.88 * keff * zTop))) * Exp(-gamma * alpha * zTop)
            End If
            Next n: Next m
        sMidDiff = Application.MMult(f2rX, Application.Transpose(Application.MMult(f2rY, Application.Transpose(fMidDiff))))
        For i = 1 To nNodesX: For j = 1 To nNodesY
            absorbLocal = sMidDiff(i, j)
            If absorbLocal < rSmallDelta * saveSource * alpha / nElementsXY Then absorbLocal = 0
            vGenRaw(i, j, k) = absorbLocal
            If (i = 1 Or i = nNodesX) And nNodesX <> 1 Then absorbLocal = 0.5 * absorbLocal
            If (j = 1 Or j = nNodesY) And nNodesY <> 1 Then absorbLocal = 0.5 * absorbLocal
            If k = 1 Or k = nNodesZ Then absorbLocal = 0.5 * absorbLocal
            absorbSum = absorbSum + absorbLocal
            Next j: Next i
    End If
Next k
    
'Scale photogeneration so that volume total matches calculated. This also corrects for rapid exponential variation in z
absorbSum = absorbSum * deltaZ / nElementsXY
      
If absorbSum > 0 Then fineTuneFactor = absorbCalc / absorbSum Else fineTuneFactor = 1
For k = 1 To nNodesZ
    zTop = (k - 1) * deltaZ
    If Exp(-alpha * zTop) > rSmallDelta Then
        For i = 1 To nNodesX: For j = 1 To nNodesY
            vGen(i, j, k) = vGen(i, j, k) + vGenRaw(i, j, k) * fineTuneFactor
        Next j: Next i
    End If
Next k

    'Calculate expected total diffuse generation due to rear surface (only need to consider n=0, since n>0 has net zero generation)
gamma = (2 + xGreen) / (1 + xGreen)
If gamma * alphaD < 10 Then
    absorbCalc = fBotDiff(1, 1) * (1 - Exp(-gamma * alphaD))
Else: absorbCalc = fBotDiff(1, 1)
End If
photoGen = photoGen + absorbCalc

    'Determine diffuse photogeneration at each node within the device due to rear surface brightness
absorbSum = 0
ReDim vGenRaw(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For k = 1 To nNodesZ
    zBot = (nNodesZ - k) * deltaZ
    If Exp(-alpha * zBot) > rSmallDelta Then
        ReDim fMidDiff(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double 'Sets all values to zero
        xGreenBot = 0.935 * Application.WorksheetFunction.Power(alpha * zBot, 0.67)
        gammaPrimeBotModifier = 0.67 * xGreenBot / (1 + xGreenBot) ^ 2
        For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
            keff = Pi * Sqr((m / deviceWidth) ^ 2 + (n / deviceLength) ^ 2)
            gamma = (2 + xGreenBot) / (1 + xGreenBot + 0.47 * keff * zBot)
            gammaPrime = gamma - gammaPrimeBotModifier
            If alpha * zBot < 10 And keff * zBot < 10 Then
                fMidDiff(m + 1, n + 1) = fMidDiff(m + 1, n + 1) + alpha * fBotDiff(m + 1, n + 1) _
                    * gammaPrime * (2 / (1 + Exp(0.88 * keff * zBot))) * Exp(-gamma * alpha * zBot)
            End If
            Next n: Next m
        sMidDiff = Application.MMult(f2rX, Application.Transpose(Application.MMult(f2rY, Application.Transpose(fMidDiff))))
        For i = 1 To nNodesX: For j = 1 To nNodesY
            absorbLocal = sMidDiff(i, j)
            If absorbLocal < rSmallDelta * saveSource * alpha / nElementsXY Then absorbLocal = 0
            vGenRaw(i, j, k) = absorbLocal
            If (i = 1 Or i = nNodesX) And nNodesX <> 1 Then absorbLocal = 0.5 * absorbLocal
            If (j = 1 Or j = nNodesY) And nNodesY <> 1 Then absorbLocal = 0.5 * absorbLocal
            If k = 1 Or k = nNodesZ Then absorbLocal = 0.5 * absorbLocal
            absorbSum = absorbSum + absorbLocal
            Next j: Next i
    End If
Next k

'Scale photogeneration so that volume total matches calculated. This also corrects for rapid exponential variation in z
absorbSum = absorbSum * deltaZ / nElementsXY
      
If absorbSum > 0 Then fineTuneFactor = absorbCalc / absorbSum Else fineTuneFactor = 1
For k = 1 To nNodesZ
    zBot = (nNodesZ - k) * deltaZ
    If Exp(-alpha * zBot) > rSmallDelta Then
        For i = 1 To nNodesX: For j = 1 To nNodesY
            vGen(i, j, k) = vGen(i, j, k) + vGenRaw(i, j, k) * fineTuneFactor
        Next j: Next i
    End If
Next k

End Sub

Sub CalculateBuriedPhotogeneration(ByVal nSide As Integer, ByVal rFlux As Double, _
                                   ByRef photoGen As Double, ByRef vGen As Variant)
'Spherical symmetry photogeneration with 1/e diameter given by rPointDiameter with total photogeneration equalling rFlux
Dim i As Integer, j As Integer, k As Integer
Dim localGen As Double, maxGen As Double, effectiveVolume As Double
Dim x As Double, y As Double, z As Double, r As Double
Dim x0 As Double, y0 As Double, z0 As Double, r0 As Double
ReDim vGen(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
photoGen = 0
If nSide = front Then
    r0 = 0.1 * rFrontPointDiameter / 2
    x0 = rFrontPointX * deviceWidth / 100
    y0 = (100 - rFrontPointY) * deviceLength / 100
    z0 = rFrontPointZ * deviceHeight / 100
Else
    r0 = 0.1 * rRearPointDiameter / 2
    x0 = rRearPointX * deviceWidth / 100
    y0 = (100 - rRearPointY) * deviceLength / 100
    z0 = (100 - rRearPointZ) * deviceHeight / 100
End If
effectiveVolume = Pi ^ 1.5 * r0 ^ 3        'Definite integral of spherical gaussian distribution
If effectiveVolume = 0 Then effectiveVolume = deviceWidth * deviceLength * deviceHeight / ((nNodesX - 1) * (nNodesY - 1) * (nNodesZ - 1))
If rFlux > 0 Then
    If x0 = 0 Or x0 = deviceWidth Then rFlux = 2 * rFlux
    If y0 = 0 Or y0 = deviceLength Then rFlux = 2 * rFlux
    If z0 = 0 Or z0 = deviceHeight Then rFlux = 2 * rFlux
    maxGen = rFlux / effectiveVolume
    For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
        x = (i - 1) / (nNodesX - 1) * deviceWidth
        y = (nNodesY - j) / (nNodesY - 1) * deviceLength
        z = (k - 1) / (nNodesZ - 1) * deviceHeight
        r = Sqr((x - x0) ^ 2 + (y - y0) ^ 2 + (z - z0) ^ 2)
        localGen = maxGen * Exp(-(r / r0) ^ 2)
        vGen(i, j, k) = localGen
        If i = 1 Or i = nNodesX Then localGen = 0.5 * localGen
        If j = 1 Or j = nNodesY Then localGen = 0.5 * localGen
        If k = 1 Or k = nNodesZ Then localGen = 0.5 * localGen
        photoGen = photoGen + localGen
    Next k: Next j: Next i
End If
photoGen = photoGen * deviceWidth * deviceLength * deviceHeight / ((nNodesX - 1) * (nNodesY - 1) * (nNodesZ - 1))
'Adjust vGen so that total generation exactly matches incident flux
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    If photoGen > 0 Then vGen(i, j, k) = vGen(i, j, k) * rFlux / photoGen
Next k: Next j: Next i
photoGen = rFlux / (deviceWidth * deviceLength)        'Convert to per cm-2/s
End Sub

