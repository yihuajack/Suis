Attribute VB_Name = "Photogeneration"
Option Explicit

Sub SolveGeneration()
Dim frontPower As Double, reflectedFraction As Double, absorbedFraction As Double
Dim n As Integer, nLambdaMin As Integer, nLambdaMax As Integer
Dim vGenLambda As Variant
ReDim vGenLambda(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGeneration(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double

'Find min and max lambda entries in rSpectrumData. Assumes increasing lambda, but may not fill array space.
nLambdaMin = LBound(rSpectrumData, 1)
nLambdaMax = nLambdaMin
For n = 1 To UBound(rSpectrumData, 1)
    If rSpectrumData(n, 1) > rSpectrumData(nLambdaMax, 1) Then nLambdaMax = n
Next n
If bSpectrum Then
    For n = nLambdaMin To nLambdaMax
        ReDim vGenLambda(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
        frontPower = rFrontSuns * rSpectrumData(n, 2) / 10000  'W/cm2
        Range(progressCell).Value = rSpectrumData(n, 1): DoEvents
        Call CalculatePhotogeneration(rSpectrumData(n, 1), frontPower, reflectedFraction, absorbedFraction, vGenLambda)
        vGeneration = MAdd3D(vGeneration, vGenLambda)
    Next n
End If
If bMono Then
    ReDim vGenLambda(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
    Range(progressCell).Value = rMonoLambda: DoEvents
    Call CalculatePhotogeneration(rMonoLambda, rFrontMonoPower, reflectedFraction, absorbedFraction, vGenLambda)
    vGeneration = MAdd3D(vGeneration, vGenLambda)
End If
rTotalGeneration = VolumeAverage(vGeneration) * deviceHeight
Range(generationCell).Value = rQ * rTotalGeneration
End Sub

Sub CalculatePhotogeneration(ByVal rLambda As Double, ByVal rPower As Double, _
                                                    ByRef reflectedFraction As Double, ByRef absorbedFraction As Double, ByRef vGen As Variant)
'This routine calculates the photogeneration rate at each node throughout the solution volume.
'The result is stored in the argument vGen(i,j,k) passed by reference (cm-3/s).
'Front illumination power has units of W per cm2 in the x-y plane at the given free-space wavelength, lambda (nm).
'If the total generation exceeds the incident flux times the transmitted fraction then generation is scaled down proportionally.
'Also returns the total flux transmitted into the silicon. This can be used to calculate the spectral transmission at the given wavelength.
Dim i As Integer, j As Integer, k As Integer        'Location in the volume
Dim x0 As Double, y0 As Double, z0 As Double
Dim xSurface As Double, ySurface As Double, zSurface As Double     'Location on the surface (cm) -W<x<W, 0<y<2W, 0<z<H
Dim incidentFluxTE As Double, incidentFluxTM As Double, siliconFlux As Double, absorbedFlux As Double, reflectedFlux As Double
Dim photoGen As Double, localGen As Double, rDistance As Double
Dim zMin As Double      'Minimum z on surface that is illuminated with reflected light
Dim reflectedFractionTE As Double, reflectedFractionTM As Double
Dim transmittedFractionTE As Double, transmittedFractionTM As Double, transmittedFraction As Double
Dim incidentAngle As Double, transmittedAngle As Double, siliconAngle As Double     'radians
Dim rIndexSi As Double, rAlphaSi As Double
Dim vGen1 As Variant, vGen2 As Variant
ReDim vGen1(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGen2(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
rAlphaSi = InterpolateColumns(rLambda, 1, rSpectrumData, 3, rSpectrumData, False, False)
rIndexSi = InterpolateColumns(rLambda, 1, rSpectrumData, 4, rSpectrumData, False, False)
If rAlphaSi = 0 Then
    reflectedFraction = 0
    absorbedFraction = 0
    Exit Sub
End If
'*** Primary incidence ***
incidentAngle = textureAngle
incidentFluxTE = 0.5 * rPower * rLambda / (rQ * hc)
incidentFluxTM = 0.5 * rPower * rLambda / (rQ * hc)
Call ARC(TE, incidentAngle, rLambda, reflectedFractionTE, transmittedFractionTE)
Call ARC(TM, incidentAngle, rLambda, reflectedFractionTM, transmittedFractionTM)
reflectedFlux = incidentFluxTE * reflectedFractionTE + incidentFluxTM * reflectedFractionTM
absorbedFlux = incidentFluxTE * (1 - reflectedFractionTE - transmittedFractionTE) + incidentFluxTM * (1 - reflectedFractionTM - transmittedFractionTM)
siliconFlux = incidentFluxTE * transmittedFractionTE + incidentFluxTM * transmittedFractionTM
transmittedAngle = WorksheetFunction.Asin(rEncapsulationIndex / rIndexSi * Sin(incidentAngle))
siliconAngle = textureAngle - transmittedAngle
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    x0 = (i - 1) / (nNodesX - 1) * deviceWidth
    y0 = (j - 1) / (nNodesY - 1) * deviceLength
    z0 = (k - 1) / (nNodesZ - 1) * deviceHeight
    localGen = 0
    If rAlphaSi * deviceHeight > 0.000001 Then   'No need to calculate generation if absorption is negligible
        Select Case nTextureType
            Case planar
                rDistance = (k - 1) * rDeltaZ
                localGen = localGen + siliconFlux * rCosTextureAngle * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
            Case grooves
                If OriginOnSurface(x0, y0, z0, siliconAngle, 0, xSurface, ySurface, zSurface) Then
                    rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                    localGen = localGen + siliconFlux * rCosTextureAngle * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                End If
                If OriginOnSurface(x0, y0, z0, siliconAngle, pi, xSurface, ySurface, zSurface) Then
                    rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                    localGen = localGen + siliconFlux * rCosTextureAngle * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                End If
            Case Else
                If OriginOnSurface(x0, y0, z0, siliconAngle, 0, xSurface, ySurface, zSurface) Then
                    rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                    localGen = localGen + siliconFlux * rCosTextureAngle * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                End If
                If OriginOnSurface(x0, y0, z0, siliconAngle, pi / 2, xSurface, ySurface, zSurface) Then
                    rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                    localGen = localGen + siliconFlux * rCosTextureAngle * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                End If
                If OriginOnSurface(x0, y0, z0, siliconAngle, pi, xSurface, ySurface, zSurface) Then
                    rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                    localGen = localGen + siliconFlux * rCosTextureAngle * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                End If
                If OriginOnSurface(x0, y0, z0, siliconAngle, 3 * pi / 2, xSurface, ySurface, zSurface) Then
                    rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                    localGen = localGen + siliconFlux * rCosTextureAngle * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                End If
        End Select
    End If
    vGen1(i, j, k) = localGen
Next k: Next j: Next i
photoGen = VolumeAverage(vGen1) * deviceHeight
If photoGen > siliconFlux Then vGen1 = MScale3D(vGen1, siliconFlux / photoGen)  'Account for complete absorption at short lambda
'*** Secondary Incidence ***
If textureAngle > pi / 6 Then    'Secondary incidence only occurs for texture angle exceeding 30 degrees
    incidentFluxTE = 0.5 * rPower * reflectedFractionTE * rLambda / (rQ * hc)   'per cm2 of x-y area
    incidentFluxTM = 0.5 * rPower * reflectedFractionTM * rLambda / (rQ * hc)   'per cm2 of x-y area
    incidentAngle = pi - 3 * textureAngle
    transmittedAngle = WorksheetFunction.Asin(rEncapsulationIndex / rIndexSi * Sin(incidentAngle))
    siliconAngle = textureAngle - transmittedAngle
    Call ARC(TE, incidentAngle, rLambda, reflectedFractionTE, transmittedFractionTE)
    Call ARC(TM, incidentAngle, rLambda, reflectedFractionTM, transmittedFractionTM)
    absorbedFlux = absorbedFlux + incidentFluxTE * (1 - reflectedFractionTE - transmittedFractionTE) + incidentFluxTM * (1 - reflectedFractionTM - transmittedFractionTM)
    reflectedFlux = incidentFluxTE * reflectedFractionTE + incidentFluxTM * reflectedFractionTM
    siliconFlux = incidentFluxTE * transmittedFractionTE + incidentFluxTM * transmittedFractionTM 'Flux density per cm2 of x-y area entering silicon for this secondary incidence
    'Only z>zMin is illuminated with secondary incidence for texture angles >45 degrees
    If textureAngle > pi / 4 Then zMin = 2 * textureDepth / (1 - Tan(textureAngle) * Tan(2 * textureAngle))
    For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
        x0 = (i - 1) / (nNodesX - 1) * deviceWidth
        y0 = (j - 1) / (nNodesY - 1) * deviceLength
        z0 = (k - 1) / (nNodesZ - 1) * deviceHeight
        localGen = 0
        If rAlphaSi * deviceHeight > 0.000001 Then   'No need to calculate generation if absorption is negligible
            Select Case nTextureType
                Case grooves
                    If OriginOnSurface(x0, y0, z0, siliconAngle, 0, xSurface, ySurface, zSurface) Then
                        If zSurface > zMin Then
                            rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                            localGen = localGen + siliconFlux * Cos(incidentAngle) * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                        End If
                    End If
                    If OriginOnSurface(x0, y0, z0, siliconAngle, pi, xSurface, ySurface, zSurface) Then
                        If zSurface > zMin Then
                            rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                            localGen = localGen + siliconFlux * Cos(incidentAngle) * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                        End If
                    End If
                Case Else
                    If OriginOnSurface(x0, y0, z0, siliconAngle, 0, xSurface, ySurface, zSurface) Then
                        If zSurface > zMin Then
                            rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                            localGen = localGen + siliconFlux * Cos(incidentAngle) * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                        End If
                    End If
                    If OriginOnSurface(x0, y0, z0, siliconAngle, pi / 2, xSurface, ySurface, zSurface) Then
                        If zSurface > zMin Then
                            rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                            localGen = localGen + siliconFlux * Cos(incidentAngle) * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                        End If
                    End If
                    If OriginOnSurface(x0, y0, z0, siliconAngle, pi, xSurface, ySurface, zSurface) Then
                        If zSurface > zMin Then
                            rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                            localGen = localGen + siliconFlux * Cos(incidentAngle) * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                        End If
                    End If
                    If OriginOnSurface(x0, y0, z0, siliconAngle, 3 * pi / 2, xSurface, ySurface, zSurface) Then
                        If zSurface > zMin Then
                            rDistance = Sqr((x0 - xSurface) ^ 2 + (y0 - ySurface) ^ 2 + (z0 - zSurface) ^ 2)
                            localGen = localGen + siliconFlux * Cos(incidentAngle) * rAlphaSi * ExpPower(-rAlphaSi * rDistance)
                        End If
                    End If
            End Select
        End If
        vGen2(i, j, k) = localGen
    Next k: Next j: Next i
    photoGen = VolumeAverage(vGen2) * deviceHeight
    If photoGen > siliconFlux Then vGen2 = MScale3D(vGen2, siliconFlux / photoGen)  'Account for complete absorption at short lambda
End If
absorbedFraction = absorbedFlux / (rPower * rLambda / (rQ * hc))
reflectedFraction = reflectedFlux / (rPower * rLambda / (rQ * hc))
vGen = MAdd3D(vGen1, vGen2)
End Sub

Function OriginOnSurface(ByVal x0 As Double, ByVal y0 As Double, ByVal z0 As Double, ByVal theta As Double, ByVal phi As Double, _
                                            ByRef xSurface As Double, ByRef ySurface As Double, ByRef zSurface As Double) As Boolean
'Returns the point (xSurface, ySurface, zSurface) in cm from which a ray with polar-coordinate angles theta and phi (radians) reaches location i, j, k
'The surface range searched is -W to +W for grooves, -W to +W and 0 to 2L for pyramids, and -W to +W and -L to +L for inverted
'zSurface is in the range 0 to textureDepth
'Function returns true if there is a matching point of origin on the surface, false if not.
OriginOnSurface = True
Do While phi < 0: phi = phi + 2 * pi: Loop
Do While phi > 2 * pi: phi = phi - 2 * pi: Loop
Select Case nTextureType
'For each facet plane, find the ray's intersection point then test to see if the point of intersection is on the textured surface
    Case planar
        xSurface = x0: ySurface = y0: zSurface = 0
        If Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
    Case grooves
        Select Case Round(phi / pi, 0)
            Case Is = 0
                'Facet z = -x tan(alpha)   {left-facing}
                xSurface = (x0 - z0 * Tan(theta) * Cos(phi)) / (1 + Tan(textureAngle) * Tan(theta) * Cos(phi))
                zSurface = -xSurface * Tan(textureAngle)
                ySurface = y0 - (z0 - zSurface) * Tan(theta) * Sin(phi)
                If Round(zSurface / textureDepth, 3) >= 0 And Round(zSurface / textureDepth, 3) <= 1 And _
                    Round(xSurface / deviceWidth, 3) <= 0 And _
                    Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
            Case Is = 1
                'Facet z = x tan(alpha)  {right-facing}
                xSurface = (x0 - z0 * Tan(theta) * Cos(phi)) / (1 - Tan(textureAngle) * Tan(theta) * Cos(phi))
                zSurface = xSurface * Tan(textureAngle)
                ySurface = y0 - (z0 - zSurface) * Tan(theta) * Sin(phi)
                If Round(zSurface / textureDepth, 3) >= 0 And Round(zSurface / textureDepth, 3) <= 1 And _
                    Round(xSurface / deviceWidth, 3) >= 0 And _
                    Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
        End Select
    Case pyramids
        Select Case Round(phi / (pi / 2))
            Case Is = 0
                'Facet z = -x tan(alpha) {left-facing}
                xSurface = (x0 - z0 * Tan(theta) * Cos(phi)) / (1 + Tan(textureAngle) * Tan(theta) * Cos(phi))
                zSurface = -xSurface * Tan(textureAngle)
                ySurface = y0 - (z0 - zSurface) * Tan(theta) * Sin(phi)
                If Round(zSurface / textureDepth, 3) >= 0 And Round(zSurface / textureDepth, 3) <= 1 And _
                    Round(ySurface / deviceLength, 3) >= Round(1 + xSurface / deviceWidth, 3) And _
                    Round(ySurface / deviceLength, 3) <= Round(1 - xSurface / deviceWidth, 3) And _
                    Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
            Case Is = 1
                'Facet z = (deviceLength - y) tan(alpha)  {rear-facing}
                ySurface = (y0 - (z0 - textureDepth) * Tan(theta) * Sin(phi)) / (1 + Tan(textureAngle) * Tan(theta) * Sin(phi))
                zSurface = (deviceLength - ySurface) * Tan(textureAngle)
                xSurface = x0 - (z0 - zSurface) * Tan(theta) * Cos(phi)
                If Round(zSurface / textureDepth, 3) >= 0 And Round(zSurface / textureDepth, 3) <= 1 And _
                    Round(xSurface / deviceWidth, 3) >= Round(-1 + ySurface / deviceLength, 3) And _
                    Round(xSurface / deviceWidth, 3) <= Round(1 - ySurface / deviceLength, 3) And _
                    Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
            Case Is = 2
                'Facet z = x tan(alpha) {right-facing}
                xSurface = (x0 - z0 * Tan(theta) * Cos(phi)) / (1 - Tan(textureAngle) * Tan(theta) * Cos(phi))
                zSurface = xSurface * Tan(textureAngle)
                ySurface = y0 - (z0 - zSurface) * Tan(theta) * Sin(phi)
                If Round(zSurface / textureDepth, 3) >= 0 And Round(zSurface / textureDepth, 3) <= 1 And _
                    Round(ySurface / deviceLength, 3) >= Round(1 - xSurface / deviceWidth, 3) And _
                    Round(ySurface / deviceLength, 3) <= Round(1 + xSurface / deviceWidth, 3) And _
                    Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
            Case Is = 3
                'Facet z = (y - devicelength) tan(alpha) {front-facing}
                ySurface = (y0 - (z0 + textureDepth) * Tan(theta) * Sin(phi)) / (1 - Tan(textureAngle) * Tan(theta) * Sin(phi))
                zSurface = (ySurface - deviceLength) * Tan(textureAngle)
                xSurface = x0 - (z0 - zSurface) * Tan(theta) * Cos(phi)
                If Round(zSurface / textureDepth, 3) >= 0 And Round(zSurface / textureDepth, 3) <= 1 And _
                    Round(xSurface / deviceWidth, 3) >= Round(1 - ySurface / deviceLength, 3) And _
                    Round(xSurface / deviceWidth, 3) <= Round(-1 + ySurface / deviceLength, 3) And _
                    Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
        End Select
    Case inverted
        Select Case Round(phi / (pi / 2))
            Case Is = 0
                'Facet z = -x tan(alpha)  {left-facing}
                xSurface = (x0 - z0 * Tan(theta) * Cos(phi)) / (1 + Tan(textureAngle) * Tan(theta) * Cos(phi))
                zSurface = -xSurface * Tan(textureAngle)
                ySurface = y0 - (z0 - zSurface) * Tan(theta) * Sin(phi)
                If Round(zSurface / textureDepth, 3) >= 0 And Round(zSurface / textureDepth) <= 1 And _
                    Round(ySurface / deviceLength, 3) > Round(-xSurface / deviceWidth, 3) And _
                    Round(ySurface / deviceLength, 3) < Round(2 + xSurface / deviceWidth, 3) And _
                    Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
            Case Is = 1
                'Facet z = - y tan(alpha)   {rear-facing}
                ySurface = (y0 - z0 * Tan(theta) * Sin(phi)) / (1 + Tan(textureAngle) * Tan(theta) * Sin(phi))
                zSurface = -ySurface * Tan(textureAngle)
                xSurface = x0 - (z0 - zSurface) * Tan(theta) * Cos(phi)
                If Round(zSurface / textureDepth, 3) >= 0 And Round(zSurface / textureDepth) <= 1 And _
                    (Round(xSurface / deviceWidth, 3) > Round(-ySurface / deviceLength, 3) Or _
                     Round(xSurface / deviceWidth, 3) < Round(ySurface / deviceLength, 3)) And _
                    Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
            Case Is = 2
                'Facet z =  x tan(alpha)  {right-facing}
                xSurface = (x0 - z0 * Tan(theta) * Cos(phi)) / (1 - Tan(textureAngle) * Tan(theta) * Cos(phi))
                zSurface = xSurface * Tan(textureAngle)
                ySurface = y0 - (z0 - zSurface) * Tan(theta) * Sin(phi)
                If Round(zSurface / textureDepth, 3) >= 0 And Round(zSurface / textureDepth, 3) <= 1 And _
                    Round(ySurface / deviceLength, 3) > Round(xSurface / deviceWidth, 3) And _
                    Round(ySurface / deviceLength, 3) < Round(2 - xSurface / deviceWidth, 3) And _
                    Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
            Case Is = 3
                'Facet z = y  tan(alpha)   {front-facing}
                ySurface = (y0 - z0 * Tan(theta) * Sin(phi)) / (1 - Tan(textureAngle) * Tan(theta) * Sin(phi))
                zSurface = ySurface * Tan(textureAngle)
                xSurface = x0 - (z0 - zSurface) * Tan(theta) * Cos(phi)
                If Round(zSurface / textureDepth, 3) >= 0 And Round(zSurface / textureDepth) <= 1 And _
                    (Round(xSurface / deviceWidth, 3) > Round(ySurface / deviceLength, 3) Or _
                     Round(xSurface / deviceWidth, 3) < Round(-ySurface / deviceLength, 3)) And _
                    Round((z0 - zSurface) / deviceHeight, 3) * Cos(theta) >= 0 Then Exit Function
    End Select
End Select
xSurface = 0: ySurface = 0: zSurface = 0
OriginOnSurface = False
End Function

