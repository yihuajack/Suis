Attribute VB_Name = "Calculations"
Option Explicit

Sub CalculateRecombination()
'This routine sets the recombination at each node in the solution volume and at both surfaces.
'It also loads the recombination and percentages in the bulk and for each surface onto the Recombination sheet.
Dim i As Integer, j As Integer, k As Integer
Dim localGamma As Double, localP As Double, localN As Double
Dim totalDefect As Double
Dim localBulk As Double, localDefect As Double, avgRecomb As Double, avgDefect As Double     'cm-3/s
Dim avgFront As Double, avgRear As Double      'cm-2/s
totalDefect = 0
'Bulk Recombination
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    localGamma = vGamma(i, j, k)
    localBulk = 0: localDefect = 0
    If localGamma > 0 Then
        localP = rEqP + localGamma
        localN = rEqN + localGamma
        localBulk = (localP * localN - rNi ^ 2) / ((localP + rNi) * rTauN + (localN + rNi) * rTauP)
        localBulk = localBulk + rCa * ((localP ^ 2 * localN + (localN ^ 2 * localP)) - (rEqP ^ 2 * rEqN + rEqN ^ 2 * rEqP))
    End If
    If bEnablePointDefect Then localDefect = localDefect + PointDefectRecombination(i, j, k)
    If bEnableLineDefect Then localDefect = localDefect + LineDefectRecombination(i, j, k)
    If bEnablePlaneDefect Then localDefect = localDefect + PlaneDefectRecombination(i, j, k)
    vRecombination(i, j, k) = localBulk + localDefect
    If (i = 1 Or i = nNodesX) Then localDefect = 0.5 * localDefect
    If (j = 1 Or j = nNodesY) Then localDefect = 0.5 * localDefect
    If (k = 1 Or k = nNodesZ) Then localDefect = 0.5 * localDefect
    totalDefect = totalDefect + localDefect
Next k: Next j: Next i
avgRecomb = VolumeAverage(nNodesX, nNodesY, nNodesZ, vRecombination)
avgDefect = totalDefect / ((nNodesX - 1) * (nNodesY - 1) * (nNodesZ - 1))
Range(recombBulkCell).Value = q * (avgRecomb - avgDefect) * deviceHeight
Range(recombDefectCell).Value = q * avgDefect * deviceHeight
'Surface Recombination
Call CalculateSurfaceRecombination  'Supplies values for sFrontRecomb and sRearRecomb
avgFront = SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontRecomb)
avgRear = SurfaceAverage(1, nNodesX, 1, nNodesY, sRearRecomb)
'Total Recombination
rTotalRecombination = avgFront + avgRear + avgRecomb * deviceHeight
Range(recombinationCell).Value = q * rTotalRecombination
'Compute and display percentage recombination of each type
If rTotalRecombination > 0 Then
    Range(recombBulkPercentCell).Value = Range(recombBulkCell) / (q * rTotalRecombination)
    Range(recombFrontJo1PercentCell).Value = Range(recombFrontJo1Cell) / (q * rTotalRecombination)
    Range(recombRearJo1PercentCell).Value = Range(recombRearJo1Cell) / (q * rTotalRecombination)
    Range(recombDefectPercentCell).Value = Range(recombDefectCell) / (q * rTotalRecombination)
    Range(recombFrontJo2PercentCell).Value = Range(recombFrontJo2Cell) / (q * rTotalRecombination)
    Range(recombRearJo2PercentCell).Value = Range(recombRearJo2Cell) / (q * rTotalRecombination)
End If
bRecombination = True
End Sub

Sub CalculateSurfaceRecombination()
Dim i As Integer, j As Integer
Dim avgFront As Double, avgRear As Double
Dim sFrontRecombJo1 As Variant, sRearRecombJo1 As Variant
Dim sFrontRecombJo2 As Variant, sRearRecombJo2 As Variant
ReDim sFrontRecombJo1(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearRecombJo1(1 To nNodesX, 1 To nNodesY) As Double
ReDim sFrontRecombJo2(1 To nNodesX, 1 To nNodesY) As Double
ReDim sRearRecombJo2(1 To nNodesX, 1 To nNodesY) As Double
For j = 1 To nNodesY
    For i = 1 To nNodesX
        sFrontRecombJo1(i, j) = SurfaceRecombinationJo1(front, i, j, sFrontGamma(i, j))
        sRearRecombJo1(i, j) = SurfaceRecombinationJo1(rear, i, j, sRearGamma(i, j))
        sFrontRecombJo2(i, j) = SurfaceRecombinationJo2(front, i, j, sFrontGamma(i, j))
        sRearRecombJo2(i, j) = SurfaceRecombinationJo2(rear, i, j, sRearGamma(i, j))
        sFrontRecomb(i, j) = sFrontRecombJo1(i, j) + sFrontRecombJo2(i, j)
        sRearRecomb(i, j) = sRearRecombJo1(i, j) + sRearRecombJo2(i, j)
    Next i
Next j
avgFront = SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontRecombJo1)
Range(recombFrontJo1Cell).Value = q * avgFront
avgFront = SurfaceAverage(1, nNodesX, 1, nNodesY, sFrontRecombJo2)
Range(recombFrontJo2Cell).Value = q * avgFront
avgRear = SurfaceAverage(1, nNodesX, 1, nNodesY, sRearRecombJo1)
Range(recombRearJo1Cell).Value = q * avgRear
avgRear = SurfaceAverage(1, nNodesX, 1, nNodesY, sRearRecombJo2)
Range(recombRearJo2Cell).Value = q * avgRear
End Sub

Sub CalculateA()
'This routine takes surface Gamma and Psi to obtain Fourier coefficients
'of Gamma and Psi at the the front surface (A).
'Note that the sums of Fourier C terms (bulk effects) must have been calculated previously.
'Both 3D and 2D versions fill (1 to nHarmonicsX+1, 1 to nHarmonicsY+1) for fGammaA and fPsiA
If b3D Then Call CalculateA3D Else Call CalculateA2D
End Sub

Sub CalculateB()
'This routine takes surface Gamma and Psi to obtain Fourier coefficients
'of Gamma and Psi at the rear surface (B).
'Note that the sums of Fourier C terms must have been calculated previously
'Both 3D and 2D versions fill (1 to nHarmonicsX+1, 1 to nHarmonicsY+1) for fGammaB and fPsiB
If b3D Then Call CalculateB3D Else Call CalculateB2D
End Sub

Sub CalculateC()
'This routine takes values of vGeneration and VRecombination to calculate Fourier components
'of the C's for Gamma, Psi and the summed C terms for Gamma and Psi at front and rear surfaces (used for A & B)
'If 2D, assumes all values are uniform in the y axis to reduce computation time.
'Both 3D and 2D versions fill (1 to nHarmonicsX+1, 1 to nHarmonicsY+1, 1 to nHarmonicsZ+1) for fGammaC and fPsiC
If b3D Then Call CalculateC3D Else Call CalculateC2D
End Sub

Sub CalculateAB()
Call CalculateA
Call CalculateB
End Sub

Sub CalculateRealGamma()
'Calculate the real-space ABC components of fGamma, and use them to calculate vGamma.
'Both 3D and 2D versions fill (1 to nNodesX, 1 to nNodesY, 1 to nNodesZ) for vGamma.
If b3D Then Call CalculateRealGamma3D Else Call CalculateRealGamma2D
End Sub

Sub CalculateRealPsi()
'Calculate the real-space ABC components of fPsi, and use them to calculate vPsi.
'Both 3D and 2D versions fill (1 to nNodesX, 1 to nNodesY, 1 to nNodesZ) for vPsi.
If b3D Then Call CalculateRealPsi3D Else Call CalculateRealPsi2D
End Sub

Sub CalculateGradGammaPsi()
'This routine uses the ABC fourier coefficients to find the
'x-, y-, and z-components of the gradient of Gamma and Psi
'Both 3D and 2D versions populate (1 to nNodesX, 1 to nNodesY, 1 to nNodesZ) of x, y, and z gradients of gamma, psi.
'If 2D, assumes uniformity in the y-axis to reduce computation time.
If b3D Then Call CalculateGradGammaPsi3D Else Call CalculateGradGammaPsi2D
End Sub

Sub CalculateNodalFluxErrors(ByVal nNodesX As Integer, ByVal nNodesY As Integer, ByRef sErrorF As Variant)
'Returns flux imbalance error for the surface region surrounding each surface node (cm-2/s)
'for each surface (front to rear), for each carrier (holes to electrons), and for each node i,j.
'When bOpenCircuit, the contact is ignored for the dissimilar-type (i.e. "emitter") contacts.
'Requires Gamma, Psi, and gradients of both at both surfaces.
'The imbalance is the net FLUX flowing INTO of each surface region minus recombination at that surface region.
'Requires input of Gamma, Psi, and Recomb at both surfaces and ABCs for both Gamma and Psi (to calculate Gradients).
'This is a 3D routine. It will work correctly in 2D only if all surface values are uniform in the y-axis!
Dim i As Integer, j As Integer
Dim rPhiP As Double, rPhiN As Double
If pType Then
    rPhiP = 0: rPhiN = -rVoltage
