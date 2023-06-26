Attribute VB_Name = "Doping"
Option Explicit

Sub CalculateDoping()
'Calculates the dopant distribution after a given time resulting from Fickian diffusion from a uniform surface dose
'The diffusion-depth parameter is sqrt (4 D T), where T is the total diffusion time.
'The diffusion type is either gaussian or erfc.
'Gaussian diffusion maintains a fixed dose (dose refers to dopants per cm2 of projected area)
'Erfc diffusion maintains a fixed surface concentration
'In 1D, surface is corrected to the specified surface value (deviation is numerical approx. error)
Dim i As Integer, j As Integer, k As Integer, nStep As Integer
Dim nTimeSteps As Long
Dim rTimeStep As Double, rDose As Double
ReDim vDopingP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vDopingN(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
nTimeSteps = Round((2 * rDiffusionDepth / rDeltaZ) ^ 2, 0) + 10
rTimeStep = (rDiffusionDepth ^ 2 / 4) / nTimeSteps
Range(progressCell) = "Diffusion"
'Initialize surface doping
For i = 1 To nNodesX: For j = 1 To nNodesY
    Select Case nDiffusionType
        Case ptype
            Select Case nDiffusionShape
                Case gaussian: vDopingP(i, j, sTexture(i, j)) = rDiffusionPeak * Sqr(pi) * rDiffusionDepth / (rCosTextureAngle * rDeltaZ)
                Case erfc: vDopingP(i, j, sTexture(i, j)) = rDiffusionPeak
            End Select
        Case ntype
            Select Case nDiffusionShape
                Case gaussian: vDopingN(i, j, sTexture(i, j)) = rDiffusionPeak * Sqr(pi) * rDiffusionDepth / (rCosTextureAngle * rDeltaZ)
                Case erfc: vDopingN(i, j, sTexture(i, j)) = rDiffusionPeak
            End Select
    End Select
Next j: Next i
'Calculate dose to make it available for debugging purposes only
Select Case nDiffusionType
    Case ptype: rDose = VolumeAverage(vDopingP) * deviceHeight
    Case ntype: rDose = VolumeAverage(vDopingN) * deviceHeight
End Select
If Range(diffusionShapeCell) <> "none" Then
    'Take time steps to implement diffusion process
    For nStep = 1 To nTimeSteps
        Call SolveTimeStep(rTimeStep)
    Next nStep
    'Refine front surface doping for 1D solutions
    If nDimensions = 1 Then
        For i = 1 To nNodesX: For j = 1 To nNodesY
            Select Case nDiffusionType
                Case ptype: vDopingP(i, j, 1) = rDiffusionPeak
                Case ntype: vDopingN(i, j, 1) = rDiffusionPeak
            End Select
        Next j: Next i
    End If
    'Calculate dose to make it available for debugging purposes
    Select Case nDiffusionType
        Case ptype: rDose = VolumeAverage(vDopingP) * deviceHeight
        Case ntype: rDose = VolumeAverage(vDopingN) * deviceHeight
    End Select
End If
'Add-in the background doping
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    Select Case nBackgroundType
        Case ptype
            vDopingP(i, j, k) = vDopingP(i, j, k) + rBackgroundDoping
        Case ntype
            vDopingN(i, j, k) = vDopingN(i, j, k) + rBackgroundDoping
    End Select
Next k: Next j: Next i
'Diffusivities
rBackgroundDp = SiHoleDiffusivity(rBackgroundP, rBackgroundN)
rBackgroundDn = SiElectronDiffusivity(rBackgroundP, rBackgroundN)
Range(resistivityCell).Value = rVt / (rQ * (rBackgroundP * rBackgroundDp + rBackgroundN * rBackgroundDn))
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    vDp(i, j, k) = SiHoleDiffusivity(vDopingP(i, j, k), vDopingN(i, j, k))
    vDn(i, j, k) = SiElectronDiffusivity(vDopingP(i, j, k), vDopingN(i, j, k))
Next k: Next j: Next i
'Bandgap Narrowing
Call CalculateBGN(holes)
Call CalculateBGN(electrons)
End Sub

Sub SolveTimeStep(ByVal rTimeStep As Double)
'Each time step is L^2/4D divided by nTimeSteps, so that the total diffusion time matches the given diffusion depth
Dim i As Integer, j As Integer, k As Integer
Dim vUpdate As Variant
ReDim vUpdate(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    Select Case nDiffusionType
        Case ptype: vUpdate(i, j, k) = vDopingP(i, j, k) + DopantIncrease(i, j, k, rTimeStep)
        Case ntype: vUpdate(i, j, k) = vDopingN(i, j, k) + DopantIncrease(i, j, k, rTimeStep)
    End Select
    If vUpdate(i, j, k) < 0 Then vUpdate(i, j, k) = 0
Next k: Next j: Next i
'Now apply update to global doping variable
Select Case nDiffusionType
    Case ptype: vDopingP = vUpdate
    Case ntype: vDopingN = vUpdate
End Select
End Sub

Function DopantIncrease(ByVal i As Integer, ByVal j As Integer, ByVal k As Integer, ByVal rTimeStep As Double) As Double
'Finds increase in vDoping at the given node resulting from Fickian in-diffusion over a time step equal to L^2/4D divided by the global value nDiffusionTimeSteps
Dim nLeft As Integer, nRight As Integer, nRear As Integer, nFront As Integer, nTop As Integer, nBot As Integer
Dim localVolumeFraction As Double
DopantIncrease = 0
Select Case nDiffusionShape
    Case gaussian:: If k < sTexture(i, j) Then Exit Function
    Case erfc: If k <= sTexture(i, j) Then Exit Function        'Don't change surface for erfc diffusion
End Select
'Assign near-nodes, utilizing symmetry planes where applicable
If i = 1 Then nLeft = i + 1 Else nLeft = i - 1
If i = nNodesX Then nRight = i - 1 Else nRight = i + 1
If j = 1 Then nRear = j + 1 Else nRear = j - 1
If j = nNodesY Then nFront = j - 1 Else nFront = j + 1
If k = nNodesZ Then nBot = k - 1 Else nBot = k + 1
If k = sTexture(i, j) Then
    nTop = k
    localVolumeFraction = VolumeFraction(i, j, nLeft, nRight, nRear, nFront)
Else
    nTop = k - 1
    localVolumeFraction = 1
End If
Select Case nDiffusionType
    Case ptype
        DopantIncrease = (k2x * (vDopingP(nLeft, j, k) + vDopingP(nRight, j, k)) _
                                 + k2y * (vDopingP(i, nRear, k) + vDopingP(i, nFront, k)) _
                                  + k2z * (vDopingP(i, j, nTop) + vDopingP(i, j, nBot)) _
                                  - 2 * k2xyz * vDopingP(i, j, k)) * rTimeStep / localVolumeFraction
    Case ntype
        DopantIncrease = (k2x * (vDopingN(nLeft, j, k) + vDopingN(nRight, j, k)) _
                                 + k2y * (vDopingN(i, nRear, k) + vDopingN(i, nFront, k)) _
                                  + k2z * (vDopingN(i, j, nTop) + vDopingN(i, j, nBot)) _
                                  - 2 * k2xyz * vDopingN(i, j, k)) * rTimeStep / localVolumeFraction
End Select
End Function

Sub CalculateBGN(nVariable As Integer)
Dim i As Integer, j As Integer, k As Integer
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = 1 To nNodesZ
    If vDopingP(i, j, k) > rVpOnset Then vVp(i, j, k) = rVpSlope * Log(vDopingP(i, j, k) / rVpOnset)
    If vDopingN(i, j, k) > rVnOnset Then vVn(i, j, k) = rVnSlope * Log(vDopingN(i, j, k) / rVnOnset)
    If (bEnablePointDefect And PointDefect(i, j, k)) Or _
       (bEnableLineDefect And LineDefect(i, j, k)) Or _
       (bEnablePlaneDefect And PlaneDefect(i, j, k)) Then
        vVp(i, j, k) = vVp(i, j, k) + rDefectBGN / 2
        vVn(i, j, k) = vVn(i, j, k) + rDefectBGN / 2
    End If
Next k: Next j: Next i
End Sub


