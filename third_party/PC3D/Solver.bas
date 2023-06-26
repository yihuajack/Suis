Attribute VB_Name = "Solver"
Option Explicit
'This module contains the code that takes the parameters that describe the device
'and solves for the bulk and surface arrays that describe the electronic state.
'Solve calls subroutines that involve lengthy iterations, typically of indeterminate time.
'Adjust refers to subroutines that update global variables after each iteration (see Adjust2D/Adjust3D modules).
'Calculate refers to subroutines that are deterministic in their execution time (see Calculations module).
'Initialze refers to subroutines that set initial values (GlobalVariables module)
'Subroutines that are implemented as Functions and return a numeric value are in the Functions module, unless
'they are 2D vs 3D specific, in which case they are in the corresponding 2D or 3D module.

Sub Solve()
'This is called when the Solve button is pushed on the Dashboard sheet.
'Progress is posted by subroutines.
If Not bDebug Then On Error GoTo ErrHandler:
If bNeedReset Then Call ResetAll    'Typically only required upon initial startup
If currentPlot = "Layout" Or currentPlot = "Generation" Or currentPlot = "Illumination" Then
    currentPlot = "Excess Carriers"     'Don't keep updating a plot that is unaffected by the electrical solution
End If
Call ResetGeneration
Call ResetElectrical
Call InitializeGammaPsi
Call SolveGeneration
If bOpenCircuit Then
    rVoltage = VocEstimate(rTotalGeneration)
    Range(voltageCell).Value = rVoltage
    Call InitializeGammaPsi
End If
Range(progressCell).Value = "Solving": DoEvents
Call SolveSurfaces
If bOpenCircuit Then     'Refined estimate of open-circuit voltage is the QF separation at the contact locations
    rVoltage = ContactAverage(holes, 1, nNodesX, 1, nNodesY, sPhi) - ContactAverage(electrons, 1, nNodesX, 1, nNodesY, sPhi)
    Range(voltageCell).Value = rVoltage
End If
Exit Sub
    
ErrHandler:
    Range(progressCell).Value = "Error"
    If Not bBatch Then MsgBox ("Execution Error: Reset required.")
End Sub