Else
    rPhiP = rVoltage: rPhiN = 0
End If
For i = 1 To nNodesX: For j = 1 To nNodesY
    sErrorF(front, holes, i, j) = SurfaceFlux(front, holes, i, j) - sFrontRecomb(i, j) + LateralNodeFlowIn(front, holes, i, j)
    If Not (nType And bOpenCircuit) Then sErrorF(front, holes, i, j) = _
        sErrorF(front, holes, i, j) + sContact(front, holes, i, j) * (rPhiP - sPhi(front, holes, i, j)) / q
    sErrorF(front, electrons, i, j) = SurfaceFlux(front, electrons, i, j) - sFrontRecomb(i, j) + LateralNodeFlowIn(front, electrons, i, j)
    If Not (pType And bOpenCircuit) Then sErrorF(front, electrons, i, j) = _
        sErrorF(front, electrons, i, j) - sContact(front, electrons, i, j) * (rPhiN - sPhi(front, electrons, i, j)) / q
    sErrorF(rear, holes, i, j) = SurfaceFlux(rear, holes, i, j) - sRearRecomb(i, j) + LateralNodeFlowIn(rear, holes, i, j)
    If Not (nType And bOpenCircuit) Then sErrorF(rear, holes, i, j) = _
        sErrorF(rear, holes, i, j) + sContact(rear, holes, i, j) * (rPhiP - sPhi(rear, holes, i, j)) / q
    sErrorF(rear, electrons, i, j) = SurfaceFlux(rear, electrons, i, j) - sRearRecomb(i, j) + LateralNodeFlowIn(rear, electrons, i, j)
    If Not (pType And bOpenCircuit) Then sErrorF(rear, electrons, i, j) = _
        sErrorF(rear, electrons, i, j) - sContact(rear, electrons, i, j) * (rPhiN - sPhi(rear, electrons, i, j)) / q
Next j: Next i
End Sub

Sub CalculateNodalConductances()
'Returns sQuadSigma as front to rear, holes to electrons, 1 to nNodesX, 1 to nNodesY, TR to BR (counter-clockwise).
'Each quadrant around the node is considered separately to have an "average" conductance.
'The average conductance is the inverse of the average sheet rho, so if any of the four corners are undoped,
'or doped with the opposite polarity, that quadrant is assigned no lateral conductance for the designated carrier.
'If bOpenCircuit, the sheet conductance at each contacted node is increased to represent lateral flow in metal contacts.
'This routine is called from SolveSurfaces prior to each solution.
Dim i As Integer, j As Integer, iSurface As Integer
Dim iLeft As Integer, iRight As Integer, jTop As Integer, jBot As Integer
Dim dx As Double, dy As Double, effectiveArea As Double
Dim saveSigma As Variant
ReDim saveSigma(front To rear, 1 To nNodesX, 1 To nNodesY) As Double
dx = deviceWidth / (nNodesX - 1)
dy = deviceLength / (nNodesY - 1)
saveSigma = sSigma
If bOpenCircuit Then        'Increase sheet conductance at contacted nodes by factor of 1000
    If b3D Then effectiveArea = deviceWidth * deviceLength Else effectiveArea = deviceWidth ^ 2
    For iSurface = front To rear: For i = 1 To nNodesX: For j = 1 To nNodesY
        If sSigma(iSurface, i, j) > 0 And sContact(iSurface, holes, i, j) > 0 Then _
            sSigma(iSurface, i, j) = sSigma(iSurface, i, j) * (1 + sContact(iSurface, holes, i, j) * effectiveArea)
        If sSigma(iSurface, i, j) < 0 And sContact(iSurface, electrons, i, j) > 0 Then _
            sSigma(iSurface, i, j) = sSigma(iSurface, i, j) * (1 + sContact(iSurface, electrons, i, j) * effectiveArea)
    Next j: Next i: Next iSurface
End If
For iSurface = front To rear
    For i = 1 To nNodesX: For j = 1 To nNodesY
        If i = 1 Then iLeft = 2 Else iLeft = i - 1
        If i = nNodesX Then iRight = nNodesX - 1 Else iRight = i + 1
        If j = 1 Then jTop = 2 Else jTop = j - 1
        If j = nNodesY Then jBot = nNodesY - 1 Else jBot = j + 1
        'Holes
        If sSigma(iSurface, i, j) <= 0 Or sSigma(iSurface, iRight, j) <= 0 _
         Or sSigma(iSurface, iRight, jTop) <= 0 Or sSigma(iSurface, i, jTop) <= 0 Then
            sQuadSigma(iSurface, holes, i, j, TR) = 0
        Else
            sQuadSigma(iSurface, holes, i, j, TR) = _
                (sSigma(iSurface, i, j) + sSigma(iSurface, iRight, j) + sSigma(iSurface, iRight, jTop) + sSigma(iSurface, i, jTop)) / 4
        End If
        If sSigma(iSurface, i, j) <= 0 Or sSigma(iSurface, i, jTop) <= 0 _
         Or sSigma(iSurface, iLeft, jTop) <= 0 Or sSigma(iSurface, iLeft, j) <= 0 Then
            sQuadSigma(iSurface, holes, i, j, TL) = 0
        Else
            sQuadSigma(iSurface, holes, i, j, TL) = _
                (sSigma(iSurface, i, j) + sSigma(iSurface, i, jTop) + sSigma(iSurface, iLeft, jTop) + sSigma(iSurface, iLeft, j)) / 4
        End If
        If sSigma(iSurface, i, j) <= 0 Or sSigma(iSurface, iLeft, j) <= 0 _
         Or sSigma(iSurface, iLeft, jBot) <= 0 Or sSigma(iSurface, i, jBot) <= 0 Then
            sQuadSigma(iSurface, holes, i, j, BL) = 0
        Else
            sQuadSigma(iSurface, holes, i, j, BL) = _
                (sSigma(iSurface, i, j) + sSigma(iSurface, iLeft, j) + sSigma(iSurface, iLeft, jBot) + sSigma(iSurface, i, jBot)) / 4
        End If
        If sSigma(iSurface, i, j) <= 0 Or sSigma(iSurface, i, jBot) <= 0 _
         Or sSigma(iSurface, iRight, jBot) <= 0 Or sSigma(iSurface, iRight, j) <= 0 Then
            sQuadSigma(iSurface, holes, i, j, BR) = 0
        Else
            sQuadSigma(iSurface, holes, i, j, BR) = _
                (sSigma(iSurface, i, j) + sSigma(iSurface, i, jBot) + sSigma(iSurface, iRight, jBot) + sSigma(iSurface, iRight, j)) / 4
        End If
        'Electrons
        If sSigma(iSurface, i, j) >= 0 Or sSigma(iSurface, iRight, j) >= 0 _
         Or sSigma(iSurface, iRight, jTop) >= 0 Or sSigma(iSurface, i, jTop) >= 0 Then
            sQuadSigma(iSurface, electrons, i, j, TR) = 0
        Else
            sQuadSigma(iSurface, electrons, i, j, TR) = _
                (sSigma(iSurface, i, j) + sSigma(iSurface, iRight, j) + sSigma(iSurface, iRight, jTop) + sSigma(iSurface, i, jTop)) / 4
        End If
        If sSigma(iSurface, i, j) >= 0 Or sSigma(iSurface, i, jTop) >= 0 _
         Or sSigma(iSurface, iLeft, jTop) >= 0 Or sSigma(iSurface, iLeft, j) >= 0 Then
            sQuadSigma(iSurface, electrons, i, j, TL) = 0
        Else
            sQuadSigma(iSurface, electrons, i, j, TL) = _
                (sSigma(iSurface, i, j) + sSigma(iSurface, i, jTop) + sSigma(iSurface, iLeft, jTop) + sSigma(iSurface, iLeft, j)) / 4
        End If
        If sSigma(iSurface, i, j) >= 0 Or sSigma(iSurface, iLeft, j) >= 0 _
         Or sSigma(iSurface, iLeft, jBot) >= 0 Or sSigma(iSurface, i, jBot) >= 0 Then
            sQuadSigma(iSurface, electrons, i, j, BL) = 0
        Else
            sQuadSigma(iSurface, electrons, i, j, BL) = _
                (sSigma(iSurface, i, j) + sSigma(iSurface, iLeft, j) + sSigma(iSurface, iLeft, jBot) + sSigma(iSurface, i, jBot)) / 4
        End If
        If sSigma(iSurface, i, j) >= 0 Or sSigma(iSurface, i, jBot) >= 0 _
         Or sSigma(iSurface, iRight, jBot) >= 0 Or sSigma(iSurface, iRight, j) >= 0 Then
            sQuadSigma(iSurface, electrons, i, j, BR) = 0
        Else
            sQuadSigma(iSurface, electrons, i, j, BR) = _
                (sSigma(iSurface, i, j) + sSigma(iSurface, i, jBot) + sSigma(iSurface, iRight, jBot) + sSigma(iSurface, iRight, j)) / 4
        End If
    Next j: Next i
