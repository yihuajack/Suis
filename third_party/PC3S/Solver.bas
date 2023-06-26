Attribute VB_Name = "Solver"
Option Explicit
'This module contains the code that takes the parameters that describe the device and solves for phip, phin, and psi.
'Solve calls subroutines that involve lengthy iterations, typically of indeterminate time.
'Calculate refers to subroutines that are deterministic in their execution time (see Calculations module).
'Initialze refers to subroutines that set initial values (GlobalVariables module)
'Subroutines that are implemented as Functions and return a numeric value are in the Functions module

Sub Solve()
'This is called when the Solve button is pushed on the Dashboard sheet.
'Progress is posted by subroutines.
If Not bDebug Then On Error GoTo ErrHandler:
If bNeedReset Then Call ResetAll    'Typically only required upon initial startup
rStartTime = Timer
Call ResetOutputs
If nSolutionMode = lateral Then Range(illuminationCell) = "Dark"
Call MapInputParameters
ReDim vGeneration(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
If bIllumination Then Call SolveGeneration
If nSolutionMode = opencircuit And rTotalGeneration = 0 Then
    Range(progressCell).Value = "null"
Else
    Call SolveNodes
End If
Exit Sub
    
ErrHandler:
    Range(progressCell).Value = "Error"
    If Not bBatch Then MsgBox ("Execution Error: Reset required.")
End Sub

Sub InitializePhi()
'This routine initializes phip and phin depending on the contact mode. For open-circuit, a rough estimate of Voc is used for the splitting.
Dim i As Integer, j As Integer, k As Integer
Dim rVoc As Double, rTauMin As Double
ReDim vPhiP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPhiN(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
Select Case nSolutionMode
    Case bipolar
        For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
            vPhiP(i, j, k) = rVoltage / 2
            vPhiN(i, j, k) = -rVoltage / 2
        Next k: Next j: Next i
    Case floating
        For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
            vPhiP(i, j, k) = rVoltage / 2
            vPhiN(i, j, k) = -rVoltage / 2
        Next k: Next j: Next i
    Case opencircuit
        If rTotalGeneration * rBackgroundDoping * rTauN * rNi = 0 Then
            rVoc = 0
        Else
            If ptype Then rTauMin = rTauN Else rTauMin = rTauP
            rVoc = rVt * Log(rTotalGeneration * Abs(rBackgroundDoping) * rTauN / rNi ^ 2)
        End If
        For i = 1 To nNodesX: For j = 1 To nNodesY:  For k = sTexture(i, j) To nNodesZ
            vPhiP(i, j, k) = rVoc / 2
            vPhiN(i, j, k) = -rVoc / 2
        Next k: Next j: Next i
    Case flatband
        For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
            vPhiP(i, j, k) = rVoltage / 2
            vPhiN(i, j, k) = -rVoltage / 2
        Next k: Next j: Next i
    Case lateral
        For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
            Select Case nSurfaceType
                Case ptype: vPhiP(i, j, k) = rVoltage * (0.5 - (i - 1) / (nNodesX - 1))
                Case ntype: vPhiN(i, j, k) = rVoltage * (0.5 - (i - 1) / (nNodesX - 1))
            End Select
        Next k: Next j: Next i
End Select
End Sub

Sub InitializePsi()
'This routine sets psi at each point equal to the charge-neutral value at that point.
Dim i As Integer, j As Integer, k As Integer
Dim rDoping As Double, rV As Double
ReDim vPsi(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    rDoping = vDopingP(i, j, k) - vDopingN(i, j, k)
    rV = vPhiP(i, j, k) + vVp(i, j, k) - vPhiN(i, j, k) + vVn(i, j, k)
    If rDoping > 0 Then
        vPsi(i, j, k) = vPhiP(i, j, k) + vVp(i, j, k) - rVt * Log((rDoping) / (2 * rNi) + Sqr((rDoping / (2 * rNi)) ^ 2 + ExpPower(rV / rVt)))
    Else
        vPsi(i, j, k) = vPhiN(i, j, k) - vVn(i, j, k) + rVt * Log((-rDoping) / (2 * rNi) + Sqr((rDoping / (2 * rNi)) ^ 2 + ExpPower(rV / rVt)))
    End If
Next k: Next j: Next i
End Sub

Function SolveNodes() As Boolean
'The function returns false if an error occurs that results in a halt of execution.
Dim iterationCounter As Integer, nLoopCounter As Integer, nPsiCounter As Integer
Dim maxDelta As Double, maxDeltaPhiP As Double, maxDeltaPhiN As Double, maxDeltaPsi As Double
Dim previousError As Double, rError As Double, saveRecombination As Double, deltaV As Double
Dim saveLateralCurrent As Double
Dim savePlot As String
Const nPsiLoops = 20
Const nLoops = 10

SolveNodes = False
bConvergenceFailure = False
If Not bDebug Then On Error GoTo ErrHandler:

Call ResetElectrical
Call InitializePhi      'Flat bands (actually shallow lateral gradient for lateral mode)
Call InitializePsi      'Charge neutral
Call CalculateSurfaceVelocity        'Surface recombination velocity as a function of location on the textured surface
Call CalculateSurfaceCharge        'Surface charge as a function of location on the textured surface

'Solve flat-band condition for Psi
savePlot = currentPlot
If currentPlot = "Doping" Then currentPlot = "Potential"
Range(progressCell) = "Flat Band"
For nLoopCounter = 1 To nMaxIterations
    Range(iterationCell).Value = nLoopCounter: DoEvents
    For nPsiCounter = 1 To nPsiLoops
        maxDeltaPsi = RefineLocal(psi)
    Next nPsiCounter
    Range(errorCell) = Abs(maxDeltaPsi)
    If maxDeltaPsi < rConvergence Then Exit For
    If nDimensions > 2 Then Call OnPlotChange
    Range(solutionTimeCell) = Timer - rStartTime
Next nLoopCounter
Range(iterationCell).Value = ""
currentPlot = savePlot

If nSolutionMode <> lateral Then
    Call CalculateRecombination
    rBulkRecombination = VolumeAverage(vRecombination) * deviceHeight
    rSurfaceRecombination = SurfaceAverage(sRecombination) / rCosTextureAngle
    rTotalRecombination = rBulkRecombination + rSurfaceRecombination
    Range(bulkRecombinationCell).Value = rQ * rBulkRecombination
    Range(surfaceRecombinationCell).Value = rQ * rSurfaceRecombination
    Range(totalRecombinationCell).Value = rQ * rTotalRecombination
End If

If (rTotalGeneration = 0 And rVoltage = 0) Or nSolutionMode = flatband Then   'Flat-band is the final solution
    SolveNodes = True
    bConvergenceFailure = False
    Call OnPlotChange
    Exit Function
End If

previousError = 1
If nSolutionMode = lateral Then
    If (currentPlot <> "Quasi-Fermi" And currentPlot <> "Current" And currentPlot <> "Velocity") Or nSelectCarrier <> nSurfaceType Then
        currentPlot = "Quasi-Fermi"
        Select Case nSurfaceType
            Case holes
                Range(selectCarrierCell).Value = "Holes": nSelectCarrier = holes
            Case electrons
                Range(selectCarrierCell).Value = "Electrons": nSelectCarrier = electrons
        End Select
    End If
Else
    If (currentPlot <> "Quasi-Fermi" And currentPlot <> "Current" And currentPlot <> "Velocity") Then currentPlot = "Recombination"
End If
Range(progressCell) = "Solving"
bIntermediatePlot = True

For iterationCounter = 1 To nMaxIterations
    saveRecombination = rTotalRecombination
    saveLateralCurrent = rLateralCurrent
    Range(iterationCell).Value = iterationCounter: DoEvents
    
    'Apply special Phi adjustment in open-circuit mode to accelerate convergence
    If nSolutionMode = opencircuit And rTotalGeneration > 0 And rTotalRecombination > 0 Then
        deltaV = rVt * Log(rTotalGeneration / rTotalRecombination)
        Call AdjustPhiSplit(deltaV)
    End If
    
    For nLoopCounter = 1 To nLoops
        maxDeltaPsi = RefineLocal(psi)
        If nSolutionMode = lateral Then      'monopolar solution
            Select Case nSurfaceType
                Case ptype
                    Call CalculateSigma(holes)
                    maxDeltaPhiP = RefineLocal(holes)
                Case ntype
                    Call CalculateSigma(electrons)
                    maxDeltaPhiN = RefineLocal(electrons)
            End Select
        Else            'bipolar solution
            Call CalculateSigma(holes)
            Call CalculateSigma(electrons)
            maxDeltaPhiP = RefineLocal(holes)
            maxDeltaPhiN = RefineLocal(electrons)
            Call CalculateRecombination
        End If
        DoEvents
    Next nLoopCounter
    
    'Post Numerical Solution Details
    Range(solutionTimeCell) = Timer - rStartTime
    Range(deltaPCell).Value = maxDeltaPhiP
    Range(deltaNCell).Value = maxDeltaPhiN
    Range(deltaPsiCell).Value = maxDeltaPsi
        
    rBulkRecombination = VolumeAverage(vRecombination) * deviceHeight
    rSurfaceRecombination = SurfaceAverage(sRecombination) / rCosTextureAngle
    rTotalRecombination = rBulkRecombination + rSurfaceRecombination
    If nSolutionMode = lateral Then
        rLateralCurrent = LateralCurrent((nNodesX - 1) \ 2 + 1)
        Range(lateralCurrentCell).Value = rLateralCurrent
    Else
        Range(bulkRecombinationCell).Value = rQ * rBulkRecombination
        Range(surfaceRecombinationCell).Value = rQ * rSurfaceRecombination
        Range(totalRecombinationCell).Value = rQ * rTotalRecombination
    End If
    
    Select Case nSolutionMode
        Case opencircuit: rError = Abs((rTotalGeneration - rTotalRecombination) / rTotalGeneration)
        Case bipolar: rError = Abs((rTotalRecombination - saveRecombination) / WorksheetFunction.max(rTotalGeneration, Abs(saveRecombination)))
        Case floating: rError = Abs((rTotalRecombination - saveRecombination) / WorksheetFunction.max(rTotalGeneration, Abs(saveRecombination)))
        Case lateral: rError = Abs((rLateralCurrent - saveLateralCurrent) / WorksheetFunction.max(saveLateralCurrent, rLateralCurrent))
    End Select
    Range(errorCell).Value = rError
    If rError < rConvergence And previousError < 2 * rConvergence Then Exit For 'Ensure it's not a fluke.
    If Abs(rQ * rTotalRecombination) > 1000# Then iterationCounter = nMaxIterations        'Exit loop if recombination gets excessively large
    previousError = rError
    If nDimensions > 1 Then Call OnPlotChange
Next iterationCounter

bIntermediatePlot = False
bConvergenceFailure = iterationCounter > nMaxIterations
If bConvergenceFailure Then Range(progressCell).Value = "Unsolved" Else Range(progressCell).Value = "Solved"
Call OnPlotChange
SolveNodes = Not bConvergenceFailure
Exit Function

ErrHandler:
    Range(progressCell).Value = "Error"
    If Not bScan And Not bBatch Then MsgBox ("Execution Error: Reset required.")
    SolveNodes = False
ExitHandler:
    Call ResetOutputs
End Function

Function RefineLocal(ByVal nVariable As Integer) As Double
'Assumes that gradients required for the specified variable have been calculated
'Updates are limited to Vt and dampened by a fixed factor to improve convergence robustness.
'Function returns maximum absolute value of the change in the specified potential
Dim i As Integer, j As Integer, k As Integer
Dim dPhiP As Double, dPhiN As Double, dPsi As Double
Dim maxDelta As Double, dampingPhi As Double
Dim signDelta As Integer
Dim vUpdateP As Variant, vUpdateN As Variant, vUpdatePsi As Variant
Select Case nVariable
    Case holes
        ReDim vUpdateP(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
        vUpdateP = vPhiP
    Case electrons
        ReDim vUpdateN(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
        vUpdateN = vPhiN
    Case psi
        ReDim vUpdatePsi(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
        vUpdatePsi = vPsi
End Select
If rVoltage < 0 Then dampingPhi = rDamping * rVt / (Abs(rVoltage) + rVt) Else dampingPhi = rDamping
maxDelta = 0: signDelta = 0
'Don't change phip and phin at that carrier's contacted surfaces
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    Select Case nVariable
        Case holes
            If Not Contact(holes, i, j, k) Then
                dPhiP = Update(holes, i, j, k)
                If Abs(dPhiP) > maxDelta Then
                    maxDelta = Abs(dPhiP): signDelta = Sgn(dPhiP)
                End If
                dPhiP = dPhiP * rVt / (Abs(dPhiP) + rVt)
                vUpdateP(i, j, k) = vPhiP(i, j, k) + dampingPhi * dPhiP
            End If
        Case electrons
            If Not Contact(electrons, i, j, k) Then
                dPhiN = Update(electrons, i, j, k)
                If Abs(dPhiN) > maxDelta Then
                    maxDelta = Abs(dPhiN): signDelta = Sgn(dPhiN)
                End If
                dPhiN = dPhiN * rVt / (Abs(dPhiN) + rVt)
                vUpdateN(i, j, k) = vPhiN(i, j, k) + dampingPhi * dPhiN
            End If
        Case psi
            dPsi = Update(psi, i, j, k)
            If Abs(dPsi) > maxDelta Then
                maxDelta = Abs(dPsi): signDelta = Sgn(dPsi)
            End If
            dPsi = dPsi * rVt / (Abs(dPsi) + rVt)
            vUpdatePsi(i, j, k) = vPsi(i, j, k) + dPsi
    End Select
Next k: Next j: Next i
'Now apply update to global variable
Select Case nVariable
    Case holes: vPhiP = vUpdateP
    Case electrons: vPhiN = vUpdateN
    Case psi: vPsi = vUpdatePsi
End Select

RefineLocal = signDelta * maxDelta
End Function

Function Update(ByVal nVariable As Integer, ByVal i As Integer, ByVal j As Integer, ByVal k As Integer) As Double
Dim localP As Double, localN As Double, rDebye As Double
Dim localSurface As Double, localCharge As Double, localGR As Double
Dim localVolumeFraction As Double
Dim nLeft As Integer, nRight As Integer, nRear As Integer, nFront As Integer, nTop As Integer, nBot As Integer
Update = 0: If k < sTexture(i, j) Then Exit Function
localSurface = 0: localCharge = 0
'Assign near-nodes, utilizing symmetry planes where applicable
If i = 1 Then nLeft = i + 1 Else nLeft = i - 1
If i = nNodesX Then nRight = i - 1 Else nRight = i + 1
If j = 1 Then nRear = j + 1 Else nRear = j - 1
If j = nNodesY Then nFront = j - 1 Else nFront = j + 1
If k = nNodesZ Then nBot = k - 1 Else nBot = k + 1
If k = sTexture(i, j) Then
    nTop = k
    localVolumeFraction = VolumeFraction(i, j, nLeft, nRight, nRear, nFront)
    localSurface = sRecombination(i, j) / (rDeltaZ * rCosTextureAngle)
    localCharge = sCharge(i, j) / (rDeltaZ * rCosTextureAngle)
Else
    nTop = k - 1
    localVolumeFraction = 1
End If
Select Case nVariable
    Case holes
        localGR = (vGeneration(i, j, k) - vRecombination(i, j, k)) * localVolumeFraction - localSurface
        Update = (k2x * vSqrSigmaP(nLeft, j, k) * vPhiP(nLeft, j, k) + k2x * vSqrSigmaP(nRight, j, k) * vPhiP(nRight, j, k) _
                      + k2y * vSqrSigmaP(i, nRear, k) * vPhiP(i, nRear, k) + k2y * vSqrSigmaP(i, nFront, k) * vPhiP(i, nFront, k) _
                      + k2z * vSqrSigmaP(i, j, nTop) * vPhiP(i, j, nTop) + k2z * vSqrSigmaP(i, j, nBot) * vPhiP(i, j, nBot) + rQ * localGR / vSqrSigmaP(i, j, k)) _
                      / (k2x * vSqrSigmaP(nLeft, j, k) + k2x * vSqrSigmaP(nRight, j, k) + k2y * vSqrSigmaP(i, nRear, k) + k2y * vSqrSigmaP(i, nFront, k) + k2z * vSqrSigmaP(i, j, nTop) + k2z * vSqrSigmaP(i, j, nBot)) _
                     - vPhiP(i, j, k)
    Case electrons
        localGR = (vGeneration(i, j, k) - vRecombination(i, j, k)) * localVolumeFraction - localSurface
        Update = (k2x * vSqrSigmaN(nLeft, j, k) * vPhiN(nLeft, j, k) + k2x * vSqrSigmaN(nRight, j, k) * vPhiN(nRight, j, k) _
                      + k2y * vSqrSigmaN(i, nRear, k) * vPhiN(i, nRear, k) + k2y * vSqrSigmaN(i, nFront, k) * vPhiN(i, nFront, k) _
                      + k2z * vSqrSigmaN(i, j, nTop) * vPhiN(i, j, nTop) + k2z * vSqrSigmaN(i, j, nBot) * vPhiN(i, j, nBot) - rQ * localGR / vSqrSigmaN(i, j, k)) _
                      / (k2x * vSqrSigmaN(nLeft, j, k) + k2x * vSqrSigmaN(nRight, j, k) + k2y * vSqrSigmaN(i, nRear, k) + k2y * vSqrSigmaN(i, nFront, k) + k2z * vSqrSigmaN(i, j, nTop) + k2z * vSqrSigmaN(i, j, nBot)) _
                     - vPhiN(i, j, k)
    Case psi
        localP = rNi * ExpPower((vPhiP(i, j, k) - vPsi(i, j, k) + vVp(i, j, k)) / rVt)
        localN = rNi * ExpPower((vPsi(i, j, k) - vPhiN(i, j, k) + vVn(i, j, k)) / rVt)
        If k = sTexture(i, j) Then      'Special treatment required at surface
            If Abs(sCharge(i, j)) > rVt / (rQ_Eps * rDeltaZ * rCosTextureAngle) Then 'Use Debye length to adjust volume charge to account for the rapid change in potential that can occur near a charged surface
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
        End If
        localCharge = localCharge + (localP - localN + vDopingN(i, j, k) - vDopingP(i, j, k)) * localVolumeFraction
        Update = ((k2x * vPsi(nLeft, j, k) + k2x * vPsi(nRight, j, k) + k2y * vPsi(i, nRear, k) + k2y * vPsi(i, nFront, k) + k2z * vPsi(i, j, nBot) + k2z * vPsi(i, j, nTop) + rQ_Eps * localCharge) _
                        / (2 * k2xyz) - vPsi(i, j, k)) _
                        / (1 + rQ_Eps * (localP + localN) * localVolumeFraction / (2 * rVt * k2xyz))
End Select
End Function

Sub AdjustPhiSplit(ByVal deltaV As Double)
Dim i As Integer, j As Integer, k As Integer
For i = 1 To nNodesX: For j = 1 To nNodesY: For k = sTexture(i, j) To nNodesZ
    vPhiP(i, j, k) = vPhiP(i, j, k) + deltaV / 2
    vPhiN(i, j, k) = vPhiN(i, j, k) - deltaV / 2
Next k: Next j: Next i
End Sub