Sub InitializeGammaPsi()
'This routine sets the surface phi for the minority carrier equal to the applied contact voltage
'and the surface phi for the majority carrier equal to zero. It also resets boolean flags for electrical values.
'These initial values of phiN and phiP are then used to set initial values of Gamma and Psi at both surfaces and in the bulk.
Dim i As Integer, j As Integer, k As Integer
Dim rPhiP As Double, rPhiN As Double, rGamma As Double, rPsi As Double
ReDim vGamma(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPsi(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaX(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaY(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradGammaZ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradPsiX(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradPsiY(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vGradPsiZ(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPointRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vLineRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
ReDim vPlaneRecombination(1 To nNodesX, 1 To nNodesY, 1 To nNodesZ) As Double
If pType Then
    rPhiP = 0: rPhiN = -rVoltage
Else
    rPhiP = rVoltage: rPhiN = 0
End If
rGamma = FunctionGamma(rPhiP, rPhiN)
If rGamma < rMinEq Then rGamma = rMinEq
rPsi = FunctionPsi(rPhiP, rPhiN)
For i = 1 To nNodesX: For j = 1 To nNodesY
    sPhi(front, holes, i, j) = rPhiP
    sPhi(front, electrons, i, j) = rPhiN
    sPhi(rear, holes, i, j) = rPhiP
    sPhi(rear, electrons, i, j) = rPhiN
    sFrontGamma(i, j) = rGamma
    sFrontPsi(i, j) = rPsi
    sRearGamma(i, j) = rGamma
    sRearPsi(i, j) = rPsi
    For k = 1 To nNodesZ
        vGamma(i, j, k) = rGamma
        vPsi(i, j, k) = rPsi
    Next k
Next j: Next i
rTotalRecombination = 0
bGamma = False:  bPsi = False
bRecombination = False: bGradGammaPsi = False
End Sub

Function SolveSurfaces() As Boolean
'Assumes that Gamma, Psi, PhiN, and PhiP on both surfaces have been initialized and vGeneration calculated.
'If bOpenCircuit, then the dissimilar-type contacts are ignored, resulting in an approximation to open circuit condition.
'The function returns false if an error occurs that results in a halt of execution.
Dim nLoops As Integer: If b3D Then nLoops = 8 Else nLoops = 4
Dim iterationCounter As Integer, loopCounter As Integer
Dim saveTotalRecomb As Double, rError As Double, previousError As Double
SolveSurfaces = False
If Not bDebug Then On Error GoTo ErrHandler:
If bOpenCircuit And Not bGeneration Then
    Range(progressCell).Value = "Dark Voc?"
    GoTo ExitHandler
End If
If rVoltage = 0 And Not bGeneration Then
    Range(progressCell).Value = "Dark Jsc?"
    GoTo ExitHandler
End If

CalculateNodalConductances      'Different if bOpenCircuit, so has to be recomputed for each SolveSurfaces

'Obtain a starting solution for Gamma and Psi using initial values (uniform QF splitting) and vGeneration
Call CalculateC
Call CalculateAB
For loopCounter = 1 To nLoops
    If b3D Then Call CalculateSurfaceGradGammaPsi3D Else Call CalculateSurfaceGradGammaPsi2D
    Call CalculateSurfaceRecombination
    Call RefineLocal(False)
    Call CalculateAB
Next loopCounter

Call CalculateRealGamma
Call CalculateRecombination

'Now iterate surface Gamma and Psi until total recombination stabilizes
previousError = 1
For iterationCounter = 1 To nMaxIterations
    saveTotalRecomb = rTotalRecombination
    Range(iterationCell).Value = iterationCounter: DoEvents
    Call CalculateGradGammaPsi
    Call CalculateC
    Call CalculateAB
    If b3D Then
         Call AdjustSurfaces3D
    Else
        Call AdjustSurfaces2D
    End If
    Call CalculateAB
    For loopCounter = 1 To nLoops
        If b3D Then Call CalculateSurfaceGradGammaPsi3D Else Call CalculateSurfaceGradGammaPsi2D
        Call CalculateSurfaceRecombination
        Call RefineLocal(True)
        Call CalculateAB
    Next loopCounter
    Call CalculateRealGamma
    Call CalculateRecombination
    If WorksheetFunction.max(saveTotalRecomb, rTotalGeneration) <> 0 Then
        rError = Abs((rTotalRecombination - saveTotalRecomb) / WorksheetFunction.max(saveTotalRecomb, rTotalGeneration))
    Else: rError = 1
    End If
    Range(errorCell).Value = rError
    If rError < rConvergence And previousError < 2 * rConvergence Then Exit For 'Ensure it's not a fluke.
    If rError > 1000 Then iterationCounter = nMaxIterations             'Exit loop if error get excessively large
    previousError = rError
    If b3D Then
        bIntermediatePlot = True
        Call OnPlotChange
    End If
Next iterationCounter

If b3D Then Call CalculateSurfaceGradGammaPsi3D Else Call CalculateSurfaceGradGammaPsi2D
Call CalculateSurfaceRecombination
Call RefineSurfaces
Call CalculateAB
Call CalculateRealGamma
Call CalculateRecombination

bIntermediatePlot = False
bConvergenceFailure = iterationCounter > nMaxIterations
If bConvergenceFailure Then Range(progressCell).Value = "Unsolved" Else Range(progressCell).Value = "Solved"
'Update values for terminals, if not open-circuit mode
If bConvergenceFailure Or bOpenCircuit Then
    Range(terminalCurrentCell).Value = ""
    Range(terminalVoltageCell).Value = ""
    Range(terminalPowerCell).Value = ""
Else
    Range(terminalCurrentCell).Value = Range(generationCell) - Range(recombinationCell)
    Range(terminalVoltageCell).Value = Range(voltageCell) - Range(terminalCurrentCell) * Range(deviceSeriesCell)
    Range(terminalPowerCell).Value = Range(terminalCurrentCell) * Range(terminalVoltageCell)
End If
If Not bConvergenceFailure Then Call FreeEnergyAnalysis
If currentPlot <> "Generation" Then Call OnPlotChange   'Don't plot electrical results if focus is on generation
SolveSurfaces = Not bConvergenceFailure
Exit Function

ErrHandler:
    Range(progressCell).Value = "Error"
    If q * rTotalRecombination > 1000000 Then  'Recombination exceeds 1 MA/cm2!
        If Not bScan And Not bBatch Then MsgBox ("Convergence Failure: Reset required.")
        SolveSurfaces = False
    Else
        If Not bScan And Not bBatch Then MsgBox ("Execution Error: Reset required.")
        SolveSurfaces = False
    End If
ExitHandler:
    Range(recombinationCell).Value = ""
    Range(terminalCurrentCell).Value = ""
    Range(terminalVoltageCell).Value = ""
    Range(terminalPowerCell).Value = ""
End Function

Sub RefineLocal(ByVal bUpdatePsi As Boolean)
'Balance flux at each individual surface node based on an analytical Jacobian that perturbs PhiN and PhiP only at that location
'Updates to Gamma and Psi are dampened by a fixed factor empirically found to improve convergence robustness.
Const damping As Double = 0.7
Dim iSurface As Integer, iCarrier As Integer, iVariable As Integer, i As Integer, j As Integer
Dim localJ As Variant, localErrors As Variant, LocalUpdates As Variant, sNodalFluxErrors As Variant
Dim sDelta As Variant
Dim newGamma As Double
ReDim sDelta(front To rear, gamma To psi, 1 To nNodesX, 1 To nNodesY) As Double
ReDim localJ(holes To electrons, gamma To psi) As Double
ReDim localErrors(holes To electrons) As Double
ReDim LocalUpdates(gamma To psi) As Double
ReDim sNodalFluxErrors(front To rear, holes To electrons, 1 To nNodesX, 1 To nNodesY) As Double
Call CalculateNodalFluxErrors(nNodesX, nNodesY, sNodalFluxErrors)
For j = 1 To nNodesY
    For iSurface = front To rear: For i = 1 To nNodesX
        localErrors(holes) = sNodalFluxErrors(iSurface, holes, i, j)
        localErrors(electrons) = sNodalFluxErrors(iSurface, electrons, i, j)
        Call CalculateLocalJacobian(iSurface, i, j, localJ)
        LocalUpdates = MSolve2x2(localJ, localErrors)
        sDelta(iSurface, gamma, i, j) = LocalUpdates(gamma)
        sDelta(iSurface, psi, i, j) = LocalUpdates(psi)
    Next i: Next iSurface
    For i = 1 To nNodesX
        'Front Gamma
        newGamma = sFrontGamma(i, j) + damping * sDelta(front, gamma, i, j)
        If newGamma <= 0 Then sFrontGamma(i, j) = sFrontGamma(i, j) / 2 Else sFrontGamma(i, j) = newGamma
        'Front Psi
        If bUpdatePsi Then sFrontPsi(i, j) = sFrontPsi(i, j) + damping * sDelta(front, psi, i, j)
        'Rear Gamma
        newGamma = sRearGamma(i, j) + damping * sDelta(rear, gamma, i, j)
        If newGamma <= 0 Then sRearGamma(i, j) = sRearGamma(i, j) / 2 Else sRearGamma(i, j) = newGamma
        'Rear Psi
        If bUpdatePsi Then sRearPsi(i, j) = sRearPsi(i, j) + damping * sDelta(rear, psi, i, j)
        'Update Phi's
        sPhi(front, holes, i, j) = PhiP(sFrontGamma(i, j), sFrontPsi(i, j))
        sPhi(front, electrons, i, j) = PhiN(sFrontGamma(i, j), sFrontPsi(i, j))
        sPhi(rear, holes, i, j) = PhiP(sRearGamma(i, j), sRearPsi(i, j))
        sPhi(rear, electrons, i, j) = PhiN(sRearGamma(i, j), sRearPsi(i, j))
    Next i
Next j
bGamma = False: bPsi = False: bRecombination = False: bGradGammaPsi = False
End Sub

Sub RefineSurfaces()
'Balance flux at each individual surface node based on an analytical Jacobian that perturbs gamma and psi at that and all adjacent nodes
Dim iSurface As Integer, iCarrier As Integer, iVariable As Integer, i As Integer, j As Integer, n As Integer, bw As Integer
Dim m As Variant, V As Variant      'Jacobian matrix and solution vector
Dim sNodalFluxErrors As Variant, sDelta As Variant
Dim newGamma As Double
ReDim sNodalFluxErrors(front To rear, holes To electrons, 1 To nNodesX, 1 To nNodesY) As Double
ReDim sDelta(front To rear, gamma To psi, 1 To nNodesX, 1 To nNodesY) As Double
Const damping = 1
n = 2 * nNodesX * nNodesY           'Length of error vector, v
bw = 4 * nNodesY + 7     'Bandwidth of Jacobian matrix, m
Call CalculateNodalFluxErrors(nNodesX, nNodesY, sNodalFluxErrors)
For iSurface = front To rear
    ReDim V(1 To n) As Double
    For i = 1 To nNodesX: For j = 1 To nNodesY: For iCarrier = holes To electrons
            V(2 * ((i - 1) * nNodesY + (j - 1)) + iCarrier) = sNodalFluxErrors(iSurface, iCarrier, i, j)
    Next iCarrier: Next j: Next i
    ReDim m(1 To n, 1 To n) As Double
    Call CalculateSurfaceJacobian(iSurface, m)
    Call LUSolve(m, n, bw, V)
    For i = 1 To nNodesX: For j = 1 To nNodesY: For iCarrier = holes To electrons
            sDelta(iSurface, iCarrier, i, j) = -V(2 * ((i - 1) * nNodesY + (j - 1)) + iCarrier)
    Next iCarrier: Next j: Next i
Next iSurface
For i = 1 To nNodesX: For j = 1 To nNodesY
        'Front Gamma
        newGamma = sFrontGamma(i, j) + damping * sDelta(front, gamma, i, j)
        If newGamma <= 0 Then sFrontGamma(i, j) = sFrontGamma(i, j) / 2 Else sFrontGamma(i, j) = newGamma
        'Front Psi
        sFrontPsi(i, j) = sFrontPsi(i, j) + damping * sDelta(front, psi, i, j)
        'Rear Gamma
        newGamma = sRearGamma(i, j) + damping * sDelta(rear, gamma, i, j)
        If newGamma <= 0 Then sRearGamma(i, j) = sRearGamma(i, j) / 2 Else sRearGamma(i, j) = newGamma
        'Rear Psi
        sRearPsi(i, j) = sRearPsi(i, j) + damping * sDelta(rear, psi, i, j)
        'Update Phi's
        sPhi(front, holes, i, j) = PhiP(sFrontGamma(i, j), sFrontPsi(i, j))
        sPhi(front, electrons, i, j) = PhiN(sFrontGamma(i, j), sFrontPsi(i, j))
        sPhi(rear, holes, i, j) = PhiP(sRearGamma(i, j), sRearPsi(i, j))
        sPhi(rear, electrons, i, j) = PhiN(sRearGamma(i, j), sRearPsi(i, j))
Next j: Next i
bGamma = False: bPsi = False: bRecombination = False: bGradGammaPsi = False
End Sub

Sub LUSolve(ByRef m As Variant, ByVal n As Integer, ByVal bw As Integer, ByRef V As Variant)
'Solves using Lower-Upper decomposition followed by foward and back solve for a banded matrix
Dim i As Integer, j As Integer, k As Integer, hbw As Integer, nmin As Integer, nmax As Integer
hbw = (bw - 1) \ 2
'LU Decomposition
For i = 1 To n
    If i + hbw > n Then nmax = n Else nmax = i + hbw
    For j = i + 1 To nmax
         If m(j, i) <> 0 Then
                m(j, i) = m(j, i) / m(i, i)
                For k = i + 1 To nmax
                    m(j, k) = m(j, k) - m(j, i) * m(i, k)
                Next k
         End If
    Next j
Next i
'Forward Solve
For i = 1 To n
    If i - hbw < 1 Then nmin = 1 Else nmin = i - hbw
    For k = nmin To i - 1
        V(i) = V(i) - m(i, k) * V(k)
    Next k
Next i
'Backward Solve
For i = n To 1 Step -1
    If i + hbw > n Then nmax = n Else nmax = i + hbw
    For k = i + 1 To nmax
        V(i) = V(i) - m(i, k) * V(k)
    Next k
    V(i) = V(i) / m(i, i)
Next i
End Sub

Sub saveSurface(ByVal nSurface As Integer)
savePhi = sPhi
If nSurface = front Then
    saveGamma = sFrontGamma
    savePsi = sFrontPsi
    saveRecomb = sFrontRecomb
Else
    saveGamma = sRearGamma
    savePsi = sRearPsi
    saveRecomb = sRearRecomb
End If
End Sub

Sub RestoreSurface(ByVal nSurface As Integer)
sPhi = savePhi
If nSurface = front Then
    sFrontGamma = saveGamma
    sFrontPsi = savePsi
    sFrontRecomb = saveRecomb
Else
    sRearGamma = saveGamma
    sRearPsi = savePsi
    sRearRecomb = saveRecomb
End If
End Sub