Next iSurface
sSigma = saveSigma     'Restore sheet conductance, which may have been altered, e.g. if open-circuit mode.
End Sub

Sub CalculateLocalJacobian(ByVal nSurface As Integer, ByVal i As Integer, ByVal j As Integer, ByRef localJ As Variant)
'Requires SurfaceGradGamma/Psi and sDeltaFluxDeltaPhi (both surfaces, both carriers)
'AbsDeltaGradGamma/Psi depends only on device specification and is computed in CalculateTransforms
'LocalJ is holes to electrons, holes to electrons (2 x 2 matrix)
'Terms related to dissimilar-type contacts are ignored if bOpenCircuit
Dim rDeltaFpDeltaGamma As Double, rDeltaFpDeltaPsi As Double
Dim rDeltaFnDeltaGamma As Double, rDeltaFnDeltaPsi As Double
Dim rDeltaRdeltaGamma As Double, dFlux_dPhi As Double, dx As Double, dy As Double, centerfactor As Double
Dim rLocalGamma As Double, rLocalP As Double, rLocalN As Double, rLocalVoltage As Double
dx = deviceWidth / (nNodesX - 1)
dy = deviceLength / (nNodesY - 1)
centerfactor = -1 / (8 * q) * (3 / dx ^ 2 + 3 / dy ^ 2)
If nSurface = front Then
    rLocalGamma = sFrontGamma(i, j)
    rLocalP = rEqP + rLocalGamma
    rLocalN = rEqN + rLocalGamma
    rLocalVoltage = rVt * Log((rLocalP * rLocalN) / rNi ^ 2)
    rDeltaFpDeltaGamma = rDp * (-sAbsDeltaGradGammaDeltaGamma(i, j) + sFrontGradPsi(i, j) / rVt)
    rDeltaFnDeltaGamma = rDn * (-sAbsDeltaGradGammaDeltaGamma(i, j) - sFrontGradPsi(i, j) / rVt)
    rDeltaFpDeltaPsi = rDp * rLocalP * -sAbsDeltaGradPsiDeltaPsi(i, j) / rVt
    rDeltaFnDeltaPsi = rDn * rLocalN * sAbsDeltaGradPsiDeltaPsi(i, j) / rVt
    rDeltaRdeltaGamma = (sJo1(front, i, j) / (q * rVt) * Exp(rLocalVoltage / rVt) _
                       + sJo2(front, i, j) / (2 * q * rVt) * Exp(rLocalVoltage / (2 * rVt)) _
                       + sGsh(front, i, j) / q) _
                       * rVt * (rLocalP + rLocalN) / (rLocalP * rLocalN)       'Note DeltaRdeltaPsi = 0
Else
    rLocalGamma = sRearGamma(i, j)
    rLocalP = rEqP + rLocalGamma
    rLocalN = rEqN + rLocalGamma
    rLocalVoltage = rVt * Log((rLocalP * rLocalN) / rNi ^ 2)
    rDeltaFpDeltaGamma = rDp * (-sAbsDeltaGradGammaDeltaGamma(i, j) - sRearGradPsi(i, j) / rVt)
    rDeltaFnDeltaGamma = rDn * (-sAbsDeltaGradGammaDeltaGamma(i, j) + sRearGradPsi(i, j) / rVt)
    rDeltaFpDeltaPsi = rDp * rLocalP * -sAbsDeltaGradPsiDeltaPsi(i, j) / rVt
    rDeltaFnDeltaPsi = rDn * rLocalN * sAbsDeltaGradPsiDeltaPsi(i, j) / rVt
    rDeltaRdeltaGamma = (sJo1(rear, i, j) / (q * rVt) * Exp(rLocalVoltage / rVt) _
                      + sJo2(rear, i, j) / (2 * q * rVt) * Exp(rLocalVoltage / (2 * rVt)) _
                      + sGsh(rear, i, j) / q) _
                      * rVt * (rLocalP + rLocalN) / (rLocalP * rLocalN)       'Note DeltaRdeltaPsi = 0
End If
dFlux_dPhi = centerfactor * (sQuadSigma(nSurface, holes, i, j, TR) + sQuadSigma(nSurface, holes, i, j, TL) _
                        + sQuadSigma(nSurface, holes, i, j, BL) + sQuadSigma(nSurface, holes, i, j, BR))
localJ(holes, gamma) = rDeltaFpDeltaGamma - rDeltaRdeltaGamma + dFlux_dPhi * rVt / rLocalP
If Not (nType And bOpenCircuit) Then localJ(holes, gamma) = localJ(holes, gamma) - sContact(nSurface, holes, i, j) / q * rVt / rLocalP
localJ(holes, psi) = rDeltaFpDeltaPsi + dFlux_dPhi
If Not (nType And bOpenCircuit) Then localJ(holes, psi) = localJ(holes, psi) - sContact(nSurface, holes, i, j) / q
dFlux_dPhi = centerfactor * (sQuadSigma(nSurface, electrons, i, j, TR) + sQuadSigma(nSurface, electrons, i, j, TL) _
                        + sQuadSigma(nSurface, electrons, i, j, BL) + sQuadSigma(nSurface, electrons, i, j, BR))            'Sigma is negative, providing the necessary sign inversion for electrons
localJ(electrons, gamma) = rDeltaFnDeltaGamma - rDeltaRdeltaGamma - dFlux_dPhi * rVt / rLocalN
If Not (pType And bOpenCircuit) Then localJ(electrons, gamma) = localJ(electrons, gamma) - sContact(nSurface, electrons, i, j) / q * rVt / rLocalN
localJ(electrons, psi) = rDeltaFnDeltaPsi + dFlux_dPhi
If Not (pType And bOpenCircuit) Then localJ(electrons, psi) = localJ(electrons, psi) + sContact(nSurface, electrons, i, j) / q
End Sub

Sub CalculateSurfaceJacobian(ByVal nSurface As Integer, ByRef m As Variant)
'Requires SurfaceGradGammaPsi
'AbsDeltaGradGamma/Psi depends only on device specification and is computed in CalculateTransforms
'm is a 2D matrix that is 2*nNodesX*nNodesY square. Terms are row-major order, with carrier as the last index
'Terms related to dissimilar-type contacts are ignored if bOpenCircuit
'The only surface-normal flux effect considered is for delta gamma and delta psi at that node
'Lateral flux terms depend on the local node plus all 8 surrounding nodes on that surface
Dim rDeltaFpDeltaGamma As Double, rDeltaFpDeltaPsi As Double
Dim rDeltaFnDeltaGamma As Double, rDeltaFnDeltaPsi As Double
Dim rDeltaRdeltaGamma As Double, dx As Double, dy As Double, dFlux_dPhi As Double
Dim centerfactor As Double, xEdgeFactor As Double, yEdgeFactor As Double, cornerFactor As Double
Dim rLocalP As Double, rLocalN As Double, rLocalVoltage As Double
Dim rLeftP As Double, rRightP As Double, rTopP As Double, rBotP As Double, rTopRightP As Double, rTopLeftP As Double, rBotLeftP As Double, rBotRightP As Double
Dim rLeftN As Double, rRightN As Double, rTopN As Double, rBotN As Double, rTopRightN As Double, rTopLeftN As Double, rBotLeftN As Double, rBotRightN As Double
Dim iLeft As Integer, iRight As Integer, jTop As Integer, jBot As Integer
Dim i As Integer, j As Integer, diag As Integer
Dim gammaL As Integer, gammaR As Integer, gammaT As Integer, gammaTL As Integer, gammaTR As Integer, gammaB As Integer, gammaBL As Integer, gammaBR As Integer
Dim psiL As Integer, psiR As Integer, psiT As Integer, psiTL As Integer, psiTR As Integer, psiB As Integer, psiBL As Integer, psiBR As Integer
'Offsets from diagonal for node (i,j)
dx = deviceWidth / (nNodesX - 1)
dy = deviceLength / (nNodesY - 1)
centerfactor = -1 / (8 * q) * (3 / dx ^ 2 + 3 / dy ^ 2)
xEdgeFactor = 1 / (8 * q) * (3 / dx ^ 2 - 1 / dy ^ 2)
yEdgeFactor = 1 / (8 * q) * (3 / dy ^ 2 - 1 / dx ^ 2)
cornerFactor = 1 / (8 * q) * (1 / dx ^ 2 + 1 / dy ^ 2)
For i = 1 To nNodesX: For j = 1 To nNodesY
    'Reflect values that are beyond the edges of the soluton volume back into the volume
    If i = 1 Then iLeft = i + 1 Else iLeft = i - 1
    If i = nNodesX Then iRight = i - 1 Else iRight = i + 1
    If j = 1 Then jTop = j + 1 Else jTop = j - 1
    If j = nNodesY Then jBot = j - 1 Else jBot = j + 1
    diag = 2 * ((i - 1) * nNodesY + (j - 1))     'Index for diagonal at upper-left corner for flux balance at node (i, j)
    gammaL = gamma + 2 * ((iLeft - i) * nNodesY)    'Offset columns from diagonal (note that gamma = 1 and psi = 2)
    gammaR = gamma + 2 * ((iRight - i) * nNodesY)
    gammaT = gamma + 2 * (jTop - j)
    gammaB = gamma + 2 * (jBot - j)
    gammaTR = gamma + 2 * ((iRight - i) * nNodesY + (jTop - j))
    gammaTL = gamma + 2 * ((iLeft - i) * nNodesY + (jTop - j))
    gammaBL = gamma + 2 * ((iLeft - i) * nNodesY + (jBot - j))
    gammaBR = gamma + 2 * ((iRight - i) * nNodesY + (jBot - j))
    psiL = psi + 2 * ((iLeft - i) * nNodesY)
    psiR = psi + 2 * ((iRight - i) * nNodesY)
    psiT = psi + 2 * (jTop - j)
    psiB = psi + 2 * (jBot - j)
    psiTR = psi + 2 * ((iRight - i) * nNodesY + (jTop - j))
    psiTL = psi + 2 * ((iLeft - i) * nNodesY + (jTop - j))
    psiBL = psi + 2 * ((iLeft - i) * nNodesY + (jBot - j))
    psiBR = psi + 2 * ((iRight - i) * nNodesY + (jBot - j))
    If nSurface = front Then
        rLocalP = rEqP + sFrontGamma(i, j)
        rRightP = rEqP + sFrontGamma(iRight, j)
        rLeftP = rEqP + sFrontGamma(iLeft, j)
        rTopP = rEqP + sFrontGamma(i, jTop)
        rBotP = rEqP + sFrontGamma(i, jBot)
        rTopRightP = rEqP + sFrontGamma(iRight, jTop)
        rTopLeftP = rEqP + sFrontGamma(iLeft, jTop)
        rBotLeftP = rEqP + sFrontGamma(iLeft, jBot)
        rBotRightP = rEqP + sFrontGamma(iRight, jBot)
        rLocalN = rEqN + sFrontGamma(i, j)
        rRightN = rEqN + sFrontGamma(iRight, j)
        rLeftN = rEqN + sFrontGamma(iLeft, j)
        rTopN = rEqN + sFrontGamma(i, jTop)
        rBotN = rEqN + sFrontGamma(i, jBot)
        rTopRightN = rEqN + sFrontGamma(iRight, jTop)
        rTopLeftN = rEqN + sFrontGamma(iLeft, jTop)
        rBotLeftN = rEqN + sFrontGamma(iLeft, jBot)
        rBotRightN = rEqN + sFrontGamma(iRight, jBot)
        rLocalVoltage = rVt * Log((rLocalP * rLocalN) / rNi ^ 2)
        rDeltaFpDeltaGamma = rDp * (-sAbsDeltaGradGammaDeltaGamma(i, j) + sFrontGradPsi(i, j) / rVt)
        rDeltaFnDeltaGamma = rDn * (-sAbsDeltaGradGammaDeltaGamma(i, j) - sFrontGradPsi(i, j) / rVt)
        rDeltaFpDeltaPsi = rDp * rLocalP * -sAbsDeltaGradPsiDeltaPsi(i, j) / rVt
        rDeltaFnDeltaPsi = rDn * rLocalN * sAbsDeltaGradPsiDeltaPsi(i, j) / rVt
        rDeltaRdeltaGamma = (sJo1(front, i, j) / (q * rVt) * Exp(rLocalVoltage / rVt) _
                           + sJo2(front, i, j) / (2 * q * rVt) * Exp(rLocalVoltage / (2 * rVt)) _
                           + sGsh(front, i, j) / q) _
                           * rVt * (rLocalP + rLocalN) / (rLocalP * rLocalN)       'Note DeltaRdeltaPsi = 0
    Else
        rLocalP = rEqP + sRearGamma(i, j)
        rRightP = rEqP + sRearGamma(iRight, j)
        rLeftP = rEqP + sRearGamma(iLeft, j)
        rTopP = rEqP + sRearGamma(i, jTop)
        rBotP = rEqP + sRearGamma(i, jBot)
        rTopRightP = rEqP + sRearGamma(iRight, jTop)
        rTopLeftP = rEqP + sRearGamma(iLeft, jTop)
        rBotLeftP = rEqP + sRearGamma(iLeft, jBot)
        rBotRightP = rEqP + sRearGamma(iRight, jBot)
        rLocalN = rEqN + sRearGamma(i, j)
        rRightN = rEqN + sRearGamma(iRight, j)
        rLeftN = rEqN + sRearGamma(iLeft, j)
        rTopN = rEqN + sRearGamma(i, jTop)
        rBotN = rEqN + sRearGamma(i, jBot)
        rTopRightN = rEqN + sRearGamma(iRight, jTop)
        rTopLeftN = rEqN + sRearGamma(iLeft, jTop)
        rBotLeftN = rEqN + sRearGamma(iLeft, jBot)
        rBotRightN = rEqN + sRearGamma(iRight, jBot)
        rLocalVoltage = rVt * Log((rLocalP * rLocalN) / rNi ^ 2)
        rDeltaFpDeltaGamma = rDp * (-sAbsDeltaGradGammaDeltaGamma(i, j) - sRearGradPsi(i, j) / rVt)
        rDeltaFnDeltaGamma = rDn * (-sAbsDeltaGradGammaDeltaGamma(i, j) + sRearGradPsi(i, j) / rVt)
        rDeltaFpDeltaPsi = rDp * rLocalP * -sAbsDeltaGradPsiDeltaPsi(i, j) / rVt
        rDeltaFnDeltaPsi = rDn * rLocalN * sAbsDeltaGradPsiDeltaPsi(i, j) / rVt
        rDeltaRdeltaGamma = (sJo1(rear, i, j) / (q * rVt) * Exp(rLocalVoltage / rVt) _
                          + sJo2(rear, i, j) / (2 * q * rVt) * Exp(rLocalVoltage / (2 * rVt)) _
                          + sGsh(rear, i, j) / q) _
                          * rVt * (rLocalP + rLocalN) / (rLocalP * rLocalN)       'Note DeltaRdeltaPsi = 0
    End If
    'Hole Flux In
    dFlux_dPhi = centerfactor * (sQuadSigma(nSurface, holes, i, j, TR) + sQuadSigma(nSurface, holes, i, j, TL) _
                        + sQuadSigma(nSurface, holes, i, j, BL) + sQuadSigma(nSurface, holes, i, j, BR))
        m(diag + holes, diag + gamma) = rDeltaFpDeltaGamma - rDeltaRdeltaGamma + dFlux_dPhi * rVt / rLocalP
        If Not (nType And bOpenCircuit) Then m(diag + holes, diag + gamma) = m(diag + holes, diag + gamma) - sContact(nSurface, holes, i, j) / q * rVt / rLocalP
        m(diag + holes, diag + psi) = rDeltaFpDeltaPsi + dFlux_dPhi
        If Not (nType And bOpenCircuit) Then m(diag + holes, diag + psi) = m(diag + holes, diag + psi) - sContact(nSurface, holes, i, j) / q
    dFlux_dPhi = xEdgeFactor * (sQuadSigma(nSurface, holes, i, j, TL) + sQuadSigma(nSurface, holes, i, j, BL))
        m(diag + holes, diag + gammaL) = m(diag + holes, diag + gammaL) + dFlux_dPhi * rVt / rLeftP
        m(diag + holes, diag + psiL) = m(diag + holes, diag + psiL) + dFlux_dPhi
    dFlux_dPhi = xEdgeFactor * (sQuadSigma(nSurface, holes, i, j, TR) + sQuadSigma(nSurface, holes, i, j, BR))
        m(diag + holes, diag + gammaR) = m(diag + holes, diag + gammaR) + dFlux_dPhi * rVt / rRightP
        m(diag + holes, diag + psiR) = m(diag + holes, diag + psiR) + dFlux_dPhi
    dFlux_dPhi = yEdgeFactor * (sQuadSigma(nSurface, holes, i, j, TL) + sQuadSigma(nSurface, holes, i, j, TR))
        m(diag + holes, diag + gammaT) = m(diag + holes, diag + gammaT) + dFlux_dPhi * rVt / rTopP
        m(diag + holes, diag + psiT) = m(diag + holes, diag + psiT) + dFlux_dPhi
    dFlux_dPhi = yEdgeFactor * (sQuadSigma(nSurface, holes, i, j, BL) + sQuadSigma(nSurface, holes, i, j, BR))
        m(diag + holes, diag + gammaB) = m(diag + holes, diag + gammaB) + dFlux_dPhi * rVt / rBotP
        m(diag + holes, diag + psiB) = m(diag + holes, diag + psiB) + dFlux_dPhi
    dFlux_dPhi = cornerFactor * sQuadSigma(nSurface, holes, i, j, TR)
        m(diag + holes, diag + gammaTR) = m(diag + holes, diag + gammaTR) + dFlux_dPhi * rVt / rTopRightP
        m(diag + holes, diag + psiTR) = m(diag + holes, diag + psiTR) + dFlux_dPhi
    dFlux_dPhi = cornerFactor * sQuadSigma(nSurface, holes, i, j, TL)
        m(diag + holes, diag + gammaTL) = m(diag + holes, diag + gammaTL) + dFlux_dPhi * rVt / rTopLeftP
        m(diag + holes, diag + psiTL) = m(diag + holes, diag + psiTL) + dFlux_dPhi
    dFlux_dPhi = cornerFactor * sQuadSigma(nSurface, holes, i, j, BL)
        m(diag + holes, diag + gammaBL) = m(diag + holes, diag + gammaBL) + dFlux_dPhi * rVt / rBotLeftP
        m(diag + holes, diag + psiBL) = m(diag + holes, diag + psiBL) + dFlux_dPhi
    dFlux_dPhi = cornerFactor * sQuadSigma(nSurface, holes, i, j, BR)
        m(diag + holes, diag + gammaBR) = m(diag + holes, diag + gammaBR) + dFlux_dPhi * rVt / rBotRightP
        m(diag + holes, diag + psiBR) = m(diag + holes, diag + psiBR) + dFlux_dPhi
    'Electron Flux In
    dFlux_dPhi = centerfactor * (sQuadSigma(nSurface, electrons, i, j, TR) + sQuadSigma(nSurface, electrons, i, j, TL) _
                        + sQuadSigma(nSurface, electrons, i, j, BL) + sQuadSigma(nSurface, electrons, i, j, BR))            'Sigma is negative, providing the necessary sign inversion for electrons
        m(diag + electrons, diag + gamma) = rDeltaFnDeltaGamma - rDeltaRdeltaGamma - dFlux_dPhi * rVt / rLocalN
        If Not (pType And bOpenCircuit) Then m(diag + electrons, diag + gamma) = m(diag + electrons, diag + gamma) - sContact(nSurface, electrons, i, j) / q * rVt / rLocalN
        m(diag + electrons, diag + psi) = rDeltaFnDeltaPsi + dFlux_dPhi
        If Not (pType And bOpenCircuit) Then m(diag + electrons, diag + psi) = m(diag + electrons, diag + psi) + sContact(nSurface, electrons, i, j) / q
    dFlux_dPhi = xEdgeFactor * (sQuadSigma(nSurface, electrons, i, j, TL) + sQuadSigma(nSurface, electrons, i, j, BL))
        m(diag + electrons, diag + gammaL) = m(diag + electrons, diag + gammaL) - dFlux_dPhi * rVt / rLeftN
        m(diag + electrons, diag + psiL) = m(diag + electrons, diag + psiL) + dFlux_dPhi
    dFlux_dPhi = xEdgeFactor * (sQuadSigma(nSurface, electrons, i, j, TR) + sQuadSigma(nSurface, electrons, i, j, BR))
        m(diag + electrons, diag + gammaR) = m(diag + electrons, diag + gammaR) - dFlux_dPhi * rVt / rRightN
        m(diag + electrons, diag + psiR) = m(diag + electrons, diag + psiR) + dFlux_dPhi
    dFlux_dPhi = yEdgeFactor * (sQuadSigma(nSurface, electrons, i, j, TL) + sQuadSigma(nSurface, electrons, i, j, TR))
        m(diag + electrons, diag + gammaT) = m(diag + electrons, diag + gammaT) - dFlux_dPhi * rVt / rTopN
        m(diag + electrons, diag + psiT) = m(diag + electrons, diag + psiT) + dFlux_dPhi
    dFlux_dPhi = yEdgeFactor * (sQuadSigma(nSurface, electrons, i, j, BL) + sQuadSigma(nSurface, electrons, i, j, BR))
        m(diag + electrons, diag + gammaB) = m(diag + electrons, diag + gammaB) - dFlux_dPhi * rVt / rBotN
        m(diag + electrons, diag + psiB) = m(diag + electrons, diag + psiB) + dFlux_dPhi
    dFlux_dPhi = cornerFactor * sQuadSigma(nSurface, electrons, i, j, TR)
        m(diag + electrons, diag + gammaTR) = m(diag + electrons, diag + gammaTR) - dFlux_dPhi * rVt / rTopRightN
        m(diag + electrons, diag + psiTR) = m(diag + electrons, diag + psiTR) + dFlux_dPhi
    dFlux_dPhi = cornerFactor * sQuadSigma(nSurface, electrons, i, j, TL)
        m(diag + electrons, diag + gammaTL) = m(diag + electrons, diag + gammaTL) - dFlux_dPhi * rVt / rTopLeftN
        m(diag + electrons, diag + psiTL) = m(diag + electrons, diag + psiTL) + dFlux_dPhi
    dFlux_dPhi = cornerFactor * sQuadSigma(nSurface, electrons, i, j, BL)
        m(diag + electrons, diag + gammaBL) = m(diag + electrons, diag + gammaBL) - dFlux_dPhi * rVt / rBotLeftN
        m(diag + electrons, diag + psiBL) = m(diag + electrons, diag + psiBL) + dFlux_dPhi
    dFlux_dPhi = cornerFactor * sQuadSigma(nSurface, electrons, i, j, BR)
        m(diag + electrons, diag + gammaBR) = m(diag + electrons, diag + gammaBR) - dFlux_dPhi * rVt / rBotRightN
        m(diag + electrons, diag + psiBR) = m(diag + electrons, diag + psiBR) + dFlux_dPhi
Next j: Next i
End Sub

Sub CalculateTransforms()
Dim i As Integer, j As Integer, m As Integer, n As Integer
Dim kmn As Double
Dim fK As Variant

If pType Then rInvL2 = 1 / (rDn * rTauN) Else rInvL2 = 1 / (rDp * rTauP)
If rInvL2 < 1 Then rInvL2 = 1

'   *** TRANSFORMATION MATRICES USED ON BOTH SURFACES ***
ReDim f2rX(1 To nNodesX, 1 To nHarmonicsX + 1) As Double
For i = 1 To nNodesX: For m = 0 To nHarmonicsX
    f2rX(i, m + 1) = Cos(Pi * m * (i - 1) / (nNodesX - 1))
Next m: Next i
ReDim f2rY(1 To nNodesY, 1 To nHarmonicsY + 1) As Double
For i = 1 To nNodesY: For m = 0 To nHarmonicsY
    f2rY(i, m + 1) = Cos(Pi * m * (i - 1) / (nNodesY - 1))
Next m: Next i
ReDim f2rZ(1 To nNodesZ, 1 To nHarmonicsZ + 1) As Double
For i = 1 To nNodesZ: For m = 0 To nHarmonicsZ
    f2rZ(i, m + 1) = Cos(Pi * m * (i - 1) / (nNodesZ - 1))
Next m: Next i
ReDim r2fX(1 To nHarmonicsX + 1, 1 To nNodesX) As Double
r2fX = Application.MInverse(f2rX)
ReDim r2fY(1 To nHarmonicsY + 1, 1 To nNodesY) As Double
r2fY = Application.MInverse(f2rY)
ReDim r2fZ(1 To nHarmonicsZ + 1, 1 To nNodesZ) As Double
r2fZ = Application.MInverse(f2rZ)
ReDim f2r2X(1 To nNodesX, 1 To nHarmonicsX + 1) As Double
For i = 1 To nNodesX: For m = 0 To nHarmonicsX
    f2r2X(i, m + 1) = f2rX(i, m + 1) * r2fX(i, m + 1)
Next m: Next i
ReDim f2r2Y(1 To nNodesY, 1 To nHarmonicsY + 1) As Double
For i = 1 To nNodesY: For m = 0 To nHarmonicsY
    f2r2Y(i, m + 1) = f2rY(i, m + 1) * r2fY(i, m + 1)
Next m: Next i
If b3D Then
    ReDim fK(1 To nHarmonicsX + 1, 1 To nHarmonicsY + 1) As Double
    For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
        kmn = Sqr((m * Pi / deviceWidth) ^ 2 + (n * Pi / deviceLength) ^ 2 + rInvL2)
        fK(m + 1, n + 1) = kmn * Cosh(kmn * deviceHeight) / Sinh(kmn * deviceHeight)
    Next n: Next m
    sAbsDeltaGradGammaDeltaGamma = Application.Transpose(Application.MMult(f2r2Y, Application.Transpose(Application.MMult(f2r2X, fK))))
    For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
        kmn = Sqr((m * Pi / deviceWidth) ^ 2 + (n * Pi / deviceLength) ^ 2)
        If kmn = 0 Then fK(m + 1, n + 1) = 1 Else fK(m + 1, n + 1) = kmn * Cosh(kmn * deviceHeight) / Sinh(kmn * deviceHeight)
    Next n: Next m
    sAbsDeltaGradPsiDeltaPsi = Application.Transpose(Application.MMult(f2r2Y, Application.Transpose(Application.MMult(f2r2X, fK))))
Else    '2D
    ReDim fK(1 To nHarmonicsX + 1, 1 To nNodesY) As Double
    For m = 0 To nHarmonicsX: For j = 1 To nNodesY
        kmn = Sqr((m * Pi / deviceWidth) ^ 2 + rInvL2)
        fK(m + 1, j) = kmn * Cosh(kmn * deviceHeight) / Sinh(kmn * deviceHeight)
    Next j: Next m
    sAbsDeltaGradGammaDeltaGamma = Application.MMult(f2r2X, fK)
    For m = 0 To nHarmonicsX: For j = 1 To nNodesY
        kmn = m * Pi / deviceWidth
        If kmn = 0 Then fK(m + 1, j) = 1 Else fK(m + 1, j) = kmn * Cosh(kmn * deviceHeight) / Sinh(kmn * deviceHeight)
    Next j: Next m
    sAbsDeltaGradPsiDeltaPsi = Application.MMult(f2r2X, fK)
End If

Call CalculateSinhTerms     'Depends on rInvL2, so must come after its definition above
Call CalculateCoshTerms
Call CalculateSinTerms

'   *** DEFECT FACTOR USED AT BOTH SURFACES AND THROUGHOUT BULK ***
'Uses f2r and r2f values and SinhTerms calculated above
rDeltaGammaDeltaR = DeltaGammaDeltaR(WorksheetFunction.Ceiling((nNodesX - 1) / 2, 1), _
                                     WorksheetFunction.Ceiling((nNodesY - 1) / 2, 1), _
                                     WorksheetFunction.Ceiling((nNodesZ - 1) / 2, 1))  'Use ceiling to allow nNodes=2

End Sub

Sub CalculateSinhTerms()
'Fills 3D global arrays for vSinhTermsA and vSinhTermsB for Gamma and Psi
'Recalculate whenever rInvL2 is changed (due to either tau_min or D_min) or deviceHeight is changed
Dim m As Integer, n As Integer, k As Integer
Dim z As Double, kmn As Double
'First for Gamma (includes rInvL2)
For k = 1 To nNodesZ
    z = deviceHeight * (k - 1) / (nNodesZ - 1)
    For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
        kmn = Sqr((m * Pi / deviceWidth) ^ 2 + (n * Pi / deviceLength) ^ 2 + rInvL2)
        If kmn > 0 Then
            vSinhTermsGammaA(m + 1, n + 1, k) = Sinh(kmn * (deviceHeight - z)) / Sinh(kmn * deviceHeight)
        Else: vSinhTermsGammaA(m + 1, n + 1, k) = (deviceHeight - z) / deviceHeight
        End If
        If kmn > 0 Then
            vSinhTermsGammaB(m + 1, n + 1, k) = Sinh(kmn * z) / Sinh(kmn * deviceHeight)
            Else: vSinhTermsGammaB(m + 1, n + 1, k) = z / deviceHeight
        End If
    Next n: Next m
Next k
'Now for Psi (excludes rInvL2)
For k = 1 To nNodesZ
    z = deviceHeight * (k - 1) / (nNodesZ - 1)
    For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
        kmn = Sqr((m * Pi / deviceWidth) ^ 2 + (n * Pi / deviceLength) ^ 2)
        If kmn > 0 Then
            vSinhTermsPsiA(m + 1, n + 1, k) = Sinh(kmn * (deviceHeight - z)) / Sinh(kmn * deviceHeight)
        Else: vSinhTermsPsiA(m + 1, n + 1, k) = (deviceHeight - z) / deviceHeight
        End If
        If kmn > 0 Then
            vSinhTermsPsiB(m + 1, n + 1, k) = Sinh(kmn * z) / Sinh(kmn * deviceHeight)
            Else: vSinhTermsPsiB(m + 1, n + 1, k) = z / deviceHeight
        End If
    Next n: Next m
Next k
End Sub

Sub CalculateCoshTerms()
'Fills 3D global arrays for vCoshTermsA and vCoshTermsB for Gamma and Psi
'Recalculate whenever rInvL2 is changed (due to either tau_min or D_min) or deviceHeight is changed
Dim m As Integer, n As Integer, k As Integer
Dim z As Double, kmn As Double
'First for Gamma (includes rInvL2)
For k = 1 To nNodesZ
    z = deviceHeight * (k - 1) / (nNodesZ - 1)
    For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
        kmn = Sqr((m * Pi / deviceWidth) ^ 2 + (n * Pi / deviceLength) ^ 2 + rInvL2)
        vCoshTermsGammaA(m + 1, n + 1, k) = kmn * Cosh(kmn * (deviceHeight - z)) / Sinh(kmn * deviceHeight)
        vCoshTermsGammaB(m + 1, n + 1, k) = kmn * Cosh(kmn * z) / Sinh(kmn * deviceHeight)
    Next n: Next m
Next k
'Next for Psi (excludes rInvL2)
For k = 1 To nNodesZ
    z = deviceHeight * (k - 1) / (nNodesZ - 1)
    For m = 0 To nHarmonicsX: For n = 0 To nHarmonicsY
        kmn = Sqr((m * Pi / deviceWidth) ^ 2 + (n * Pi / deviceLength) ^ 2)
        If kmn > 0 Then
            vCoshTermsPsiA(m + 1, n + 1, k) = kmn * Cosh(kmn * (deviceHeight - z)) / Sinh(kmn * deviceHeight)
        Else: vCoshTermsPsiA(m + 1, n + 1, k) = 1 / deviceHeight
        End If
        If kmn > 0 Then
            vCoshTermsPsiB(m + 1, n + 1, k) = kmn * Cosh(kmn * z) / Sinh(kmn * deviceHeight)
            Else: vCoshTermsPsiB(m + 1, n + 1, k) = 1 / deviceHeight
        End If
    Next n: Next m
Next k
End Sub

Sub CalculateSinTerms()
'Fills 2D global sinTerms arrays for each axis
'Recalculate whenever device dimensions are changed
'Includes low-pass filter in all three axes to remove noise associated with gradient operation
Dim i As Integer, j As Integer, k As Integer
Dim m As Integer, n As Integer, P As Integer
Dim x As Double, y As Double, z As Double
For i = 1 To nNodesX
    x = deviceWidth * (i - 1) / (nNodesX - 1)
    sinTermsX(i, 1) = 0
    For m = 1 To nHarmonicsX
        sinTermsX(i, m + 1) = (m * Pi / deviceWidth) * Sin(m * Pi * x / deviceWidth) * Sin(m * Pi / nHarmonicsX) / (m * Pi / nHarmonicsX)
    Next m
Next i
For j = 1 To nNodesY
    y = deviceLength * (j - 1) / (nNodesY - 1)
    sinTermsY(j, 1) = 0
    For n = 1 To nHarmonicsY
        sinTermsY(j, n + 1) = (n * Pi / deviceLength) * Sin(n * Pi * y / deviceLength) * Sin(n * Pi / nHarmonicsY) / (n * Pi / nHarmonicsY)
    Next n
Next j
For k = 1 To nNodesZ
    z = deviceHeight * (k - 1) / (nNodesZ - 1)
    sinTermsZ(k, 1) = 0
    For P = 1 To nHarmonicsZ
        sinTermsZ(k, P + 1) = (P * Pi / deviceHeight) * Sin(P * Pi * z / deviceHeight) * Sin(P * Pi / nHarmonicsZ) / (P * Pi / nHarmonicsZ)
    Next P
Next k
End Sub

Sub CalculateIlluminationUniformity(ByVal nSurface As Integer, ByRef sArray As Variant)
'Fills sArray (sFrontIllumination or sRearIllumination), which represents
'the fraction of the designated illumination intensity incident at each surface node.
'Note that point illumination with z>0 requires modification of photogeneration.
Dim i As Integer, j As Integer
Dim shape As String
Dim x As Double, y As Double, r As Double, x0 As Double, y0 As Double, r0 As Double
Dim a As Double, localBackground As Double, rBackground As Double
Dim rPointX As Double, rPointY As Double, rPointZ As Double, rPointDiameter As Double
Dim rLinePeak As Double, rLineWidth As Double
Dim lineDirection As String, lineShape As String
ReDim sArray(1 To nNodesX, 1 To nNodesY) As Double
If Not bSpectrum And Not bMono Then Exit Sub
If nSurface = front Then
    shape = frontShape
    rPointX = rFrontPointX: rPointY = rFrontPointY: rPointZ = rFrontPointZ: rPointDiameter = rFrontPointDiameter
    rLinePeak = rFrontLinePeak: rLineWidth = rFrontLineWidth
    lineDirection = frontLineDirection: lineShape = frontLineShape
Else
    shape = rearShape
    rPointX = rRearPointX: rPointY = rRearPointY: rPointZ = rRearPointZ: rPointDiameter = rRearPointDiameter
    rLinePeak = rRearLinePeak: rLineWidth = rRearLineWidth
    lineDirection = rearLineDirection: lineShape = rearLineShape
End If
For i = 1 To nNodesX: For j = 1 To nNodesY
    x = 10 * deviceWidth * (i - 1) / (nNodesX - 1)
    y = 10 * deviceLength * (j - 1) / (nNodesY - 1)
    If shape = "Uniform" Then
        sArray(i, j) = 1
    End If
    If shape = "Point" And rPointZ = 0 Then     'z>0 is buried, not surface illumination
        x0 = rPointX * deviceWidth / 10             'Measure from the side view plane
        y0 = (100 - rPointY) * deviceLength / 10    'Measure from the front view plane
        r0 = rPointDiameter / 2
        r = Sqr((x - x0) ^ 2 + (y - y0) ^ 2)
        sArray(i, j) = Exp(-r ^ 2 / r0 ^ 2)
    End If
    If shape = "Line" Then
        x0 = rLinePeak * deviceWidth / 10
        y0 = (100 - rLinePeak) * deviceLength / 10
        r0 = rLineWidth / 2
        If lineDirection = "Side" Then
            If lineShape = "Box" Then      'Uniform Line
                If Abs(y - y0) <= r0 Then sArray(i, j) = 1
            Else                        'Gaussian Line
                sArray(i, j) = Exp(-(y - y0) ^ 2 / r0 ^ 2)
            End If
        End If
        If lineDirection = "Face" Then
            If lineShape = "Box" Then      'Uniform Line
                If Abs(x - x0) <= r0 Then sArray(i, j) = 1
            Else                        'Gaussian Line
                sArray(i, j) = Exp(-(x - x0) ^ 2 / r0 ^ 2)
            End If
        End If
        If lineDirection = "Diagonal" Then
            a = deviceLength / deviceWidth      'Aspect ratio
            r = Sqr((y - a * x) ^ 2 / (a ^ 2 + 1))
            If lineShape = "Box" Then      'Uniform Line
                If r <= r0 Then sArray(i, nNodesY - j + 1) = 1
            Else                        'Gaussian Line
                sArray(i, nNodesY - j + 1) = Exp(-r ^ 2 / r0 ^ 2)
            End If
        End If
    End If
Next j: Next i
End Sub

Sub TwoDiodeModel(ByVal bIncludeJL As Boolean, ByVal bIncludeGsh As Boolean, ByVal bIncludeRs As Boolean)
'Fits a two-diode circuit model to the given set of I-V data. Minimum 5 data points.
'Posts values for JL, Jo1, Jo2, Gsh, Rs, RMS error, Pmax, and Voc on the Models sheet
'If bIncludeJL, then dataIV current is assumed out of device, otherwise current is assumed into the device
Dim Rminus As Double, Rmid As Double, Rplus As Double
Dim RMSminus As Double, RMSmid As Double, RMSplus As Double
Dim a As Double, b As Double, c As Double
Dim JL As Double, Jo1 As Double, Jo2 As Double, Gsh As Double, Rs As Double, RMSerror As Double
Rminus = 0: Rmid = 0: Rplus = 0: Rs = 0
RMSminus = 0: RMSmid = 0: RMSplus = 0
Do
    RMSerror = TwoDiodeFit(bIncludeJL, bIncludeGsh, Rs, JL, Jo1, Jo2, Gsh)
    Rminus = Rmid: Rmid = Rplus: Rplus = Rs
    RMSminus = RMSmid: RMSmid = RMSplus: RMSplus = RMSerror
    If (RMSminus > 0 And RMSplus > RMSmid) Or Not bIncludeRs Then Exit Do
    Rs = Rs + 0.02
Loop While Rs < 10
If Rs > 10 Then Rs = 10
If bIncludeRs And Rplus > Rmid And Rmid > Rminus Then
    If ParabolicFit(Rminus, Rmid, Rplus, RMSminus, RMSmid, RMSplus, a, b, c) Then Rs = -b / (2 * a) Else Rs = 0
    RMSerror = TwoDiodeFit(bIncludeJL, bIncludeGsh, Rs, JL, Jo1, Jo2, Gsh)
End If
Range(modelJLCell).Value = JL
Range(modelJo1Cell).Value = Jo1 * 1E+15
Range(modelJo2Cell).Value = Jo2 * 1000000000#
Range(modelGshCell).Value = Gsh * 1000#
Range(modelRsCell).Value = Rs
Range(modelTCell).Value = (rVt / rVt300) * 300 - 273.15
If bIncludeJL Then Range(modelRMSCell).Value = RMSerror Else Range(modelRMSCell).Value = ""
If JL > 0 Then Range(modelPmaxCell).Value = TwoDiodePmax(JL, Jo1, Jo2, Gsh, Rs) Else Range(modelPmaxCell).Value = ""
If JL > 0 Then Range(modelVocCell).Value = TwoDiodeVoc(JL, Jo1, Jo2, Gsh) Else Range(modelVocCell).Value = ""
If Gsh < 0 Then Call TwoDiodeModel(bIncludeJL, False, bIncludeRs)    'Repeat fitting routine without Gsh term if initial Gsh<0 (Warning: recursive!)
End Sub

Sub FreeEnergyAnalysis()
'Uses the current solution to evaluate free energy rates (powers) due to various loss mechanisms
Dim rPowGeneration As Double, rPowTransportBulk As Double
Dim rPowSeries As Double, rPowContactFront As Double, rPowContactRear As Double
Dim rPowTransportFront As Double, rPowTransportRear As Double
Dim rPowRecombBulk As Double, rPowRecombFront As Double, rPowRecombRear As Double
Dim i As Integer, j As Integer, k As Integer, f As Double, rLocalP As Double, rLocalN As Double, rJ2 As Double, rK2 As Double
'Calculate global power components
rPowSeries = rSeries * Range(terminalCurrentCell) ^ 2
'Calculate volumetric power components
If Not bGradGammaPsi Then Call CalculateGradGammaPsi
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    f = 1
    If i = 1 Or i = nNodesX Then f = 0.5 * f
    If j = 1 Or j = nNodesY Then f = 0.5 * f
    If k = 1 Or k = nNodesZ Then f = 0.5 * f
    rLocalP = rEqP + vGamma(i, j, k)
    rLocalN = rEqN + vGamma(i, j, k)        'J dot gradPhi for holes and electrons
    rPowTransportBulk = rPowTransportBulk - f * (-q * rDp * rLocalP * vGradPsiX(i, j, k) / rVt - q * rDp * vGradGammaX(i, j, k)) * (vGradPsiX(i, j, k) + rVt * vGradGammaX(i, j, k) / rLocalP) _
                                                                         - f * (-q * rDp * rLocalP * vGradPsiY(i, j, k) / rVt - q * rDp * vGradGammaY(i, j, k)) * (vGradPsiY(i, j, k) + rVt * vGradGammaY(i, j, k) / rLocalP) _
                                                                         - f * (-q * rDp * rLocalP * vGradPsiZ(i, j, k) / rVt - q * rDp * vGradGammaZ(i, j, k)) * (vGradPsiZ(i, j, k) + rVt * vGradGammaZ(i, j, k) / rLocalP)
    rPowTransportBulk = rPowTransportBulk - f * (-q * rDn * rLocalN * vGradPsiX(i, j, k) / rVt + q * rDn * vGradGammaX(i, j, k)) * (vGradPsiX(i, j, k) - rVt * vGradGammaX(i, j, k) / rLocalN) _
                                                                         - f * (-q * rDn * rLocalN * vGradPsiY(i, j, k) / rVt + q * rDn * vGradGammaY(i, j, k)) * (vGradPsiY(i, j, k) - rVt * vGradGammaY(i, j, k) / rLocalN) _
                                                                         - f * (-q * rDn * rLocalN * vGradPsiZ(i, j, k) / rVt + q * rDn * vGradGammaZ(i, j, k)) * (vGradPsiZ(i, j, k) - rVt * vGradGammaZ(i, j, k) / rLocalN)
    rPowGeneration = rPowGeneration + f * q * vGeneration(i, j, k) * rVt * Log(rLocalP * rLocalN / rNi ^ 2)
    rPowRecombBulk = rPowRecombBulk + f * q * vRecombination(i, j, k) * rVt * Log(rLocalP * rLocalN / rNi ^ 2)
Next k: Next j: Next i
rPowTransportBulk = rPowTransportBulk * deviceHeight / ((nNodesX - 1) * (nNodesY - 1) * (nNodesZ - 1))
rPowGeneration = rPowGeneration * deviceHeight / ((nNodesX - 1) * (nNodesY - 1) * (nNodesZ - 1))
rPowRecombBulk = rPowRecombBulk * deviceHeight / ((nNodesX - 1) * (nNodesY - 1) * (nNodesZ - 1))
'Calculate surface power components
For i = 1 To nNodesX: For j = 1 To nNodesY
    f = 1
    If i = 1 Or i = nNodesX Then f = 0.5 * f
    If j = 1 Or j = nNodesY Then f = 0.5 * f
    rLocalP = rEqP + vGamma(i, j, 1)
    rLocalN = rEqN + vGamma(i, j, 1)
    If sSigma(front, i, j) > 0 Then    '-K dot GradPhiP
        rPowTransportFront = rPowTransportFront - f * LateralNodeCurrentX(front, i, j) * (vGradPsiX(i, j, 1) + rVt * vGradGammaX(i, j, 1) / rLocalP) _
                                                                               - f * LateralNodeCurrentY(front, i, j) * (vGradPsiY(i, j, 1) + rVt * vGradGammaY(i, j, 1) / rLocalP)
    Else    '-K dot GradPhiN
        rPowTransportFront = rPowTransportFront - f * LateralNodeCurrentX(front, i, j) * (vGradPsiX(i, j, 1) - rVt * vGradGammaX(i, j, 1) / rLocalN) _
                                                                               - f * LateralNodeCurrentY(front, i, j) * (vGradPsiY(i, j, 1) - rVt * vGradGammaY(i, j, 1) / rLocalN)
    End If
    rLocalP = rEqP + vGamma(i, j, nNodesZ)
    rLocalN = rEqN + vGamma(i, j, nNodesZ)
    If sSigma(rear, i, j) > 0 Then '-K dot GradPhiP
        rPowTransportRear = rPowTransportRear - f * LateralNodeCurrentX(rear, i, j) * (vGradPsiX(i, j, nNodesZ) + rVt * vGradGammaX(i, j, nNodesZ) / rLocalP) _
                                                                              - f * LateralNodeCurrentY(rear, i, j) * (vGradPsiY(i, j, nNodesZ) + rVt * vGradGammaY(i, j, nNodesZ) / rLocalP)
    Else    '-K dot GradPhiN
        rPowTransportRear = rPowTransportRear - f * LateralNodeCurrentX(rear, i, j) * (vGradPsiX(i, j, nNodesZ) - rVt * vGradGammaX(i, j, nNodesZ) / rLocalN) _
                                                                              - f * LateralNodeCurrentY(rear, i, j) * (vGradPsiY(i, j, nNodesZ) - rVt * vGradGammaY(i, j, nNodesZ) / rLocalN)
    End If
    If Not bOpenCircuit Then        'Contact resistance loss
        If pType Then
            rPowContactFront = rPowContactFront _
                + f * sContact(front, holes, i, j) * PhiP(sFrontGamma(i, j), sFrontPsi(i, j)) ^ 2 _
                + f * sContact(front, electrons, i, j) * (PhiN(sFrontGamma(i, j), sFrontPsi(i, j)) + rVoltage) ^ 2
        Else
            rPowContactFront = rPowContactFront _
                + f * sContact(front, holes, i, j) * (PhiP(sFrontGamma(i, j), sFrontPsi(i, j)) - rVoltage) ^ 2 _
                + f * sContact(front, electrons, i, j) * PhiN(sFrontGamma(i, j), sFrontPsi(i, j)) ^ 2
        End If
        If pType Then
            rPowContactRear = rPowContactRear _
                + f * sContact(rear, holes, i, j) * PhiP(sRearGamma(i, j), sRearPsi(i, j)) ^ 2 _
                + f * sContact(rear, electrons, i, j) * (PhiN(sRearGamma(i, j), sRearPsi(i, j)) + rVoltage) ^ 2
        Else
            rPowContactRear = rPowContactRear _
                + f * sContact(rear, holes, i, j) * (PhiP(sRearGamma(i, j), sRearPsi(i, j)) - rVoltage) ^ 2 _
                + f * sContact(rear, electrons, i, j) * PhiN(sRearGamma(i, j), sRearPsi(i, j)) ^ 2
        End If
    End If
    rPowRecombFront = rPowRecombFront + f * q * sFrontRecomb(i, j) * (sPhi(front, holes, i, j) - sPhi(front, electrons, i, j))
    rPowRecombRear = rPowRecombRear + f * q * sRearRecomb(i, j) * (sPhi(rear, holes, i, j) - sPhi(rear, electrons, i, j))
Next j: Next i
rPowTransportFront = rPowTransportFront / ((nNodesX - 1) * (nNodesY - 1))
rPowTransportRear = rPowTransportRear / ((nNodesX - 1) * (nNodesY - 1))
rPowContactFront = rPowContactFront / ((nNodesX - 1) * (nNodesY - 1))
rPowContactRear = rPowContactRear / ((nNodesX - 1) * (nNodesY - 1))
rPowRecombFront = rPowRecombFront / ((nNodesX - 1) * (nNodesY - 1))
rPowRecombRear = rPowRecombRear / ((nNodesX - 1) * (nNodesY - 1))
'Post FELA results on the Models sheet
Range(modelGenCell).Value = rPowGeneration
Range(modelRecombCell).Value = rPowRecombFront + rPowRecombRear + rPowRecombBulk
Range(modelTransportCell).Value = rPowTransportFront + rPowTransportRear + rPowTransportBulk
Range(modelResistanceCell).Value = rPowContactFront + rPowContactRear + rPowSeries
Range(modelTotalCell).Value = Range(modelRecombCell) + Range(modelTransportCell) + Range(modelResistanceCell)
Range(modelRecombFrontCell).Value = rPowRecombFront: Range(modelRecombRearCell).Value = rPowRecombRear: Range(modelRecombBulkCell).Value = rPowRecombBulk
Range(modelTransportFrontCell).Value = rPowTransportFront: Range(modelTransportRearCell).Value = rPowTransportRear: Range(modelTransportBulkCell).Value = rPowTransportBulk
Range(modelResistanceFrontCell).Value = rPowContactFront: Range(modelResistanceRearCell).Value = rPowContactRear: Range(modelResistanceSeriesCell).Value = rPowSeries
Range(modelTotalFrontCell).Value = rPowRecombFront + rPowTransportFront + rPowContactFront
Range(modelTotalRearCell).Value = rPowRecombRear + rPowTransportRear + rPowContactRear
Range(modelTotalBulkCell).Value = rPowRecombBulk + rPowTransportBulk + rPowSeries
If Range(modelGenCell) > 0 Then
    Range(modelGenPctCell).Value = 1
    Range(modelRecombPctCell).Value = Range(modelRecombCell) / Range(modelGenCell)
    Range(modelTransportPctCell).Value = Range(modelTransportCell) / Range(modelGenCell)
    Range(modelResistancePctCell).Value = Range(modelResistanceCell) / Range(modelGenCell)
    Range(modelTotalPctCell).Value = Range(modelTotalCell) / Range(modelGenCell)
    Range(modelRecombFrontPctCell).Value = Range(modelRecombFrontCell) / Range(modelGenCell)
    Range(modelRecombRearPctCell).Value = Range(modelRecombRearCell) / Range(modelGenCell)
    Range(modelRecombBulkPctCell).Value = Range(modelRecombBulkCell) / Range(modelGenCell)
    Range(modelTransportFrontPctCell).Value = Range(modelTransportFrontCell) / Range(modelGenCell)
    Range(modelTransportRearPctCell).Value = Range(modelTransportRearCell) / Range(modelGenCell)
    Range(modelTransportBulkPctCell).Value = Range(modelTransportBulkCell) / Range(modelGenCell)
    Range(modelResistanceFrontPctCell).Value = Range(modelResistanceFrontCell) / Range(modelGenCell)
    Range(modelResistanceRearPctCell).Value = Range(modelResistanceRearCell) / Range(modelGenCell)
    Range(modelResistanceSeriesPctCell).Value = Range(modelResistanceSeriesCell) / Range(modelGenCell)
    Range(modelTotalFrontPctCell).Value = Range(modelTotalFrontCell) / Range(modelGenCell)
    Range(modelTotalRearPctCell).Value = Range(modelTotalRearCell) / Range(modelGenCell)
    Range(modelTotalBulkPctCell).Value = Range(modelTotalBulkCell) / Range(modelGenCell)
Else
    If Range(modelTotalCell) > 0 Then
        Range(modelGenPctCell).Value = ""
        Range(modelRecombPctCell).Value = Range(modelRecombCell) / Range(modelTotalCell)
        Range(modelTransportPctCell).Value = Range(modelTransportCell) / Range(modelTotalCell)
        Range(modelResistancePctCell).Value = Range(modelResistanceCell) / Range(modelTotalCell)
        Range(modelTotalPctCell).Value = 1
        Range(modelRecombFrontPctCell).Value = Range(modelRecombFrontCell) / Range(modelTotalCell)
        Range(modelRecombRearPctCell).Value = Range(modelRecombRearCell) / Range(modelTotalCell)
        Range(modelRecombBulkPctCell).Value = Range(modelRecombBulkCell) / Range(modelTotalCell)
        Range(modelTransportFrontPctCell).Value = Range(modelTransportFrontCell) / Range(modelTotalCell)
        Range(modelTransportRearPctCell).Value = Range(modelTransportRearCell) / Range(modelTotalCell)
        Range(modelTransportBulkPctCell).Value = Range(modelTransportBulkCell) / Range(modelTotalCell)
        Range(modelResistanceFrontPctCell).Value = Range(modelResistanceFrontCell) / Range(modelTotalCell)
        Range(modelResistanceRearPctCell).Value = Range(modelResistanceRearCell) / Range(modelTotalCell)
        Range(modelResistanceSeriesPctCell).Value = Range(modelResistanceSeriesCell) / Range(modelTotalCell)
        Range(modelTotalFrontPctCell).Value = Range(modelTotalFrontCell) / Range(modelTotalCell)
        Range(modelTotalRearPctCell).Value = Range(modelTotalRearCell) / Range(modelTotalCell)
        Range(modelTotalBulkPctCell).Value = Range(modelTotalBulkCell) / Range(modelTotalCell)
    Else
        Call ClearFreeEnergyAnalysis
    End If
End If
End Sub

